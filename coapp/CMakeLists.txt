cmake_minimum_required(VERSION 3.20)

if (NOT APP_VERSION)
    set(APP_VERSION "1.0.0")
endif ()

project(CoApp VERSION ${APP_VERSION} LANGUAGES CXX)

set(TARGET_OUTPUT_NAME "Collection App")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(FFMPEG REQUIRED)   # put FFMPEG before QT to avoid some lib be hidden
find_package(Qt6 COMPONENTS Core Widgets Svg Network Mqtt Multimedia SerialPort LinguistTools REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

qt_standard_project_setup(REQUIRES 6.5 I18N_TRANSLATED_LANGUAGES zh_CN)

qt_add_executable(${PROJECT_NAME}
    main.cpp
)

add_subdirectory(src)
add_subdirectory(resources)

if (Qt6_VERSION VERSION_GREATER_EQUAL "6.9.0")
    qt_add_translations(CoApp
        TS_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/i18n
    )
else ()
    qt_add_translations(CoApp
        TS_FILE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources/i18n
    )
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    APP_VERSION="${APP_VERSION}"
)

set(APP_PACKAGE_DIR ${APP_OUTPUT_DIRECTORY}/coapp)
if (APPLE)
    set(AI_SERVICE_OUTPUT_DIR ${APP_PACKAGE_DIR}/${TARGET_OUTPUT_NAME}.app/Contents/MacOS)
else ()
    set(AI_SERVICE_OUTPUT_DIR ${APP_PACKAGE_DIR})
endif ()
set(AI_SERVICE_BUILD_DIR ${CMAKE_BINARY_DIR}/ai_service_build/$<CONFIG>)

add_custom_target(Build_${PROJECT_NAME}_AiService
    COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/ai_service/build_ai_service.py
        --service-dir ${CMAKE_SOURCE_DIR}/ai_service
        --output-dir ${AI_SERVICE_OUTPUT_DIR}
        --build-dir ${AI_SERVICE_BUILD_DIR}
        --name ai_service
    COMMENT "Building ai_service executable for ${PROJECT_NAME} ..."
    VERBATIM
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${TARGET_OUTPUT_NAME}
    RUNTIME_OUTPUT_DIRECTORY ${APP_PACKAGE_DIR}
    MACOSX_BUNDLE_GUI_IDENTIFIER "edu.perlab.CoApp"
    MACOSX_BUNDLE_COPYRIGHT "Copyright©2025 CCNU PerLab. All rights reserved."
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/macOS/Info.plist.in"
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

add_dependencies(${PROJECT_NAME} Build_${PROJECT_NAME}_AiService)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core Qt6::Widgets CoApp_components CoApp_services CoApp_views
)

if (WIN32)
    find_program(QT_DEPLOY_QT NAMES windeployqt)
    set(QT_DEPLOY_ARGS
        ${APP_PACKAGE_DIR}/${TARGET_OUTPUT_NAME}.exe
        --translations en,zh_CN
        --translationdir i18n
        $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:--debug>
        $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>>:--release>
    )
elseif (APPLE)
    find_program(QT_DEPLOY_QT NAMES macdeployqt)
    set(QT_DEPLOY_ARGS
        ${APP_PACKAGE_DIR}/${TARGET_OUTPUT_NAME}.app
    )
endif ()

if (QT_DEPLOY_QT)
    add_custom_target(Deploy_${PROJECT_NAME} ALL
        COMMAND ${QT_DEPLOY_QT} ${QT_DEPLOY_ARGS}
        COMMENT "Deploying dependencies for ${PROJECT_NAME} after build ..."
        WORKING_DIRECTORY ${APP_PACKAGE_DIR}
        DEPENDS ${PROJECT_NAME}
    )
endif ()
