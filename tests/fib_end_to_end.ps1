#!/usr/bin/env pwsh
# fib_end_to_end.ps1 — the whole-scope integration test.
#
# It drives the real `logimake` CLI to turn examples/fib/fib.lm into a
# built C++23 and a built C++17 executable (exercising: .lm parsing ->
# Prolog resolution -> git-backed source resolution -> per-target
# CXX_STANDARD emission -> CMake configure -> CMake build), then runs
# those two executables plus the Python and Rust siblings and asserts all
# available implementations print the same answer: the sum of the first
# ten Fibonacci numbers, 88.
#
# Python and Rust are skipped (not failed) when their toolchains are
# absent, so the test is meaningful on any machine and complete on one
# with all four.
#
# Usage:
#   ./tests/fib_end_to_end.ps1 [-BuildDir <dir>]

param(
    [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"

$RepoRoot  = Split-Path -Parent $PSScriptRoot
$logimake  = Join-Path $RepoRoot "logimake.ps1"
$project   = Join-Path $RepoRoot "examples/fib/fib.lm"
$expected  = "88"

if ([System.IO.Path]::IsPathRooted($BuildDir)) {
    $fibBuild = Join-Path $BuildDir "logimake/fib"
} else {
    $fibBuild = Join-Path $RepoRoot (Join-Path $BuildDir "logimake/fib")
}

$binExt = if ($IsWindows -or $env:OS -eq "Windows_NT") { ".exe" } else { "" }

function Get-CommandOutput {
    param([string]$Exe, [string[]]$Arguments = @())
    $out = if ($Arguments.Count -gt 0) { & $Exe @Arguments } else { & $Exe }
    if ($LASTEXITCODE -ne 0) { throw "$Exe exited with $LASTEXITCODE" }
    return ($out | Out-String).Trim()
}

function Find-BuiltExe {
    param([string]$Root, [string]$Name)
    $needle = "$Name$binExt"
    $hit = Get-ChildItem -Path $Root -Recurse -File -Filter $needle -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if (-not $hit) { throw "built executable '$needle' not found under $Root" }
    return $hit.FullName
}

# Resolve a real Python interpreter, skipping the Windows Store "app
# execution alias" stub under WindowsApps — it isn't a usable
# interpreter and throws when its output is redirected (as it is here).
function Resolve-Python {
    foreach ($name in @("python", "python3")) {
        $candidates = Get-Command $name -All -ErrorAction SilentlyContinue
        foreach ($c in $candidates) {
            if ($c.Source -and $c.Source -notmatch '[\\/]WindowsApps[\\/]') {
                return $c.Source
            }
        }
    }
    return $null
}

# --- 1. Build the two C++ targets end-to-end through the logimake CLI ---
if (-not $env:LOGICMAKE_ROOT) { $env:LOGICMAKE_ROOT = $RepoRoot }
& $logimake build $project -BuildDir $fibBuild
if ($LASTEXITCODE -ne 0) { throw "logimake build failed for $project" }

# --- 2. The emitted CMake must carry the per-target standards (the
#        feature that makes a C++17 and a C++23 target coexist) ---
$generated = Get-Content -Raw -LiteralPath (Join-Path $fibBuild "src/CMakeLists.txt")
$failures = @()
if ($generated -notmatch 'set_target_properties\(fib_cpp23 PROPERTIES CXX_STANDARD 23') {
    $failures += "generated CMake is missing CXX_STANDARD 23 for fib_cpp23"
}
if ($generated -notmatch 'set_target_properties\(fib_cpp17 PROPERTIES CXX_STANDARD 17') {
    $failures += "generated CMake is missing CXX_STANDARD 17 for fib_cpp17"
}

# --- 3. Collect each implementation's output (skipping absent toolchains) ---
$results = [ordered]@{}
$binDir  = Join-Path $fibBuild "build"

$results["cpp23"] = Get-CommandOutput (Find-BuiltExe $binDir "fib_cpp23")
$results["cpp17"] = Get-CommandOutput (Find-BuiltExe $binDir "fib_cpp17")

$python = Resolve-Python
if ($python) {
    $results["python"] = Get-CommandOutput $python @((Join-Path $RepoRoot "examples/fib/fib.py"))
} else {
    Write-Host "[SKIP] python (no interpreter found; the WindowsApps stub does not count)" -ForegroundColor Yellow
}

$rustc = Get-Command rustc -ErrorAction SilentlyContinue
if ($rustc) {
    $rustOutDir = Join-Path $fibBuild "rust"
    New-Item -ItemType Directory -Force -Path $rustOutDir | Out-Null
    $rustExe = Join-Path $rustOutDir "fib_rust$binExt"
    & $rustc.Source (Join-Path $RepoRoot "examples/fib/fib.rs") -o $rustExe
    if ($LASTEXITCODE -ne 0) { throw "rustc failed to compile examples/fib/fib.rs" }
    $results["rust"] = Get-CommandOutput $rustExe
} else {
    Write-Host "[SKIP] rust (rustc not installed)" -ForegroundColor Yellow
}

# --- 4. Every available implementation must agree on the expected sum ---
foreach ($lang in $results.Keys) {
    if ($results[$lang] -ne $expected) {
        $failures += "$lang printed '$($results[$lang])', expected '$expected'"
    } else {
        Write-Host ("[ OK ] {0,-6} = {1}" -f $lang, $results[$lang]) -ForegroundColor Green
    }
}

if ($failures.Count -gt 0) {
    foreach ($f in $failures) { Write-Host "[FAIL] $f" -ForegroundColor Red }
    exit 1
}

Write-Host "fib cross-language end-to-end: $($results.Count) implementation(s) all agree on $expected" -ForegroundColor Green
