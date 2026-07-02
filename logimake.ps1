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
                throw "Only one project .pl file can be passed to logimake build"
            }
            $project = $arg
        }
    }

    if (-not $project) {
        throw "Usage: logimake build <project.pl> [-BuildDir <dir>] [-LogicMakeRoot <dir>]"
    }

    $projectPath = Resolve-ProjectPath $project
    $projectDir = Split-Path -Parent $projectPath
    $repoRoot = Resolve-LogicMakeRoot -StartDir $projectDir -LogicMakeRoot $logicMakeRoot

    $projectName = [System.IO.Path]::GetFileNameWithoutExtension($projectPath)
    if (-not $buildDir) {
        $buildDir = Join-Path $repoRoot "build/logimake/$projectName"
    } elseif (-not [System.IO.Path]::IsPathRooted($buildDir)) {
        $buildDir = Join-Path (Get-Location) $buildDir
    }

    $scratchSourceDir = Join-Path $buildDir "src"
    $scratchBuildDir = Join-Path $buildDir "build"
    $generatedCMake = Join-Path $scratchSourceDir "CMakeLists.txt"

    New-Item -ItemType Directory -Force -Path $scratchSourceDir | Out-Null
    New-Item -ItemType Directory -Force -Path $scratchBuildDir | Out-Null

    Push-Location $repoRoot
    try {
        & "$repoRoot/scripts/generate.ps1" -Input $projectPath -Output $generatedCMake
        if ($LASTEXITCODE -ne 0) { throw "CppLogicMake generation failed" }

        $projectRelativePath = Resolve-Path -LiteralPath $projectPath -Relative
        $projectRelativePath = $projectRelativePath.TrimStart(".", "\", "/")
        if (-not $projectRelativePath.StartsWith("..")) {
            $projectTop = ($projectRelativePath -split '[\\/]', 2)[0]
            $trackedFiles = git ls-files -- $projectTop
            if ($LASTEXITCODE -ne 0) { throw "Could not enumerate tracked project files" }
            foreach ($file in $trackedFiles) {
                $sourceFile = Join-Path $repoRoot $file
                $targetFile = Join-Path $scratchSourceDir $file
                $targetDir = Split-Path -Parent $targetFile
                New-Item -ItemType Directory -Force -Path $targetDir | Out-Null
                Copy-Item -Force -LiteralPath $sourceFile -Destination $targetFile
            }
        } else {
            $projectLeafDir = Split-Path -Leaf $projectDir
            $copyTarget = Join-Path $scratchSourceDir $projectLeafDir
            Copy-Item -Recurse -Force $projectDir $copyTarget
        }

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
  logimake build <project.pl> [options]
  logimake generate <scripts/generate.ps1 args>
  logimake verify [scripts/verify.ps1 args]
  logimake test [scripts/test.ps1 args]

Build options:
  -LogicMakeRoot <dir>   CppLogicMake repo root, if it cannot be discovered
  -BuildDir <dir>        Output directory for generated/build files
  -Generator <name>      CMake generator, e.g. Ninja
  -CxxCompiler <path>    C++ compiler passed to CMake, default clang++

Compatibility:
  logimake <project.pl>     Same as logimake build <project.pl>
"@
}

if ($Command.EndsWith(".pl") -or (Test-Path -LiteralPath $Command)) {
    $buildArgs = @($Command) + @($Rest)
    Invoke-ProjectBuild -BuildArgs $buildArgs
    return
}

switch ($Command) {
    "build" {
        $buildArgs = @($Rest)
        Invoke-ProjectBuild -BuildArgs $buildArgs
    }
    "generate" {
        Invoke-RepoScript -ScriptName "generate" -ScriptArgs $Rest
    }
    "verify" {
        Invoke-RepoScript -ScriptName "verify" -ScriptArgs $Rest
    }
    "test" {
        Invoke-RepoScript -ScriptName "test" -ScriptArgs $Rest
    }
    "help" {
        Show-Usage
    }
    "--help" {
        Show-Usage
    }
    "-h" {
        Show-Usage
    }
    default {
        throw "Unknown logimake command: $Command"
    }
}
