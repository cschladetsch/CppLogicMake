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
    [string[]]$Input,

    [string]$Output = "CMakeLists.txt",

    [string]$OutputDir = "",

    [string]$Schema = "prolog/targets.pl",

    [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
Push-Location $RepoRoot
try {
    $binName = if ($IsWindows) { "logicmake.exe" } else { "logicmake" }
    $driver = Join-Path $BuildDir $binName

    if (-not (Test-Path $driver)) {
        Write-Host "driver not built yet, building..." -ForegroundColor Yellow
        & "$PSScriptRoot/build.ps1" -BuildDir $BuildDir
        if ($LASTEXITCODE -ne 0) { throw "build failed" }
    }

    $driverArgs = @("--schema", $Schema)
    foreach ($i in $Input) { $driverArgs += @("--input", $i) }

    if ($Input.Count -gt 1) {
        if (-not $OutputDir) { throw "-OutputDir is required when passing multiple -Input files" }
        $driverArgs += @("--output-dir", $OutputDir)
    } else {
        $driverArgs += @("--output", $Output)
    }

    & $driver @driverArgs
    if ($LASTEXITCODE -ne 0) { throw "generation failed" }
}
finally {
    Pop-Location
}
