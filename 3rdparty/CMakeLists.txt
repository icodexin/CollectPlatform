include(".cmake/InstallHelper.cmake")

find_package(HuskarUI CONFIG QUIET)
if(NOT HuskarUI_FOUND)
    set(HuskarUI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/HuskarUI)

    apply_patch(${CMAKE_CURRENT_SOURCE_DIR}/patches/HuskarUI.diff ${HuskarUI_SOURCE_DIR})

    set(HuskarUI_PACKAGE_PATH)
    install_3rd(HuskarUI
        SOURCE_DIR ${HuskarUI_SOURCE_DIR}
        BUILD_TREE_DIR ${CMAKE_BINARY_DIR}/3rdparty
        INSTALL_DIR ${THIRD_INSTALL_DIR}
        CONFIGURE_ARGS
            -DCMAKE_PREFIX_PATH=$ENV{QT6_DIR}
            -DBUILD_HUSKARUI_GALLERY=OFF
        RESULT_PATH HuskarUI_PACKAGE_PATH
    )
    find_package(HuskarUI REQUIRED PATHS ${HuskarUI_PACKAGE_PATH})
    set(HuskarUI_DIR ${HuskarUI_PACKAGE_PATH} CACHE PATH "" FORCE)
else ()
    message(STATUS "HuskarUI already installed at: ${HuskarUI_DIR}")
    message(STATUS "You can use \"find_package(HuskarUI CONFIG REQUIRED)\" to find it.")
endif()

add_subdirectory(qtmqtt)