#!/usr/bin/env pwsh
# install.ps1 — make `logimake` a global command in this user's
# PowerShell sessions.
#
# It adds a small wrapper function to your PowerShell profile ($PROFILE)
# that calls this repo's logimake.ps1 by absolute path. A profile
# function — rather than putting the repo on PATH — is what lets you type
# a bare `logimake`: Windows PATHEXT doesn't include .PS1, so a script on
# PATH wouldn't resolve without its extension.
#
# Idempotent: the managed block is delimited by markers, so re-running
# updates it in place instead of appending a duplicate (handy after you
# move the repo — just re-run to refresh the path).
#
# Usage:
#   ./install.ps1              install / refresh the `logimake` command
#   ./install.ps1 -Uninstall   remove it again

param(
    [switch]$Uninstall
)

$ErrorActionPreference = "Stop"

$RepoRoot = $PSScriptRoot
$Script = Join-Path $RepoRoot "logimake.ps1"
if (-not (Test-Path -LiteralPath $Script)) {
    throw "logimake.ps1 not found next to install.ps1 (looked in $RepoRoot)"
}

$beginMarker = "# >>> CppLogicMake (logimake) >>>"
$endMarker   = "# <<< CppLogicMake (logimake) <<<"

# The block written into the profile. Note the backtick-escaped `$ so
# those expand at *call* time inside the profile, while $Script is
# interpolated now to bake in this repo's absolute path. Defaulting
# LOGICMAKE_ROOT to the script's folder also lets it build project files
# that live outside the repo tree, where directory walk-up finds nothing.
$block = @"
$beginMarker
function logimake {
    `$script = '$Script'
    if (-not `$env:LOGICMAKE_ROOT) {
        `$env:LOGICMAKE_ROOT = Split-Path -Parent `$script
    }
    & `$script @args
}
$endMarker
"@

$profilePath = $PROFILE
if (-not (Test-Path -LiteralPath $profilePath)) {
    $dir = Split-Path -Parent $profilePath
    if ($dir -and -not (Test-Path -LiteralPath $dir)) {
        New-Item -ItemType Directory -Path $dir -Force | Out-Null
    }
    New-Item -ItemType File -Path $profilePath | Out-Null
}

$content = Get-Content -LiteralPath $profilePath -Raw
if ($null -eq $content) { $content = "" }

# Strip any previously-installed managed block before deciding what to do.
$pattern = [regex]::Escape($beginMarker) + ".*?" + [regex]::Escape($endMarker)
$content = ([regex]::Replace($content, $pattern, "", "Singleline")).TrimEnd()

if ($Uninstall) {
    Set-Content -LiteralPath $profilePath -Value $content
    Write-Host "Removed the logimake command from $profilePath" -ForegroundColor Green
    return
}

$newContent = if ($content) { "$content`n`n$block`n" } else { "$block`n" }
Set-Content -LiteralPath $profilePath -Value $newContent

Write-Host "Installed the logimake command to $profilePath" -ForegroundColor Green
Write-Host "Reload it with:  . `$PROFILE   (or just open a new shell)"
Write-Host "Then, from anywhere:  logimake build <project.pl>"
