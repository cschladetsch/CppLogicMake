#!/usr/bin/env pwsh
# build.ps1 — configure and build CppLogicMake.
#
# Defaults to clang/clang++. Override with -Compiler gcc if you'd
# rather use gcc/g++ (both are verified to produce identical output —
# see README's "Toolchain" section).
#
# Usage:
#   ./scripts/build.ps1 [-BuildDir build] [-Config Release] [-Compiler clang]
#   ./scripts/build.ps1 -Sanitize thread     # TSan build, see README

param(
    [string]$BuildDir = "build",
    [string]$Config = "Release",
    [ValidateSet("clang", "gcc")]
    [string]$Compiler = "clang",
    [ValidateSet("", "thread", "address")]
    [string]$Sanitize = ""
)

$ErrorActionPreference = "Stop"

if ($IsWindows -or $env:OS -eq "Windows_NT") {
    $pathValue = [System.Environment]::GetEnvironmentVariable("Path", "Process")
    if (-not $pathValue) {
        $pathValue = [System.Environment]::GetEnvironmentVariable("PATH", "Process")
    }
    if ($pathValue) {
        [System.Environment]::SetEnvironmentVariable("PATH", $null, "Process")
        [System.Environment]::SetEnvironmentVariable("Path", $null, "Process")
        [System.Environment]::SetEnvironmentVariable("Path", $pathValue, "Process")
    }
}

$RepoRoot = Split-Path -Parent $PSScriptRoot
Push-Location $RepoRoot
try {
    if (-not (Test-Path "external/CppProlog/src/prolog/interpreter.h") -or
        -not (Test-Path "external/googletest/googletest/CMakeLists.txt")) {
        Write-Host "submodules not initialised, running git submodule update..." -ForegroundColor Yellow
        git submodule update --init --recursive
        if ($LASTEXITCODE -ne 0) { throw "submodule init failed" }
    }

    $onWindows = $IsWindows -or $env:OS -eq "Windows_NT"
    $useNinja = $onWindows -and $Compiler -eq "clang" -and
        [bool](Get-Command ninja -ErrorAction SilentlyContinue)
    $cc = if ($Compiler -eq "clang") {
        "clang"
    } else {
        "gcc"
    }
    $cxx = if ($Compiler -eq "clang") {
        "clang++"
    } else {
        "g++"
    }

    $cachePath = Join-Path $BuildDir "CMakeCache.txt"
    if ($useNinja -and (Test-Path $cachePath)) {
        $cache = Get-Content $cachePath -Raw
        if ($cache -notmatch "CMAKE_GENERATOR:INTERNAL=Ninja") {
            Write-Host "build dir was configured without Ninja/Clang, regenerating CMake cache..." -ForegroundColor Yellow
            Remove-Item -Force -LiteralPath $cachePath
            $cmakeFiles = Join-Path $BuildDir "CMakeFiles"
            if (Test-Path $cmakeFiles) {
                Remove-Item -Recurse -Force -LiteralPath $cmakeFiles
            }
        }
    }

    $cmakeArgs = @(
        "-S", ".", "-B", $BuildDir
    )
    if ($useNinja) {
        $cmakeArgs += @("-G", "Ninja")
    }
    $cmakeArgs += @(
        "-DCMAKE_C_COMPILER=$cc",
        "-DCMAKE_CXX_COMPILER=$cxx",
        "-DCMAKE_BUILD_TYPE=$Config"
    )
    if ($Sanitize) {
        $cmakeArgs += "-DLOGICMAKE_SANITIZE=$Sanitize"
    }

    cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { throw "cmake configure failed" }

    cmake --build $BuildDir --config $Config
    if ($LASTEXITCODE -ne 0) { throw "cmake build failed" }

    Write-Host "built: $BuildDir/logicmake" -ForegroundColor Green
}
finally {
    Pop-Location
}
