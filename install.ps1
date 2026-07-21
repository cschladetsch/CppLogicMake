#!/usr/bin/env pwsh
# install.ps1 — make `logimake` a global command for this user.
#
# It writes small launcher shims into <repo>/bin and puts that directory
# on the user's PATH, so a bare `logimake` resolves as an ordinary
# executable in every shell (cmd, PowerShell, and git-bash) — not just in
# PowerShell. .CMD is in the default PATHEXT, so unlike a bare .ps1 the
# shim resolves without needing its extension typed.
#
# The shims bake in this repo's absolute path (like a compiled launcher
# would), and default LOGICMAKE_ROOT to it so logimake can build project
# files that live outside the repo tree, where directory walk-up finds
# nothing.
#
# Idempotent: re-running refreshes the shims and leaves PATH with a single
# entry (handy after moving the repo — just re-run). It also removes the
# profile-function install used by earlier versions, so upgrading is a
# no-op beyond re-running this.
#
# Usage:
#   ./install.ps1              install / refresh the `logimake` command
#   ./install.ps1 -Uninstall   remove it again

param(
    [switch]$Uninstall,
    # Skip building the driver during install (just wire up the command);
    # the driver is then built lazily on the first `logimake build`.
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$RepoRoot = $PSScriptRoot
$Script   = Join-Path $RepoRoot "logimake.ps1"
if (-not (Test-Path -LiteralPath $Script)) {
    throw "logimake.ps1 not found next to install.ps1 (looked in $RepoRoot)"
}

$BinDir  = Join-Path $RepoRoot "bin"
$CmdShim = Join-Path $BinDir "logimake.cmd"
$ShShim  = Join-Path $BinDir "logimake"

# A ZIP download tags files with the "mark of the web", which can stop
# PowerShell from running them under the default execution policy. Clear
# it from the scripts this wires up so direct invocation works too (the
# shims also pass -ExecutionPolicy Bypass). Best-effort and Windows-only.
if ($IsWindows -or $env:OS -eq "Windows_NT") {
    foreach ($d in @($RepoRoot, (Join-Path $RepoRoot "scripts"), (Join-Path $RepoRoot "tests"))) {
        if (Test-Path -LiteralPath $d) {
            Get-ChildItem -LiteralPath $d -Filter *.ps1 -File -ErrorAction SilentlyContinue |
                Unblock-File -ErrorAction SilentlyContinue
        }
    }
}

# --- user PATH helpers (registry-backed; new shells inherit it) --------

function Get-UserPathParts {
    $raw = [Environment]::GetEnvironmentVariable("PATH", "User")
    if (-not $raw) { return @() }
    return @($raw -split ';' | Where-Object { $_ -ne '' })
}

function Set-UserPathParts {
    param([string[]]$Parts)
    [Environment]::SetEnvironmentVariable("PATH", ($Parts -join ';'), "User")
}

function Add-ToUserPath {
    param([string]$Dir)
    $parts = Get-UserPathParts
    if ($parts -notcontains $Dir) {
        Set-UserPathParts (@($parts) + $Dir)
    }
    # Reflect in the current process so a shell that dot-sources this
    # (or runs it and keeps going) sees it without a restart.
    if (($env:PATH -split ';') -notcontains $Dir) {
        $env:PATH = "$env:PATH;$Dir"
    }
}

function Remove-FromUserPath {
    param([string]$Dir)
    $parts = Get-UserPathParts
    if ($parts -contains $Dir) {
        Set-UserPathParts (@($parts | Where-Object { $_ -ne $Dir }))
    }
    $env:PATH = (($env:PATH -split ';' | Where-Object { $_ -ne $Dir }) -join ';')
}

# --- migration: drop the profile-function install from older versions --

$beginMarker = "# >>> CppLogicMake (logimake) >>>"
$endMarker   = "# <<< CppLogicMake (logimake) <<<"

function Remove-LegacyProfileBlock {
    $profilePath = $PROFILE
    if (-not (Test-Path -LiteralPath $profilePath)) { return }
    $content = Get-Content -LiteralPath $profilePath -Raw
    if ($null -eq $content) { return }
    $pattern = [regex]::Escape($beginMarker) + ".*?" + [regex]::Escape($endMarker)
    if ($content -notmatch $pattern) { return }
    $stripped = ([regex]::Replace($content, $pattern, "", "Singleline")).TrimEnd()
    Set-Content -LiteralPath $profilePath -Value $stripped
    Write-Host "Removed the legacy logimake profile function from $profilePath" -ForegroundColor Yellow
}

# --- uninstall ---------------------------------------------------------

if ($Uninstall) {
    Remove-LegacyProfileBlock
    Remove-FromUserPath $BinDir
    if (Test-Path -LiteralPath $CmdShim) { Remove-Item -Force -LiteralPath $CmdShim }
    if (Test-Path -LiteralPath $ShShim)  { Remove-Item -Force -LiteralPath $ShShim }
    if ((Test-Path -LiteralPath $BinDir) -and
        -not (Get-ChildItem -LiteralPath $BinDir -Force)) {
        Remove-Item -Force -LiteralPath $BinDir
    }
    Write-Host "Uninstalled the logimake command (removed $BinDir from PATH)." -ForegroundColor Green
    Write-Host "Open a new shell for the PATH change to take effect."
    return
}

# --- install -----------------------------------------------------------

Remove-LegacyProfileBlock
New-Item -ItemType Directory -Force -Path $BinDir | Out-Null

$RepoRootFwd = $RepoRoot -replace '\\', '/'

# Windows shim: prefer pwsh, fall back to Windows PowerShell. setlocal
# keeps the defaulted LOGICMAKE_ROOT from leaking into the caller's cmd
# session; the last interpreter call's exit code is what the .cmd
# returns, so build failures still propagate.
$cmdContent = @"
@echo off
setlocal
if not defined LOGICMAKE_ROOT set "LOGICMAKE_ROOT=$RepoRoot"
where /q pwsh
if %ERRORLEVEL%==0 (
    pwsh -NoProfile -ExecutionPolicy Bypass -File "$Script" %*
) else (
    powershell -NoProfile -ExecutionPolicy Bypass -File "$Script" %*
)
"@
# .cmd must be CRLF to be safe for cmd.exe.
Set-Content -LiteralPath $CmdShim -Value $cmdContent -Encoding ascii

# POSIX shim for git-bash / WSL. LF line endings, no BOM.
$shContent = @"
#!/usr/bin/env sh
: "`${LOGICMAKE_ROOT:=$RepoRootFwd}"
export LOGICMAKE_ROOT
exec pwsh -NoProfile -ExecutionPolicy Bypass -File "$RepoRootFwd/logimake.ps1" "`$@"
"@
$shBytes = [System.Text.Encoding]::ASCII.GetBytes(($shContent -replace "`r`n", "`n"))
[System.IO.File]::WriteAllBytes($ShShim, $shBytes)

Add-ToUserPath $BinDir

Write-Host "Installed the logimake command." -ForegroundColor Green
Write-Host "  shims: $CmdShim"
Write-Host "         $ShShim"
Write-Host "  PATH : added $BinDir (user)"

# Build the driver now so a fresh checkout yields a working `logimake`
# immediately, and so any missing prerequisite (uninitialised submodule,
# no compiler/cmake) is reported here with context rather than surfacing
# on the user's first build. build.ps1 initialises the external/ submodules
# itself when they are absent. The command is already installed above, so
# a build failure is a warning, not fatal — the lazy build retries on
# first use.
if (-not $SkipBuild) {
    Write-Host ""
    Write-Host "Building the logimake driver (first-time setup)..." -ForegroundColor Cyan
    $buildOk = $false
    try {
        & (Join-Path $RepoRoot "scripts/build.ps1")
        if ($LASTEXITCODE -ne 0) { throw "build.ps1 exited with code $LASTEXITCODE" }
        $buildOk = $true
    } catch {
        Write-Warning "Driver build did not complete: $($_.Exception.Message)"
        if (-not (Test-Path -LiteralPath (Join-Path $RepoRoot "external/CppProlog/src/prolog/interpreter.h"))) {
            Write-Warning "The external/CppProlog submodule looks missing. If you downloaded a ZIP, clone instead:"
            Write-Warning "    git clone --recursive <repo-url>   (or run: git submodule update --init --recursive)"
        } else {
            Write-Warning "Check that cmake and a C++23 compiler (clang/gcc) are installed and on PATH."
        }
        Write-Warning "The 'logimake' command is still installed; it will retry the build on first use."
    }
}

Write-Host ""
Write-Host "Open a NEW shell, then from anywhere:  logimake build <project.lm>"
Write-Host "(This session already has it on PATH: try  logimake help)"
