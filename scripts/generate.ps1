#!/usr/bin/env pwsh
# generate.ps1 — run the driver against one or more .lm project files.
#
# Single project:
#   ./scripts/generate.ps1 -Input examples/kai_workspace.lm -Output CMakeLists.txt
#
# Multiple projects, resolved in parallel (see README's "Multi-threading"
# section — one CppProlog engine per input, no shared mutable state):
#   ./scripts/generate.ps1 -Input a.lm,b.lm,c.lm -OutputDir generated/

param(
    [Parameter(Mandatory = $true)]
    [Alias("Input")]
    [string[]]$ProjectInput,

    [string]$Output = "CMakeLists.txt",

    [string]$OutputDir = "",

    [string]$Schema = "prolog/targets.pl",

    [string]$BuildDir = "build",

    # Directory the driver runs from. The driver resolves a project's
    # sources with `git ls-files` and stamps provenance with `git
    # rev-parse` in its own working directory, and emits source paths
    # relative to it — so this must be the *project's* git root, not the
    # tool repo. Defaults to the tool repo (RepoRoot) for backward
    # compatibility when invoked directly.
    [string]$WorkingDirectory = ""
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot

# Locate (and if necessary build) the driver, and resolve the schema, in
# the tool repo — these live with the tool, not the project. Capture
# absolute paths so they still resolve once we change into the project's
# working directory to run the driver.
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

    $driverPath = (Resolve-Path $driver).ProviderPath

    if ([System.IO.Path]::IsPathRooted($Schema)) {
        $schemaPath = $Schema
    } else {
        $schemaPath = Join-Path $RepoRoot $Schema
    }
    $schemaPath = (Resolve-Path -LiteralPath $schemaPath).ProviderPath
}
finally {
    Pop-Location
}

$driverArgs = @("--schema", $schemaPath)
foreach ($i in $ProjectInput) { $driverArgs += @("--input", $i) }

if ($ProjectInput.Count -gt 1) {
    if (-not $OutputDir) { throw "-OutputDir is required when passing multiple -Input files" }
    $driverArgs += @("--output-dir", $OutputDir)
} else {
    $driverArgs += @("--output", $Output)
}

$runDir = if ($WorkingDirectory) {
    (Resolve-Path -LiteralPath $WorkingDirectory).ProviderPath
} else {
    $RepoRoot
}

Push-Location $runDir
try {
    if ($ProjectInput.Count -le 1) {
        $outputParent = Split-Path -Parent $Output
        if ($outputParent) {
            New-Item -ItemType Directory -Force -Path $outputParent | Out-Null
        }
    }

    & $driverPath @driverArgs
    if ($LASTEXITCODE -ne 0) { throw "generation failed" }
}
finally {
    Pop-Location
}
