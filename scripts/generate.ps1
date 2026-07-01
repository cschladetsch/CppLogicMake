#!/usr/bin/env pwsh
# generate.ps1 — run the driver against one or more .pl project files.
#
# Single project:
#   ./scripts/generate.ps1 -Input examples/kai_workspace.pl -Output CMakeLists.txt
#
# Multiple projects, resolved in parallel (see README's "Multi-threading"
# section — one CppProlog engine per input, no shared mutable state):
#   ./scripts/generate.ps1 -Input a.pl,b.pl,c.pl -OutputDir generated/

param(
    [Parameter(Mandatory = $true)]
    [Alias("Input")]
    [string[]]$ProjectInput,

    [string]$Output = "CMakeLists.txt",

    [string]$OutputDir = "",

    [string]$Schema = "prolog/targets.pl",

    [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
Push-Location $RepoRoot
try {
    $binName = if ($IsWindows -or $env:OS -eq "Windows_NT") { "logicmake.exe" } else { "logicmake" }
    $driver = Join-Path $BuildDir $binName
    $configDriver = Join-Path (Join-Path $BuildDir "Release") $binName

    if (-not (Test-Path $driver) -and (Test-Path $configDriver)) {
        $driver = $configDriver
    }

    if (-not (Test-Path $driver)) {
        Write-Host "driver not built yet, building..." -ForegroundColor Yellow
        & "$PSScriptRoot/build.ps1" -BuildDir $BuildDir
        if ($LASTEXITCODE -ne 0) { throw "build failed" }
        if (-not (Test-Path $driver) -and (Test-Path $configDriver)) {
            $driver = $configDriver
        }
    }

    $driverArgs = @("--schema", $Schema)
    foreach ($i in $ProjectInput) { $driverArgs += @("--input", $i) }

    if ($ProjectInput.Count -gt 1) {
        if (-not $OutputDir) { throw "-OutputDir is required when passing multiple -Input files" }
        $driverArgs += @("--output-dir", $OutputDir)
    } else {
        $outputParent = Split-Path -Parent $Output
        if ($outputParent) {
            New-Item -ItemType Directory -Force -Path $outputParent | Out-Null
        }
        $driverArgs += @("--output", $Output)
    }

    & (Resolve-Path $driver) @driverArgs
    if ($LASTEXITCODE -ne 0) { throw "generation failed" }
}
finally {
    Pop-Location
}
