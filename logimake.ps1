#!/usr/bin/env pwsh

param(
    [string]$LogicMakeRoot = "",
    [string]$BuildDir = "",
    [string]$Generator = "",
    [string]$CxxCompiler = "",

    [Parameter(Position = 0, ValueFromRemainingArguments = $true)]
    [object[]]$AllArgs
)

$ErrorActionPreference = "Stop"
if ($AllArgs.Count -gt 0) {
    $Command = [string]$AllArgs[0]
    $Rest = @($AllArgs | Select-Object -Skip 1)
} else {
    $Command = "help"
    $Rest = @()
}
function Remove-BoundOptionValue {
    param(
        [object[]]$Items,
        [string]$Value
    )

    if (-not $Value) {
        return $Items
    }

    $removed = $false
    $result = @()
    foreach ($item in $Items) {
        if (-not $removed -and [string]$item -eq $Value) {
            $removed = $true
            continue
        }
        $result += $item
    }
    return $result
}

function Remove-BoundOptionPhrase {
    param(
        [object[]]$Items,
        [string[]]$Names,
        [string]$Value
    )

    if (-not $Value) {
        return $Items
    }

    $result = @()
    foreach ($item in $Items) {
        $text = [string]$item
        foreach ($name in $Names) {
            $index = $text.IndexOf($name)
            if ($index -ge 0) {
                $text = $text.Substring(0, $index)
            }
        }
        if ($text) {
            $result += $text
        }
    }
    return $result
}

$Rest = @(Remove-BoundOptionPhrase $Rest @("-LogicMakeRoot", "--logicmake-root") $LogicMakeRoot)
$Rest = @(Remove-BoundOptionPhrase $Rest @("-BuildDir", "--build-dir") $BuildDir)
$Rest = @(Remove-BoundOptionPhrase $Rest @("-Generator", "--generator") $Generator)
$Rest = @(Remove-BoundOptionPhrase $Rest @("-CxxCompiler", "--cxx-compiler") $CxxCompiler)
$Rest = @(Remove-BoundOptionValue $Rest $LogicMakeRoot)
$Rest = @(Remove-BoundOptionValue $Rest $BuildDir)
$Rest = @(Remove-BoundOptionValue $Rest $Generator)
$Rest = @(Remove-BoundOptionValue $Rest $CxxCompiler)
if ($LogicMakeRoot) {
    $Rest += @("-LogicMakeRoot", $LogicMakeRoot)
}
if ($BuildDir) {
    $Rest += @("-BuildDir", $BuildDir)
}
if ($Generator) {
    $Rest += @("-Generator", $Generator)
}
if ($CxxCompiler) {
    $Rest += @("-CxxCompiler", $CxxCompiler)
}
function Find-LogicMakeRoot {
    param([string]$StartDir)

    $dir = Resolve-Path -LiteralPath $StartDir
    while ($dir) {
        $candidate = $dir.ProviderPath
        if ((Test-Path -LiteralPath (Join-Path $candidate "scripts/generate.ps1")) -and
            (Test-Path -LiteralPath (Join-Path $candidate "prolog/targets.pl"))) {
            return $candidate
        }

        $parent = Split-Path -Parent $candidate
        if (-not $parent -or $parent -eq $candidate) {
            break
        }
        $dir = Resolve-Path -LiteralPath $parent
    }

    return $null
}

function Resolve-LogicMakeRoot {
    param(
        [string]$StartDir,
        [string]$LogicMakeRoot = ""
    )

    if ($LogicMakeRoot) {
        return (Resolve-Path -LiteralPath $LogicMakeRoot).ProviderPath
    }
    if ($env:LOGICMAKE_ROOT) {
        return (Resolve-Path -LiteralPath $env:LOGICMAKE_ROOT).ProviderPath
    }

    $root = Find-LogicMakeRoot -StartDir $StartDir
    if (-not $root) {
        $root = Find-LogicMakeRoot -StartDir (Get-Location).ProviderPath
    }
    if (-not $root) {
        throw "Could not find CppLogicMake root. Run from inside the repo, set LOGICMAKE_ROOT, or pass -LogicMakeRoot."
    }
    return $root
}

function Resolve-ProjectPath {
    param([string]$Project)

    if ([System.IO.Path]::IsPathRooted($Project)) {
        return (Resolve-Path -LiteralPath $Project).ProviderPath
    }
    return (Resolve-Path -LiteralPath (Join-Path (Get-Location) $Project)).ProviderPath
}

function Resolve-ProjectRoot {
    param([string]$ProjectDir)

    Push-Location -LiteralPath $ProjectDir
    try {
        $top = git rev-parse --show-toplevel 2>$null
        if ($LASTEXITCODE -eq 0 -and $top) {
            return (Resolve-Path -LiteralPath ([string]$top).Trim()).ProviderPath
        }
    }
    catch {
        # git not installed or not a repo — fall through to the .pl dir.
    }
    finally {
        Pop-Location
    }

    return (Resolve-Path -LiteralPath $ProjectDir).ProviderPath
}

function Resolve-ProjectFileArgument {
    param([string]$Argument)

    # Already carries a project extension: hand it to the builder as-is
    # so a missing file fails normally (file-not-found), not as a
    # "no project file" error.
    if ($Argument -match '\.(pl|lm)$') {
        return $Argument
    }

    # A bare name resolves, in order, to:
    #   1. <name>/<name>.lm  — a project living in its own directory
    #   2. <name>.lm         — a bare project file in the current dir
    # .lm is the going-forward extension; .pl is accepted at each step
    # only during the transition, and only as a second choice.
    $leaf = Split-Path -Leaf $Argument
    if (Test-Path -LiteralPath $Argument -PathType Container) {
        foreach ($ext in @(".lm", ".pl")) {
            $inDir = Join-Path $Argument "$leaf$ext"
            if (Test-Path -LiteralPath $inDir -PathType Leaf) {
                return $inDir
            }
        }
    }

    foreach ($ext in @(".lm", ".pl")) {
        $bare = "$Argument$ext"
        if (Test-Path -LiteralPath $bare -PathType Leaf) {
            return $bare
        }
    }

    # No directory-scoped or bare project file matched.
    return $null
}

function Read-OptionValue {
    param(
        [object[]]$InputArgs,
        [int]$Index,
        [string]$Name
    )

    if ($Index + 1 -ge $InputArgs.Count) {
        throw "$Name requires a value"
    }
    return [string]$InputArgs[$Index + 1]
}

function Invoke-ProjectBuild {
    param([object[]]$BuildArgs)

    if ($BuildArgs.Count -gt 0 -and [string]$BuildArgs[0] -eq "build") {
        $BuildArgs = @($BuildArgs | Select-Object -Skip 1)
    }

    $project = ""
    $logicMakeRoot = ""
    $buildDir = ""
    $generator = ""
    $cxxCompiler = "clang++"

    for ($i = 0; $i -lt $BuildArgs.Count; ++$i) {
        $arg = [string]$BuildArgs[$i]
        if ($arg -in @("-LogicMakeRoot", "--logicmake-root")) {
            $logicMakeRoot = Read-OptionValue -InputArgs $BuildArgs -Index $i -Name $arg
            ++$i
        } elseif ($arg -in @("-BuildDir", "--build-dir")) {
            $buildDir = Read-OptionValue -InputArgs $BuildArgs -Index $i -Name $arg
            ++$i
        } elseif ($arg -in @("-Generator", "--generator")) {
            $generator = Read-OptionValue -InputArgs $BuildArgs -Index $i -Name $arg
            ++$i
        } elseif ($arg -in @("-CxxCompiler", "--cxx-compiler")) {
            $cxxCompiler = Read-OptionValue -InputArgs $BuildArgs -Index $i -Name $arg
            ++$i
        } else {
            if ($arg.StartsWith("-")) {
                throw "Unknown build option: $arg"
            }
            if ($project) {
                throw "Only one project (.lm) file can be passed to logimake build"
            }
            $project = $arg
        }
    }

    if (-not $project) {
        throw "Usage: logimake build <project.lm> [-BuildDir <dir>] [-LogicMakeRoot <dir>]"
    }

    $projectPath = Resolve-ProjectPath $project
    $projectDir = Split-Path -Parent $projectPath

    # $toolRoot only locates the tool's own driver binary and schema.
    # Everything else — source resolution, build output, cmake — is
    # anchored to the project, not the tool repo.
    $toolRoot = Resolve-LogicMakeRoot -StartDir $projectDir -LogicMakeRoot $logicMakeRoot

    # The project's git root is the working directory the driver and
    # cmake operate from: source pathspecs in the .pl file are resolved
    # (via git ls-files) and emitted relative to it. Fall back to the
    # .pl file's own directory when it isn't inside a git repo.
    $projectRoot = Resolve-ProjectRoot -ProjectDir $projectDir

    $projectName = [System.IO.Path]::GetFileNameWithoutExtension($projectPath)
    if (-not $buildDir) {
        $buildDir = Join-Path $projectRoot "build/logimake/$projectName"
    } elseif (-not [System.IO.Path]::IsPathRooted($buildDir)) {
        $buildDir = Join-Path (Get-Location) $buildDir
    }

    $scratchSourceDir = Join-Path $buildDir "src"
    $scratchBuildDir = Join-Path $buildDir "build"
    $generatedCMake = Join-Path $scratchSourceDir "CMakeLists.txt"

    New-Item -ItemType Directory -Force -Path $scratchSourceDir | Out-Null
    New-Item -ItemType Directory -Force -Path $scratchBuildDir | Out-Null

    # Never let a failed generation leave a previous run's CMakeLists.txt
    # in place for cmake to build — clear it first so a resolver error
    # (which now halts generate.ps1) can't silently fall through to a
    # stale target.
    if (Test-Path -LiteralPath $generatedCMake) {
        Remove-Item -Force -LiteralPath $generatedCMake
    }

    # The generated CMakeLists.txt references the project's sources by
    # path relative to its own location (back into $projectRoot), so no
    # copy of the source tree into the scratch dir is needed — cmake
    # compiles the originals in place while build artifacts stay under
    # $scratchBuildDir. The driver runs with $projectRoot as its working
    # directory (via -WorkingDirectory) so git and glob resolution
    # happen in the project's repo.
    & "$toolRoot/scripts/generate.ps1" -Input $projectPath -Output $generatedCMake -WorkingDirectory $projectRoot
    if ($LASTEXITCODE -ne 0) { throw "CppLogicMake generation failed" }
    if (-not (Test-Path -LiteralPath $generatedCMake)) {
        throw "CppLogicMake generation did not produce $generatedCMake"
    }

    Push-Location $projectRoot
    try {
        $cmakeArgs = @("-S", $scratchSourceDir, "-B", $scratchBuildDir)
        if ($generator) {
            $cmakeArgs += @("-G", $generator)
        } elseif (Get-Command ninja -ErrorAction SilentlyContinue) {
            $cmakeArgs += @("-G", "Ninja")
        }
        if ($cxxCompiler) {
            $cmakeArgs += "-DCMAKE_CXX_COMPILER=$cxxCompiler"
        }

        cmake @cmakeArgs
        if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

        cmake --build $scratchBuildDir
        if ($LASTEXITCODE -ne 0) { throw "CMake build failed" }

        Write-Host "built: $scratchBuildDir" -ForegroundColor Green
    }
    finally {
        Pop-Location
    }
}

function Split-LogicMakeRootOption {
    param([object[]]$ScriptArgs)

    $logicMakeRoot = ""
    $remaining = @()
    for ($i = 0; $i -lt $ScriptArgs.Count; ++$i) {
        $arg = [string]$ScriptArgs[$i]
        if ($arg -in @("-LogicMakeRoot", "--logicmake-root")) {
            $logicMakeRoot = Read-OptionValue -InputArgs $ScriptArgs -Index $i -Name $arg
            ++$i
        } else {
            $remaining += $arg
        }
    }

    return @{
        LogicMakeRoot = $logicMakeRoot
        Args = $remaining
    }
}

function Invoke-RepoScript {
    param(
        [string]$ScriptName,
        [object[]]$ScriptArgs
    )

    if ($ScriptArgs.Count -gt 0 -and [string]$ScriptArgs[0] -eq $ScriptName) {
        $ScriptArgs = @($ScriptArgs | Select-Object -Skip 1)
    }

    $split = Split-LogicMakeRootOption $ScriptArgs
    $repoRoot = Resolve-LogicMakeRoot -StartDir (Get-Location).ProviderPath -LogicMakeRoot $split.LogicMakeRoot
    $scriptPath = "$repoRoot/scripts/$ScriptName.ps1"
    if ($split.Args -and $split.Args.Count -gt 0) {
        & $scriptPath @($split.Args)
    } else {
        & $scriptPath
    }
    if ($LASTEXITCODE -ne 0) {
        throw "$ScriptName failed"
    }
}

function Show-Usage {
    Write-Host @"
Usage:
  logimake build <project.lm> [options]
  logimake generate <scripts/generate.ps1 args>
  logimake verify [scripts/verify.ps1 args]
  logimake test [scripts/test.ps1 args]

Build options:
  -LogicMakeRoot <dir>   CppLogicMake repo root, if it cannot be discovered
  -BuildDir <dir>        Output directory for generated/build files
  -Generator <name>      CMake generator, e.g. Ninja
  -CxxCompiler <path>    C++ compiler passed to CMake, default clang++

Compatibility:
  logimake <project.lm>     Same as logimake build <project.lm>
  logimake <name>           Resolves <name>/<name>.lm, else <name>.lm in the current
                            directory (.pl accepted during the transition)
"@
}

# Recognized subcommands are dispatched before any file resolution, so a
# mistyped command can never be misread as a (missing) project file.
switch ($Command) {
    "build" {
        Invoke-ProjectBuild -BuildArgs @($Rest)
        return
    }
    "generate" {
        Invoke-RepoScript -ScriptName "generate" -ScriptArgs $Rest
        return
    }
    "verify" {
        Invoke-RepoScript -ScriptName "verify" -ScriptArgs $Rest
        return
    }
    "test" {
        Invoke-RepoScript -ScriptName "test" -ScriptArgs $Rest
        return
    }
    { $_ -in @("help", "--help", "-h") } {
        Show-Usage
        return
    }
}

# Not a subcommand: treat the argument as a project file to build,
# resolving a bare <name> to <name>/<name>.lm or a bare <name>.lm.
$projectFile = Resolve-ProjectFileArgument $Command
if ($projectFile) {
    Invoke-ProjectBuild -BuildArgs (@($projectFile) + @($Rest))
    return
}

# Hard stop — never fall through to some other target (which is how the
# earlier root-confusion bug built the wrong thing).
[Console]::Error.WriteLine("No subcommand or project file found for '$Command'. Try: logimake build <path-to-.lm-file>, or place $Command.lm inside a $Command/ directory.")
exit 1
