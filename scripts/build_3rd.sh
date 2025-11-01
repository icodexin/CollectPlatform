#!/bin/bash

pkg_names=("HuskarUI" "Qt6Mqtt")
build_types=("Debug" "Release")
generator="Ninja"
build_root="../build"
install_dir="../output/3rdparty"

function log_info() {
    printf "$1\n"
}
function log_tip() {
    printf "\033[36m$1\033[0m\n"
}
function log_warn() {
    printf "\033[33m$1\033[0m\n"
}
function log_success() {
    printf "\033[32m$1\033[0m\n"
}
function log_error() {
    printf "\033[31m[ERROR] $1\033[0m\n" >&2
}

function show_help() {
    cat <<EOF
Usage: $(basename "$0") [options]

Options:
  --pkg-names <name1,name2,...>    要构建的三方包列表（逗号分隔），注意 Qt6 Additional 包需以 Qt6 前缀开头，默认构建所有支持的包
  --build-types <t1,t2,...>        构建类型（逗号分隔），默认构建 Debug 和 Release
  --generator | -G <name>          CMake 生成器名称，默认使用Ninja，可选值包括 "Ninja" 和 "Ninja Multi-Config"
  --build-root | -B <path>         构建输出根目录（相对于仓库根路径）
  --install-dir <path>             安装目标目录（相对于仓库根路径），Qt6 Additional 包不受此参数影响，将始终安装在 QT6_DIR 指定的目录下
  --help | -h                      显示此帮助并退出

支持构建的三方包列表：
  HuskarUI, Qt6Mqtt

示例:
  $(basename "$0") --pkg-names HuskarUI --build_types Debug --generator Ninja
EOF
}

# 解析命令行参数，覆盖默认值
while [[ $# -gt 0 ]]; do
    case "$1" in
        --pkg-names)
            shift
            if [[ -z "$1" ]]; then
                log_error "--pkg-names requires a value"
                exit 1
            fi
            IFS=',' read -ra pkg_names <<< "$1"
            shift
            ;;
        --build-types)
            shift
            if [[ -z "$1" ]]; then
                log_error "--build-types requires a value"
                exit 1
            fi
            IFS=',' read -ra build_types <<< "$1"
            shift
            ;;
        --generator|-G)
            shift
            if [[ -z "$1" ]]; then
                log_error "--generator requires a value"
                exit 1
            fi
            generator="$1"
            shift
            ;;
        --build-root|-B)
            shift
            if [[ -z "$1" ]]; then
                log_error "--build-root requires a value"
                exit 1
            fi
            build_root="$1"
            shift
            ;;
        --install-dir)
            shift
            if [[ -z "$1" ]]; then
                log_error "--install-dir requires a value"
                exit 1
            fi
            install_dir="$1"
            shift
            ;;
        --help|-h)
            show_help
            exit 0
            ;;
        *)
            log_warn "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

function get_host_arch() {
    local arch=$(uname -m)
    case "$arch" in
        x86_64)
            echo "x64"
            ;;
        aarch64 | arm64)
            echo "arm64"
            ;;
        *)
            log_error "Unsupported architecture: $arch"
            exit 1
            ;;
    esac
}


function join_paths() {
    local path=""
    for p in "$@"; do
        p="${p%/}"  # Remove trailing slash
        if [[ -z "$path" ]]; then
            path="$p"
        else
            p="${p#/}"  # Remove leading slash
            path="$path/$p"
        fi
    done
    echo "$path"
}

function apply_diffpatch() {
    local patch_file=$1
    local target_dir=$2

    if [[ -z "$patch_file" || -z "$target_dir" ]]; then
        log_error "Usage: ApplyDiffPatch <PatchFile> <TargetDir>"
        return 1
    fi

    if [[ ! -f "$patch_file" ]]; then
        log_error "Patch file $patch_file does not exist"
        exit 1
    fi
    if [[ ! -d "$target_dir" ]]; then
        log_error "Target directory $target_dir does not exist"
        exit 1
    fi

    patch_file="$(realpath "$patch_file")"
    target_dir="$(realpath "$target_dir")"

    git -C "$target_dir" apply --check --quiet "$patch_file"
    if [[ $? -ne 0 ]]; then
        log_info "Patch $patch_file has already been applied to $target_dir, skipped."
        return
    fi

    log_info "Applying patch $patch_file to $target_dir ..."
    git -C "$target_dir" apply --ignore-space-change --whitespace=nowarn "$patch_file"
    if [[ $? -ne 0 ]]; then
        log_error "Failed to apply patch $patch_file to $target_dir"
        exit 1
    fi
}

function get_qt6_dir() {
    if [[ -z "$QT6_DIR" ]]; then
        log_error "QT6_DIR is not set. Please set QT6_DIR to the Qt6 installation directory."
        exit 1
    fi
    echo "$QT6_DIR"
}

function get_pkg_fullname() {
    local pkg_name=$1
    local pkg_type=${2:-""}

    local fullname="$pkg_name"
    if [[ "$pkg_name" == "Qt6"* ]]; then
        fullname="Qt6::${pkg_name:3}"
    fi
    if [[ -n "$pkg_type" ]]; then
        fullname="${fullname} ($pkg_type)"
    fi

    echo "$fullname"
}

function cmake_configure() {
    local pkg_name=""
    local pkg_src_dir=""
    local pkg_build_dir=""
    local pkg_install_dir=""
    local pkg_build_type=""
    local -a cmake_config_args=()

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --name|-n)
                pkg_name="$2"
                shift 2
                ;;
            --src-dir|-s)
                pkg_src_dir="$2"
                shift 2
                ;;
            --build-dir|-b)
                pkg_build_dir="$2"
                shift 2
                ;;
            --install-dir|-i)
                pkg_install_dir="$2"
                shift 2
                ;;
            --build-type|-t)
                pkg_build_type="$2"
                shift 2
                ;;
            --extra-args)
                shift
                while [[ $# -gt 0 && "$1" != --* ]]; do
                    cmake_config_args+=("$1")
                    shift
                done
                ;;
            *)
                log_error "Unknown argument: $1"
                exit 1
                ;;
        esac
    done

    if [[ -z "$pkg_name" || -z "$pkg_src_dir" || -z "$pkg_build_dir" || -z "$pkg_install_dir" ]]; then
        log_error "cmake_configure requires --name, --src-dir, --build-dir, and --install-dir arguments"
        exit 1
    fi

    if [[ ! -d "$pkg_build_dir" ]]; then
        mkdir -p "$pkg_build_dir"
    fi

    local log_file=$(join_paths "$pkg_build_dir" "$pkg_name-config.log")
    local pkg_fullname=$(get_pkg_fullname "$pkg_name" "$pkg_build_type")

    log_tip "==================== Configuring $pkg_fullname ===================="

    # 切换到构建目录
    cd "$pkg_build_dir" || exit 1

    if [[ "$pkg_name" == "Qt6"* ]]; then
        "$QT6_DIR/bin/qt-configure-module" $pkg_src_dir 2>&1 | tee "$log_file"
        if [[ $? -ne 0 ]]; then
            log_error "qt-configure-module for $pkg_fullname failed, see more details at $log_file"
            exit 1
        fi
    else
        cmake -S "$pkg_src_dir" -B "$pkg_build_dir" -DCMAKE_INSTALL_PREFIX="$pkg_install_dir" "${cmake_config_args[@]}" 2>&1 | tee "$log_file"

        if [[ $? -ne 0 ]]; then
            log_error "cmake configure for $pkg_fullname failed, see more details at $log_file"
            exit 1
        fi
    fi
}

function cmake_build() {
    local pkg_name=""
    local pkg_build_dir=""
    local pkg_build_type=""

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --name|-n)
                pkg_name="$2"
                shift 2
                ;;
            --build-dir|-b)
                pkg_build_dir="$2"
                shift 2
                ;;
            --build-type|-t)
                pkg_build_type="$2"
                shift 2
                ;;
            *)
                log_error "Unknown argument: $1"
                exit 1
                ;;
        esac
    done

    if [[ -z "$pkg_name" || -z "$pkg_build_dir" ]]; then
        log_error "cmake_build requires --name and --build-dir arguments"
        exit 1
    fi

    if [[ ! -d "$pkg_build_dir" ]]; then
        log_error "Build directory $pkg_build_dir does not exist."
        exit 1
    fi

    local log_file=$(join_paths "$pkg_build_dir" "$pkg_name-build.log")
    local pkg_fullname=$(get_pkg_fullname "$pkg_name" "$pkg_build_type")

    log_tip "==================== Building $pkg_fullname ===================="

    # 切换到构建目录
    cd "$pkg_build_dir" || exit 1

    if [[ -z "$pkg_build_type" ]]; then
        config_arg=()
    else
        config_arg=("--config" "$pkg_build_type")
    fi

    cmake --build . "${config_arg[@]}" --parallel 2>&1 | tee "$log_file"

    if [[ $? -ne 0 ]]; then
        log_error "cmake build for $pkg_fullname failed, see more details at $log_file"
        exit 1
    fi
}

function cmake_install() {
    local pkg_name=""
    local pkg_build_dir=""
    local pkg_build_type=""

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --name|-n)
                pkg_name="$2"
                shift 2
                ;;
            --build-dir|-b)
                pkg_build_dir="$2"
                shift 2
                ;;
            --build-type|-t)
                pkg_build_type="$2"
                shift 2
                ;;
            *)
                log_error "Unknown argument: $1"
                exit 1
                ;;
        esac
    done

    if [[ -z "$pkg_name" || -z "$pkg_build_dir" ]]; then
        log_error "cmake_install requires --name and --build-dir arguments"
        exit 1
    fi

    if [[ ! -d "$pkg_build_dir" ]]; then
        log_error "Build directory $pkg_build_dir does not exist."
        exit 1
    fi

    local log_file=$(join_paths "$pkg_build_dir" "$pkg_name-install.log")
    local pkg_fullname=$(get_pkg_fullname "$pkg_name" "$pkg_build_type")

    log_tip "==================== Installing $pkg_fullname ===================="

    # 切换到构建目录
    cd "$pkg_build_dir" || exit 1

    if [[ -z "$pkg_build_type" ]]; then
        config_arg=()
    else
        config_arg=("--config" "$pkg_build_type")
    fi

    cmake --build . "${config_arg[@]}" --target install 2>&1 | tee "$log_file"

    if [[ $? -ne 0 ]]; then
        log_error "cmake install for $pkg_fullname failed, see more details at $log_file"
        exit 1
    fi
}

function build_package() {
    local pkg_name
    local -a cmake_config_args=()

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --name|-n)
                pkg_name="$2"
                shift 2
                ;;
            --extra-config-args)
                shift
                while [[ $# -gt 0 && "$1" != --* ]]; do
                    cmake_config_args+=("$1")
                    shift
                done
                ;;
            *)
                log_error "Unknown argument: $1"
                exit 1
                ;;
        esac
    done

    if [[ -z "$pkg_name" ]]; then
        log_error "build_package requires --name argument"
        exit 1
    fi

    local qt6_pkg=$([[ "$pkg_name" == Qt6* ]] && echo true || echo false)
    if [[ $qt6_pkg == true ]]; then
        pkg_dirname=$(echo "qt${pkg_name:3}" | tr '[:upper:]' '[:lower:]')
        local -a pkg_build_types=("") # Qt6 包构建不受 BuildTypes 参数影响，内部自动构建 Debug 和 RelWithDebInfo 版本
        local pkg_install_dir=$(get_qt6_dir) # Qt6 包始终安装在系统 Qt6 目录下
    else
        pkg_dirname="$pkg_name"
        local -a pkg_build_types=("${build_types[@]}")
        local pkg_install_dir="${install_dir}"
    fi
    local pkg_src_dir="${repo_root}/3rdparty/${pkg_dirname}"

    if [[ ! -d "$pkg_src_dir" ]]; then
        log_error "Package $(get_pkg_fullname "$pkg_name") source directory $pkg_src_dir does not exist."
        exit 1
    fi

    if [[ $generator == "Ninja" ]]; then
        cmake_config_args+=("-G" "Ninja")
        cmake_config_args+=("-DCMAKE_BUILD_TYPE=${pkg_build_type}")
        for build_type in "${pkg_build_types[@]}"; do
            if [[ $qt6_pkg == true ]]; then
                local build_name="$(echo "${build_types[0]}" | tr '[:upper:]' '[:lower:]')-macos-$(get_host_arch)" # 取第一个构建类型作为目录名
            else
                local build_name="$(echo "$build_type" | tr '[:upper:]' '[:lower:]')-macos-$(get_host_arch)" # 取当前构建类型作为目录名, e.g. debug-macos-arm64
            fi
            local pkg_build_dir="${build_root}/${build_name}/${pkg_dirname}"
            cmake_configure --name "$pkg_name" --src-dir "$pkg_src_dir" --build-dir "$pkg_build_dir" --install-dir "$pkg_install_dir" \
                --build-type "$build_type" --extra-args "${cmake_config_args[@]}"
            cmake_build --name "$pkg_name" --build-dir "$pkg_build_dir" --build-type "$build_type"
            cmake_install --name "$pkg_name" --build-dir "$pkg_build_dir" --build-type "$build_type"
        done
    elif [[ $generator == "Ninja Multi-Config" ]]; then
        cmake_config_args+=("-G" "Ninja Multi-Config")
        local build_name="macos-$(get_host_arch)" # e.g. macos-arm64
        local pkg_build_dir="${build_root}/${build_name}/${pkg_dirname}"
        cmake_configure --name "$pkg_name" --src-dir "$pkg_src_dir" --build-dir "$pkg_build_dir" --install-dir "$pkg_install_dir" \
            --extra-args "${cmake_config_args[@]}"
        for build_type in "${pkg_build_types[@]}"; do
            cmake_build --name "$pkg_name" --build-dir "$pkg_build_dir" --build-type "$build_type"
            cmake_install --name "$pkg_name" --build-dir "$pkg_build_dir" --build-type "$build_type"
        done
    else
        log_error "Unsupported CMake generator: $generator"
        exit 1
    fi
}

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(realpath "$script_dir/..")"
build_root="$repo_root/$build_root"
install_dir="$repo_root/$install_dir"

cd "$repo_root" || exit 1

for pkg_name in "${pkg_names[@]}"; do
    (
        case "$pkg_name" in
            "HuskarUI")
                apply_diffpatch "./3rdparty/patches/huskarui.diff" "./3rdparty/HuskarUI"
                build_package --name "$pkg_name" --extra-config-args "-DCMAKE_PREFIX_PATH=${QT6_DIR}" "-DBUILD_HUSKARUI_GALLERY=OFF"
                ;;
            "Qt6Mqtt")
                build_package --name "$pkg_name"
                ;;
            *)
                log_warn "No support for package: $pkg_name, skipped."
                ;;
        esac
    )
done