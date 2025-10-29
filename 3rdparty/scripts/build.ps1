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
    if (-not [System.IO.Path]::IsPathRooted($Path)) {
        $AbsolutePath = [System.IO.Path]::GetFullPath((Join-Path -Path $PSScriptRoot -ChildPath $Path))
    }
    else {
        $AbsolutePath = [System.IO.Path]::GetFullPath($Path)
    }
    return GetUnixStylePath -Path $AbsolutePath
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
        return
    }
    Write-Host "Applying patch $PatchFile to $TargetDir ..."
    & git -C "$TargetDir" apply "$PatchFile"
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to apply patch $PatchFile to $TargetDir"
        exit 1
    }
}

function CheckPackageExistence {
    param (
        [string]$PkgName,
        [string]$InstallDir = "../../../output/3rdparty",
        [bool]$Verbose = $true
    )

    # 获取安装位置的绝对路径
    $InstallDir = GetAbsolutePath -Path $InstallDir

    if ($Verbose) {
        Write-Host "Checking if there is a cmake package '$PkgName' in $InstallDir ..."
    }

    # 先检查cmake包目录是否存在
    if (-not (Test-Path "${InstallDir}/lib/cmake/${PkgName}")) {
        return $false
    }

    # 创建临时CMakeLists.txt文件以检查包是否可以find
    $TempDir = New-Item -ItemType Directory -Path (Join-Path -Path $env:TEMP -ChildPath ([System.Guid]::NewGuid().ToString()))
    $CMakeFile = Join-Path $TempDir "CMakeLists.txt"
    $CMakeContent = @"
    cmake_minimum_required(VERSION 3.20)
    project(Check${PkgName})
    find_package(${PkgName} QUIET PATHS "$InstallDir" NO_DEFAULT_PATH)

    if(${PkgName}_FOUND)
        message(STATUS "${PkgName} found")
    else()
        message(FATAL_ERROR "${PkgName} not found")
    endif()
"@
    Set-Content -Path $CMakeFile -Value $CMakeContent -Encoding UTF8

    $Found = $false
    $output = & cmake -S $TempDir.FullName -B "$TempDir/build" 2>&1 | Out-String

    if ($LASTEXITCODE -ne 0 -or $output -match "CMake Error" -or $output -match "$PkgName not found") {
        $Found = $false
    }
    else {
        $Found = $true
    }

    Remove-Item -Recurse -Force $TempDir

    return $Found
}

function CheckQt6PackageExistence {
    param (
        [string]$PkgName,
        [bool]$Verbose = $true
    )

    $InstallDir = GetAbsolutePath -Path $env:Qt6_DIR

    if ($Verbose) {
        Write-Host "Checking if there is a Qt6 cmake package '$PkgName' in $InstallDir ..."
    }

    # 先检查cmake包目录是否存在
    if (-not (Test-Path "${InstallDir}/lib/cmake/Qt6${PkgName}")) {
        return $false
    }

    # 创建临时CMakeLists.txt文件以检查包是否可以find
    $TempDir = New-Item -ItemType Directory -Path (Join-Path -Path $env:TEMP -ChildPath ([System.Guid]::NewGuid().ToString()))
    $CMakeFile = Join-Path $TempDir "CMakeLists.txt"
    $CMakeContent = @"
    cmake_minimum_required(VERSION 3.20)
    project(Check${PkgName})
    Message(STATUS "Using CMAKE_PREFIX_PATH: `${CMAKE_PREFIX_PATH}`")
    find_package(Qt6 COMPONENTS ${PkgName} QUIET)

    if(TARGET Qt6::${PkgName})
        message(STATUS "Qt6::${PkgName} found")
    else()
        message(FATAL_ERROR "Qt6::${PkgName} not found")
    endif()
"@
    Set-Content -Path $CMakeFile -Value $CMakeContent -Encoding UTF8

    $Found = $false
    $output = & cmake -S $TempDir.FullName -B "$TempDir/build" -DCMAKE_PREFIX_PATH="$env:QT6_DIR" 2>&1 | Out-String

    if ($LASTEXITCODE -ne 0 -or $output -match "CMake Error" -or $output -match "$PkgName not found") {
        $Found = $false
    }
    else {
        $Found = $true
    }

    Remove-Item -Recurse -Force $TempDir

    return $Found
}

function CMakeBuildAndInstall {
    param (
        [Parameter(Mandatory = $true)]
        [string]$PkgName,
        [Parameter(Mandatory = $true)]
        [string]$SrcDir,
        [Parameter(Mandatory = $true)]
        [string]$BuildDir,
        [string]$InstallDir,
        [string]$BuildType = "Release",
        [string[]]$CMAKE_CONFIG_ARGS = @()
    )

    write-Host "================= Building ($BuildType) ================="

    if (-not (Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir | Out-Null
    }

    $cfgLog = Join-Path $BuildDir "$PkgName-config.log"
    $buildLog = Join-Path $BuildDir "$PkgName-build.log"
    $installLog = Join-Path $BuildDir "$PkgName-install.log"

    & cmake -S $SrcDir -B $BuildDir -G Ninja -DCMAKE_BUILD_TYPE="$BuildType" -DCMAKE_INSTALL_PREFIX="$InstallDir" @CMAKE_CONFIG_ARGS 2>&1 | Tee-Object -FilePath $cfgLog
    if ($LASTEXITCODE -ne 0) {
        Write-Error "cmake configure ($BuildType) failed, see $cfgLog for details";
        exit 1 
    }
    
    & cmake --build $BuildDir --config $BuildType --target all --parallel 2>&1 | Tee-Object -FilePath $buildLog
    if ($LASTEXITCODE -ne 0) {
        Write-Error "cmake build ($BuildType) failed, see $buildLog for details";
        exit 1 
    }

    & cmake --build $BuildDir --config $BuildType --target install 2>&1 | Tee-Object -FilePath $installLog
    if ($LASTEXITCODE -ne 0) {
        Write-Error "cmake install ($BuildType) failed, see $installLog for details";
        exit 1
    }
}

function BuildPackage {
    param (
        [Parameter(Mandatory = $true)]
        [string]$PkgName,
        [string]$SrcDir = "../$PkgName",
        [string]$BuildRoot = "../../../build",
        [string]$InstallDir = "../../../output/3rdparty",
        [string[]]$CMAKE_CONFIG_ARGS = @(),
        [string[]]$BuildType = @("Debug", "Release"),
        [bool]$ForceRebuild = $false
    )

    $SrcDir = GetAbsolutePath -Path $SrcDir
    $BuildRoot = GetAbsolutePath -Path $BuildRoot
    $InstallDir = GetAbsolutePath -Path $InstallDir

    if (-not $ForceRebuild) {
        if (-not (CheckPackageExistence -PkgName $PkgName -InstallDir $InstallDir)) {
            Write-Host "$PkgName not found. Building..."
        }
        else {
            Write-Host "$PkgName already exists. Skipping build."
            return
        }
    }

    if (-not (Test-Path $SrcDir)) {
        Write-Error "Package '$PkgName' source directory $SrcDir does not exist."
        exit 1
    }

    foreach ($Type in $BuildType) {
        $BuildName = "$($Type.ToLower())-windows-$env:VSCMD_ARG_TGT_ARCH" # e.g. debug-windows-x64
        $BuildDir = GetUnixStylePath (Join-Path -Path $BuildRoot -ChildPath ("$BuildName/3rdparty/$PkgName"))
        CMakeBuildAndInstall -PkgName $PkgName -SrcDir $SrcDir -BuildDir $BuildDir -InstallDir $InstallDir -BuildType $Type -CMAKE_CONFIG_ARGS $CMAKE_CONFIG_ARGS
    }

    # 最后再次检查包是否成功安装
    if (CheckPackageExistence -PkgName $PkgName -InstallDir $InstallDir -Verbose $false) {
        Write-Host "$PkgName built and installed successfully. You can use it as shown below in your CMakeLists.txt now:"
        Write-Host "    find_package($PkgName CONFIG REQUIRED)"
    }
    else {
        Write-Error "$PkgName build or installation failed."
        exit 1
    }
}

function BuildQt6Package {
    param (
        [Parameter(Mandatory = $true)]
        [string]$PkgName,
        [string]$SrcDir = "",
        [string]$BuildRoot = "../../../build",
        [bool]$ForceRebuild = $false
    )

    # 确保Qt环境变量已设置
    if (-not $env:Qt6_DIR) {
        Write-Error "Qt6_DIR environment variable is not set. Please set it to the Qt installation directory."
        exit 1
    }

    if (-not $SrcDir -or $SrcDir -eq "") {
        $SrcDir = GetAbsolutePath -Path "../qt$($PkgName.ToLower())"
    }
    $BuildRoot = GetAbsolutePath -Path $BuildRoot
    $BuildName = "debug-windows-$env:VSCMD_ARG_TGT_ARCH" # e.g. debug-windows-x64
    $BuildDir = GetUnixStylePath (Join-Path -Path $BuildRoot -ChildPath ("$BuildName/3rdparty/qt$($PkgName.ToLower())"))

    if (-not $ForceRebuild) {
        if (-not (CheckQt6PackageExistence -PkgName $PkgName)) {
            Write-Host "Qt6::$PkgName not found. Building..."
        }
        else {
            Write-Host "Qt6::$PkgName already exists. Skipping build."
            return
        }
    }

    if (-not (Test-Path $SrcDir)) {
        Write-Error "Package 'Qt6::$PkgName' source directory $SrcDir does not exist." -ErrorAction SilentlyContinue
        exit 1
    }

    if (-not (Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir | Out-Null
    }

    $cfgLog = Join-Path $BuildDir "Qt${PkgName}-config.log"
    $buildLog = Join-Path $BuildDir "Qt${PkgName}-build.log"
    $installLog = Join-Path $BuildDir "Qt${PkgName}-install.log"

    write-Host "================= Building Qt6::$PkgName ================="

    # 保存当前目录
    $orginalDir = Get-Location

    # 切换到构建目录
    Set-Location $BuildDir

    & $env:Qt6_DIR/bin/qt-configure-module.bat $SrcDir 2>&1 | Tee-Object -FilePath $cfgLog
    if ($LASTEXITCODE -ne 0) {
        Write-Error "qt-configure-module for Qt6::$PkgName failed, see $cfgLog for details";
        exit 1 
    }
        
    & cmake --build . --parallel 2>&1 | Tee-Object -FilePath $buildLog
    if ($LASTEXITCODE -ne 0) {
        Write-Error "cmake build for Qt6::$PkgName failed, see $buildLog for details";
        exit 1 
    }

    & cmake --install . 2>&1 | Tee-Object -FilePath $installLog
    if ($LASTEXITCODE -ne 0) {
        Write-Error "cmake install for Qt6::$PkgName failed, see $installLog for details";
        exit 1
    }

    # 恢复原始目录
    Set-Location $orginalDir

    # 最后再次检查包是否成功安装
    if (CheckQt6PackageExistence -PkgName $PkgName -Verbose $false) {
        Write-Host "Qt6::$PkgName built and installed successfully. You can use it as shown below in your CMakeLists.txt now:"
        Write-Host "    find_package(Qt6 COMPONENTS $PkgName REQUIRED)"
    }
    else {
        Write-Error "Qt6::$PkgName build or installation failed."
        exit 1
    }
}

# 检查是否在Visual Studio环境中运行
if (-not $env:VCINSTALLDIR -or -not (Get-Command "cl.exe" -ErrorAction SilentlyContinue)) {
    Write-Host "Not in Visual Studio environment. Please run this script from a Visual Studio Developer Command Prompt."
    exit 1
}

Set-Location $PSScriptRoot

ApplyDiffPatch -PatchFile "../patches/HuskarUI.diff" -TargetDir "../HuskarUI"
BuildPackage -PkgName HuskarUI -CMAKE_CONFIG_ARGS @("-DCMAKE_PREFIX_PATH=$env:Qt6_DIR", "-DBUILD_HUSKARUI_GALLERY=OFF")

BuildQt6Package -PkgName Mqtt