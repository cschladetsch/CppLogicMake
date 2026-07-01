#!/usr/bin/env pwsh
# test.ps1 — build and run the test suite via CTest.
#
# Usage:
#   ./scripts/test.ps1 [-BuildDir build]

param(
    [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
Push-Location $RepoRoot
try {
    & "$PSScriptRoot/build.ps1" -BuildDir $BuildDir
    if ($LASTEXITCODE -ne 0) { throw "build failed" }

    Push-Location $BuildDir
    try {
        ctest --output-on-failure
        if ($LASTEXITCODE -ne 0) { throw "tests failed" }
    }
    finally {
        Pop-Location
    }

    Write-Host "all tests passed" -ForegroundColor Green
}
finally {
    Pop-Location
}
