cmake_minimum_required(VERSION 3.20)

if (NOT APP_VERSION)
    set(APP_VERSION "0.1.0")
endif()

project(MuApp VERSION ${APP_VERSION} LANGUAGES CXX)

set(TARGET_OUTPUT_NAME "Multimodal App")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 COMPONENTS Core Quick Qml Graphs REQUIRED)
find_package(HuskarUI REQUIRED)
qt_standard_project_setup(REQUIRES 6.5)

set(CPP_FILES
    main.cpp
)

qt_add_resources(RESOURCES resources/resources.qrc)

qt_add_executable(${PROJECT_NAME}
    ${CPP_FILES} ${RESOURCES}
)

qt_policy(SET QTP0004 NEW)

set(QML_FILES
    App.qml
    AppController.qml
    ui/Components/MuPage.qml
    ui/Components/MuScrollablePage.qml
    ui/Components/MuPanel.qml
    ui/Components/Charts/MuDateTimeAxis.qml
    ui/Components/Charts/MuValueAxis.qml
    ui/Components/Charts/MuGraphsTheme.qml
    ui/Components/Charts/MuLineSeries.qml
    ui/Components/Charts/SingleLineChart.qml
    ui/Components/Charts/MultiLineChart.qml
    ui/Components/Charts/util.js
    ui/Views/EEGView.qml
    ui/Views/BandView.qml
    ui/Pages/HomePage.qml
    ui/Pages/RealtimePage.qml
    ui/Pages/OfflinePage.qml
    ui/Pages/StoragePage.qml
    ui/Pages/SettingPage.qml
)

set(QML_CPP_FILES
    src/models/MinMaxQueue.h src/models/MinMaxQueue.cpp
    src/models/BandViewFrame.h src/models/BandViewFrame.cpp
    src/BandViewController.h src/BandViewController.cpp
)

set_source_files_properties(AppController.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(${PROJECT_NAME}
    URI MuApp
        VERSION 1.0
    QML_FILES ${QML_FILES}
    SOURCES ${QML_CPP_FILES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    APP_VERSION="${APP_VERSION}"
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

set(APP_PACKAGE_DIR ${APP_OUTPUT_DIRECTORY}/muapp)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${TARGET_OUTPUT_NAME}
    RUNTIME_OUTPUT_DIRECTORY ${APP_PACKAGE_DIR}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE # 如果使用CLion / VS Code等IDE进行调试时无法看到js console输出，则需要屏蔽此属性
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Quick Qt6::Qml Qt6::Graphs HuskarUI::Basic PlatformCommon
)

if (WIN32)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${HuskarUI_QML_IMPORT_PATH} ${APP_PACKAGE_DIR}/qml
    )

    find_program(QT_DEPLOY_QT NAMES windeployqt)
    set(QT_DEPLOY_ARGS
        ${APP_PACKAGE_DIR}/${TARGET_OUTPUT_NAME}.exe
        $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:--debug>
        $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>>:--release>
        --qmldir=${CMAKE_CURRENT_SOURCE_DIR}
        --no-widgets
        --no-quickcontrols2windowsstyleimpl
        --no-quickcontrols2fluentwinui3styleimpl
        --no-quickcontrols2fusion
        --no-quickcontrols2fusionstyleimpl
        --no-quickcontrols2material
        --no-quickcontrols2materialstyleimpl
        --no-quickcontrols2universal
        --no-quickcontrols2universalstyleimpl
        --no-quickcontrols2imagine
        --no-quickcontrols2imaginestyleimpl
        --translations en,zh_CN
        --translationdir i18n
    )
    set(DEBUG_SUFFIX $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:d>)
    set(THIRD_DLIBS
        ${HuskarUI_DIR}/../../../bin/HuskarUIBasic${DEBUG_SUFFIX}.dll
    )
    add_custom_target(Deploy_${PROJECT_NAME} ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Copy Qt6 dependencies ..."
        COMMAND ${QT_DEPLOY_QT} ${QT_DEPLOY_ARGS}
        COMMAND ${CMAKE_COMMAND} -E echo "Copy 3rd party dependencies ..."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${THIRD_DLIBS} ${APP_PACKAGE_DIR}
        COMMENT "Deploying dependencies for ${PROJECT_NAME} after build ..."
        WORKING_DIRECTORY ${APP_PACKAGE_DIR}
        DEPENDS ${PROJECT_NAME}
    )
elseif(APPLE)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${HuskarUI_QML_IMPORT_PATH} ${APP_PACKAGE_DIR}/${TARGET_OUTPUT_NAME}.app/Contents/Resources/qml
    )

    find_program(QT_DEPLOY_QT NAMES macdeployqt)
    set(QT_DEPLOY_ARGS
        ${APP_PACKAGE_DIR}/${TARGET_OUTPUT_NAME}.app
        $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-no-strip>
        -qmldir=${CMAKE_CURRENT_SOURCE_DIR}
        -appstore-compliant
    )
    add_custom_target(Deploy_${PROJECT_NAME} ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Copy Qt6 dependencies ..."
        COMMAND ${QT_DEPLOY_QT} ${QT_DEPLOY_ARGS}
        COMMENT "Deploying dependencies for ${PROJECT_NAME} after build ..."
        WORKING_DIRECTORY ${APP_PACKAGE_DIR}
        DEPENDS ${PROJECT_NAME}
    )
endif()
