#!/usr/bin/env pwsh
# Compatibility wrapper for the pluralized test script name.

& "$PSScriptRoot/test.ps1" @args
exit $LASTEXITCODE
