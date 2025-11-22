<#
.SYNOPSIS
    构建第三方依赖包。

.DESCRIPTION
    该脚本用于构建项目所需的第三方依赖包，并将其安装到指定目录。

.PARAMETER Help
    显示帮助信息。

.PARAMETER PkgNames
    要构建的包的名称列表。注意 Qt6 Additional 包名需要以 Qt6 开头。

.PARAMETER BuildTypes
    要构建的配置类型列表。Qt6 Additional 包不受此参数影响，将始终构建Debug和RelWithDebInfo版本。

.PARAMETER Generator
    CMake 生成器类型，支持 "Ninja" 和 "Ninja Multi-Config"。

.PARAMETER BuildRoot
    构建输出的根目录。

.PARAMETER InstallDir
    第三方包的安装目录。Qt6 Additional 包不受此参数影响，将始终安装在系统 Qt6 目录下。
#>


param (
    [Parameter(Mandatory = $false)]
    [Alias("h")]
    [switch]$Help,
    
    [ValidateSet("HuskarUI", "Qt6Mqtt")]
    [string[]]$PkgNames = @("HuskarUI", "Qt6Mqtt"), 

    [string[]]$BuildTypes = @("Debug", "Release"),

    [ValidateSet("Ninja", "Ninja Multi-Config")]
    [Alias("G")]
    [string]$Generator = "Ninja",

    [Alias("B")]
    [string]$BuildRoot = "../build",

    [string]$InstallDir = "../output/3rdparty"
)

function GetUnixStylePath {
    param (
        [Parameter(Mandatory = $true)]
        [string]$Path
    )
    return $Path -replace '\\', '/'
}

function GetAbsolutePath {
    param (
        [Parameter(Mandatory = $true)]
        [string]$Path
    )
    try {
        $resolved = (Resolve-Path -LiteralPath $Path -ErrorAction Stop).Path
    }
    catch {
        $resolved = [System.IO.Path]::GetFullPath($Path)
    }
    return GetUnixStylePath -Path $resolved
}

function ApplyDiffPatch {
    param (
        [Parameter(Mandatory = $true)]
        [string]$PatchFile,
        [Parameter(Mandatory = $true)]
        [string]$TargetDir
    )

    $PatchFile = GetAbsolutePath -Path $PatchFile
    $TargetDir = GetAbsolutePath -Path $TargetDir

    & git -C "$TargetDir" apply --check --quiet "$PatchFile"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Patch $PatchFile has already been applied to $TargetDir, skipped."
        return
    }
    Write-Host "Applying patch $PatchFile to $TargetDir ..."
    & git -C "$TargetDir" apply --ignore-space-change --whitespace=nowarn "$PatchFile"
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to apply patch $PatchFile to $TargetDir"
        exit 1
    }
}

function RevertDiffPatch {
    param (
        [Parameter(Mandatory = $true)]
        [string]$PatchFile,
        [Parameter(Mandatory = $true)]
        [string]$TargetDir
    )

    $PatchFile = GetAbsolutePath -Path $PatchFile
    $TargetDir = GetAbsolutePath -Path $TargetDir

    Write-Host "Reverting patch $PatchFile in $TargetDir ..."
    & git -C "$TargetDir" apply -R --ignore-space-change --whitespace=nowarn "$PatchFile"
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to revert patch $PatchFile in $TargetDir"
        exit 1
    }
}

function GetQt6Dir {
    if (-not $env:QT6_DIR) {
        Write-Error "Environment variable QT6_DIR is not set. Please set it to the Qt6 installation directory."
        exit 1
    }
    return GetUnixStylePath -Path $env:QT6_DIR
}

function GetPkgFullName {
    param (
        [string]$PkgName,
        [Parameter(Mandatory = $false)]
        [string]$Type = ""
    )

    $PkgFullName = $PkgName
    if ($PkgName.StartsWith("Qt6")) {
        $subName = $PkgName.Substring(3)
        $PkgFullName = "Qt6::${subName}"
    }
    if ($Type -ne "") {
        $PkgFullName += " ($Type)"
    }

    return $PkgFullName
}

function CMakeConfigure {
    param (
        [Parameter(Mandatory = $true)]
        [string]$PkgName,
        [Parameter(Mandatory = $true)]
        [string]$PkgSrcDir,
        [Parameter(Mandatory = $true)]
        [string]$PkgBuildDir,
        [string]$PkgInstallDir,
        [string]$PkgBuildType = "",
        [string[]]$CMAKE_CONFIG_ARGS = @()
    )

    if (-not (Test-Path $PkgBuildDir)) {
        New-Item -ItemType Directory -Path $PkgBuildDir | Out-Null
    }

    $logFile = Join-Path $PkgBuildDir "$PkgName-config.log"
    $pkgFullName = GetPkgFullName -PkgName $PkgName -Type $PkgBuildType

    write-Host "==================== Configuring $pkgFullName ====================" -ForegroundColor Green

    # 切换到构建目录
    Set-Location $PkgBuildDir

    $Qt6Pkg = $PkgName.StartsWith("Qt6")
    if ($Qt6Pkg) {
        & $env:Qt6_DIR/bin/qt-configure-module.bat $PkgSrcDir 2>&1 | Tee-Object -FilePath $logFile
        if ($LASTEXITCODE -ne 0) {
            Write-Error "qt-configure-module for $pkgFullName failed, see more details at $logFile";
            exit 1 
        }
    }
    else {
        & cmake -S $PkgSrcDir -B $PkgBuildDir @CMAKE_CONFIG_ARGS -DCMAKE_INSTALL_PREFIX="$PkgInstallDir" 2>&1 | Tee-Object -FilePath $logFile
        if ($LASTEXITCODE -ne 0) {
            Write-Error "cmake configure $pkgFullName failed, see more details at $logFile";
            exit 1 
        }
    }
}

function CMakeBuild {
    param (
        [Parameter(Mandatory = $true)]
        [string]$PkgName,
        [Parameter(Mandatory = $true)]
        [string]$PkgBuildDir,
        [string]$PkgBuildType = ""
    )

    if (-not (Test-Path $PkgBuildDir)) {
        Write-Error "Build directory $PkgBuildDir does not exist."
        exit 1
    }

    $logFile = Join-Path $PkgBuildDir "$PkgName-build.log"
    $pkgFullName = GetPkgFullName -PkgName $PkgName -Type $PkgBuildType

    write-Host "==================== Building $pkgFullName ====================" -ForegroundColor Green

    # 切换到构建目录
    Set-Location $PkgBuildDir

    $configArg = if ([string]::IsNullOrEmpty($PkgBuildType)) { @() } else { @("--config", $PkgBuildType) }

    & cmake --build . @configArg --parallel 2>&1 | Tee-Object -FilePath $logFile

    if ($LASTEXITCODE -ne 0) {
        Write-Error "cmake build $pkgFullName failed, see more details at $logFile";
        exit 1 
    }
}

function CMakeInstall {
    param (
        [Parameter(Mandatory = $true)]
        [string]$PkgName,
        [Parameter(Mandatory = $true)]
        [string]$PkgBuildDir,
        [string]$PkgBuildType = ""
    )

    if (-not (Test-Path $PkgBuildDir)) {
        Write-Error "Build directory $PkgBuildDir does not exist."
        exit 1
    }

    $logFile = Join-Path $PkgBuildDir "$PkgName-install.log"
    $pkgFullName = GetPkgFullName -PkgName $PkgName -Type $PkgBuildType

    write-Host "==================== Installing $pkgFullName ====================" -ForegroundColor Green

    # 切换到构建目录
    Set-Location $PkgBuildDir

    $configArg = if ([string]::IsNullOrEmpty($PkgBuildType)) { @() } else { @("--config", $PkgBuildType) }

    & cmake --build . @configArg --target install 2>&1 | Tee-Object -FilePath $logFile

    if ($LASTEXITCODE -ne 0) {
        Write-Error "cmake install $pkgFullName failed, see more details at $logFile";
        exit 1 
    }
}

function BuildPackage {
    param (
        [Parameter(Mandatory = $true)]
        [string]$PkgName,
        [string[]]$CMAKE_CONFIG_ARGS = @()
    )
    $Qt6Pkg = $PkgName.StartsWith("Qt6")
    if ($Qt6Pkg) {
        $PkgDirName = "qt$($PkgName.Substring(3).ToLower())"
        $PkgBuildTypes = @("") # Qt6 包构建不受 BuildTypes 参数影响，内部自动构建 Debug 和 RelWithDebInfo 版本
        $PkgInstallDir = GetQt6Dir # Qt6 包始终安装在系统 Qt6 目录下
    }
    else {
        $PkgDirName = $PkgName
        $PkgBuildTypes = $BuildTypes
        $PkgInstallDir = GetAbsolutePath -Path $InstallDir
    }
    $PkgSrcDir = GetAbsolutePath -Path "./3rdparty/$PkgDirName"
    
    if (-not (Test-Path $PkgSrcDir)) {
        $PkgFullName = GetPkgFullName -PkgName $PkgName
        Write-Error "Package '$PkgFullName' source directory $PkgSrcDir does not exist."
        exit 1
    }

    if ($Generator -eq "Ninja") {
        $CMAKE_CONFIG_ARGS += @("-G", "Ninja")
        foreach ($Type in $PkgBuildTypes) {
            if ($Qt6Pkg) {
                $BuildName = "$($BuildTypes[0].ToLower())-windows-$env:VSCMD_ARG_TGT_ARCH" # 取第一个构建类型作为目录名
            } else {
                $BuildName = "$($Type.ToLower())-windows-$env:VSCMD_ARG_TGT_ARCH" # e.g. debug-windows-x64
            }
            $PkgBuildDir = GetUnixStylePath (Join-Path -Path $BuildRoot -ChildPath ("$BuildName/3rdparty/$PkgDirname"))
            $LOCAL_CMAKE_CONFIG_ARGS = $CMAKE_CONFIG_ARGS + "-DCMAKE_BUILD_TYPE=${Type}"
            CMakeConfigure -PkgName $PkgName -PkgSrcDir $PkgSrcDir -PkgBuildDir $PkgBuildDir -PkgInstallDir $PkgInstallDir `
                -PkgBuildType $Type -CMAKE_CONFIG_ARGS $LOCAL_CMAKE_CONFIG_ARGS
            CMakeBuild -PkgName $PkgName -PkgBuildDir $PkgBuildDir -PkgBuildType $Type
            CMakeInstall -PkgName $PkgName -PkgBuildDir $PkgBuildDir -PkgBuildType $Type
        }
    }
    elseif ($Generator -eq "Ninja Multi-Config") {
        $CMAKE_CONFIG_ARGS += @("-G", "Ninja Multi-Config")
        $BuildName = "windows-$env:VSCMD_ARG_TGT_ARCH" # e.g. windows-x64
        $PkgBuildDir = GetUnixStylePath (Join-Path -Path $BuildRoot -ChildPath ("$BuildName/3rdparty/$PkgDirname"))
        CMakeConfigure -PkgName $PkgName -PkgSrcDir $PkgSrcDir -PkgBuildDir $PkgBuildDir -PkgInstallDir $PkgInstallDir `
            -CMAKE_CONFIG_ARGS $CMAKE_CONFIG_ARGS
        foreach ($Type in $PkgBuildTypes) {
            CMakeBuild -PkgName $PkgName -PkgBuildDir $PkgBuildDir -PkgBuildType $Type
            CMakeInstall -PkgName $PkgName -PkgBuildDir $PkgBuildDir -PkgBuildType $Type
        }
    }

}

if ($Help) {
    Get-Help $PSCommandPath -Full -ErrorAction SilentlyContinue
    exit 0
}

# 检查是否在Visual Studio环境中运行
if (-not $env:VCINSTALLDIR -or -not (Get-Command "cl.exe" -ErrorAction SilentlyContinue)) {
    Write-Host "Not in Visual Studio environment. Please run this script from a Visual Studio Developer Command Prompt."
    exit 1
}

$repoRoot = GetAbsolutePath -Path (Join-Path $PSScriptRoot "..")
$BuildRoot = GetAbsolutePath -Path (Join-Path $repoRoot $BuildRoot)
$InstallDir = GetAbsolutePath -Path (Join-Path $repoRoot $InstallDir)

$originalPath = Get-Location

try {
    foreach ($PkgName in $PkgNames) {
        Set-Location $repoRoot
        switch ($PkgName) {
            "HuskarUI" {
                ApplyDiffPatch -PatchFile "${repoRoot}/3rdparty/patches/${PkgName}.diff" -TargetDir "${repoRoot}/3rdparty/$PkgName"
                BuildPackage -PkgName $PkgName `
                    -CMAKE_CONFIG_ARGS @("-DCMAKE_PREFIX_PATH=$env:Qt6_DIR", `
                                         "-DBUILD_HUSKARUI_GALLERY=OFF", `
                                         "-DINSTALL_HUSKARUI_IN_DEFAULT_LOCATION=OFF")
                RevertDiffPatch -PatchFile "${repoRoot}/3rdparty/patches/${PkgName}.diff" -TargetDir "${repoRoot}/3rdparty/$PkgName"
            }
            "Qt6Mqtt" {
                BuildPackage -PkgName $PkgName
            }
            Default {
                Write-Error "Unknown package name: $PkgName"
                continue
            }
        }
    }
} finally {
    Set-Location $originalPath
}
