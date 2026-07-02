#!/usr/bin/env pwsh
# verify.ps1 — generates CMakeLists.txt from examples/kai_workspace.lm
# and actually configures + builds it with real CMake, in a scratch
# directory. This is the accuracy check that matters: passing GTest
# doesn't prove the *output* is valid CMake, only that this repo's own
# code behaves as expected. A CMakeLists.txt that fails to configure
# (as every version of this tool's output did before git-backed
# source resolution replaced raw globs — see README) would still pass
# every unit test that doesn't itself invoke cmake.
#
# Usage:
#   ./scripts/verify.ps1

param(
    [string]$BuildDir = "build",
    [string]$ScratchDir = ""
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
        & "$PSScriptRoot/build.ps1" -BuildDir $BuildDir
        if ($LASTEXITCODE -ne 0) { throw "build failed" }
        if (-not (Test-Path $driver) -and (Test-Path $configDriver)) {
            $driver = $configDriver
        }
    }

    if (-not $ScratchDir) {
        $ScratchDir = Join-Path ([System.IO.Path]::GetTempPath()) "logicmake_verify_$(Get-Random)"
    }
    New-Item -ItemType Directory -Force -Path $ScratchDir | Out-Null

    # Generated paths are repo-root-relative (see README's "Loading
    # model" / source resolution notes), so the scratch CMakeLists.txt
    # needs the same relative layout as the real repo root: copy the
    # example fixture tree alongside it rather than symlinking the
    # whole repo.
    Copy-Item -Recurse -Force "examples" (Join-Path $ScratchDir "examples")

    $generated = Join-Path $ScratchDir "CMakeLists.txt"
    & (Resolve-Path $driver) --input "examples/kai_workspace.lm" --output $generated
    if ($LASTEXITCODE -ne 0) { throw "generation failed" }

    Push-Location $ScratchDir
    try {
        cmake -S . -B build
        if ($LASTEXITCODE -ne 0) { throw "generated CMakeLists.txt failed to configure" }

        cmake --build build
        if ($LASTEXITCODE -ne 0) { throw "generated CMakeLists.txt failed to build" }

        Write-Host "verified: generated CMakeLists.txt configures and builds" -ForegroundColor Green
    }
    finally {
        Pop-Location
    }

    Remove-Item -Recurse -Force $ScratchDir
}
finally {
    Pop-Location
}
