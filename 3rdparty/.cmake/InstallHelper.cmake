if(NOT DEFINED THIRD_PARTY_BUILD_TYPE)
    set(THIRD_PARTY_BUILD_TYPE "Release")
endif()

include_guard(DIRECTORY)

#[[
    Install third-party at configuration phase.

    install_3rd(<name>
        [SOURCE_DIR <dir>]
        [BUILD_TREE_DIR <dir>]
        [INSTALL_DIR <dir>]
        [CMAKE_PACKAGE_SUBDIR <subdir>]

        [BUILD_TYPE <type>]
        [CONFIGURE_ARGS <arg...>]

        [RESULT_PATH <VAR>]
    )
]] #
function(install_3rd _name)
    set(options)
    set(oneValueArgs SOURCE_DIR BUILD_TREE_DIR INSTALL_DIR CMAKE_PACKAGE_SUBDIR BUILD_TYPE RESULT_PATH)
    set(multiValueArgs CONFIGURE_ARGS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # 检查源目录
    if(NOT FUNC_SOURCE_DIR)
        message(FATAL_ERROR "install {_name}: SOURCE_DIR is not specified!")
    endif()
    set(_src_dir ${FUNC_SOURCE_DIR})

    # 设置构建目录
    if(FUNC_BUILD_TREE_DIR)
        set(_build_tree_dir ${FUNC_BUILD_TREE_DIR})
    else()
        set(_build_tree_dir ${CMAKE_BINARY_DIR}/_build)
    endif()
    set(_build_dir ${_build_tree_dir}/${_name})

    # 设置安装目录
    if(FUNC_INSTALL_DIR)
        set(_install_dir ${FUNC_INSTALL_DIR})
    else()
        set(_install_dir ${CMAKE_BINARY_DIR}/_install)
    endif()

    # 设置CMAKE包安装目录
    if(FUNC_CMAKE_PACKAGE_SUBDIR)
        set(_cmake_subdir ${FUNC_CMAKE_PACKAGE_SUBDIR})
    else()
        include(GNUInstallDirs)
        set(_cmake_subdir "${CMAKE_INSTALL_LIBDIR}/cmake/${_name}")
    endif()
    set(_install_cmake_dir ${_install_dir}/${_cmake_subdir})

    message(STATUS "Preparing to install 3rd-party to: ${_install_cmake_dir}")

    # 构建类型处理
    if(FUNC_BUILD_TYPE)
        set(_build_types ${FUNC_BUILD_TYPE})
        set(_build_macro -DCMAKE_BUILD_TYPE=${FUNC_BUILD_TYPE})
    elseif(CMAKE_CONFIGURATION_TYPES)
        set(_build_types ${CMAKE_CONFIGURATION_TYPES})
        set(_build_macro)
    elseif(CMAKE_BUILD_TYPE)
        set(_build_types ${CMAKE_BUILD_TYPE})
        set(_build_macro -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
    else()
        set(_build_types ${THIRD_PARTY_BUILD_TYPE})
        set(_build_macro -DCMAKE_BUILD_TYPE=${THIRD_PARTY_BUILD_TYPE})
    endif()

    if(NOT IS_DIRECTORY ${_install_cmake_dir})
        # 检测生成器和平台
        set(_extra_args)
        if(CMAKE_GENERATOR)
            set(_extra_args -G "${CMAKE_GENERATOR}")
        endif()
        if(CMAKE_GENERATOR_PLATFORM)
            set(_extra_args -A "${CMAKE_GENERATOR_PLATFORM}")
        endif()

        # 清理旧的构建目录
        if(IS_DIRECTORY ${_build_dir})
            file(REMOVE_RECURSE ${_build_dir})
        endif()
        file(MAKE_DIRECTORY ${_build_tree_dir})

        # 配置
        message(STATUS "  Configuring ${_name}...")
        set(_log_file ${_build_tree_dir}/${_name}_configure.log)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S ${_src_dir} -B ${_build_dir} ${_extra_args} ${_build_macro}
            "-DCMAKE_INSTALL_PREFIX=${_install_dir}" ${FUNC_CONFIGURE_ARGS}
            OUTPUT_FILE ${_log_file}
            ERROR_FILE ${_log_file}
            RESULT_VARIABLE _code
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        if(NOT ${_code} EQUAL 0)
            message(FATAL_ERROR "Configure failed, check \"${_log_file}\"")
        endif()

        foreach(_item IN LISTS _build_types)
            # 构建
            message(STATUS "  Building ${_name} (${_item})...")
            set(_log_file ${_build_tree_dir}/${_name}_build-${_item}.log)
            execute_process(
                COMMAND ${CMAKE_COMMAND} --build ${_build_dir} --config ${_item} --parallel
                OUTPUT_FILE ${_log_file}
                ERROR_FILE ${_log_file}
                RESULT_VARIABLE _code
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            )
            if(NOT ${_code} EQUAL 0)
                message(FATAL_ERROR "Build failed, check \"${_log_file}\"")
            endif()

            # 安装
            message(STATUS "  Installing ${_name} (${_item})...")
            set(_log_file ${_build_tree_dir}/${_name}_install-${_item}.log)
            execute_process(
                COMMAND ${CMAKE_COMMAND} --build ${_build_dir} --config ${_item} --target install
                OUTPUT_FILE ${_log_file}
                ERROR_FILE ${_log_file}
                RESULT_VARIABLE _code
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            )
            if(NOT ${_code} EQUAL 0)
                message(FATAL_ERROR "Install failed, check \"${_log_file}\"")
            endif()
        endforeach()
    endif()

    if(FUNC_RESULT_PATH)
        set(${FUNC_RESULT_PATH} ${_install_cmake_dir} PARENT_SCOPE)
    endif()

    message(STATUS "Install 3rd-party: ${_name} done.")
    message(STATUS "You can use \"find_package(${_name} CONFIG REQUIRED)\" to find it.")
endfunction()

function(apply_patch _patch_file _target_dir)
    if(NOT IS_DIRECTORY ${_target_dir})
        message(FATAL_ERROR "apply_patch: Target directory \"${_target_dir}\" does not exist!")
    endif()
    if(NOT EXISTS ${_patch_file})
        message(FATAL_ERROR "apply_patch: Patch file \"${_patch_file}\" does not exist!")
    endif()

    execute_process(
        COMMAND git apply --check --quiet "${_patch_file}"
        WORKING_DIRECTORY ${_target_dir}
        RESULT_VARIABLE _code
    )
    if(NOT ${_code} EQUAL 0)
        return()
    endif()
    message(STATUS "Applying patch \"${_patch_file}\" to \"${_target_dir}\"...")
    execute_process(
        COMMAND git apply --ignore-space-change --whitespace=nowarn "${_patch_file}"
        WORKING_DIRECTORY ${_target_dir}
        RESULT_VARIABLE _code
    )
    if(NOT ${_code} EQUAL 0)
        message(FATAL_ERROR "apply_patch: Failed to apply patch \"${_patch_file}\" to \"${_target_dir}\"!")
    endif()
endfunction()