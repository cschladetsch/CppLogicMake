#!/usr/bin/env pwsh
# Compatibility wrapper for running the driver.

if ($args.Count -eq 0) {
    & "$PSScriptRoot/generate.ps1" `
        -Input "examples/kai_workspace.lm" `
        -Output "build/run/CMakeLists.txt"
} else {
    & "$PSScriptRoot/generate.ps1" @args
}
exit $LASTEXITCODE
