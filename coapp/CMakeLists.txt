cmake_minimum_required(VERSION 3.20)

project(CoApp VERSION 0.1.1)

set(TARGET_OUTPUT_NAME "Collection App")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 COMPONENTS Core Widgets Svg Network Mqtt Multimedia MultimediaWidgets REQUIRED)
qt_standard_project_setup(REQUIRES 6.5)

set(H_FILES
    src/views/MainWindow.h
    src/views/DeviceView.h
    src/views/SettingView.h
    src/views/EEGView.h
    src/views/BandView.h
    src/views/CameraView.h
    src/components/BaseWidget.h
    src/components/BarCard.h
    src/components/IPv4Edit.h
    src/components/LogBox.h
    src/components/RecordingIndicator.h
    src/components/SvgIconHelper.h
    src/services/log.h
    src/services/CameraService.h
    src/services/BandServer.h
    src/services/EEGReceiver.h
    src/services/MqttPublisher.h
    src/services/SettingsManager.h
)

set(CPP_FILES
    main.cpp
    src/views/MainWindow.cpp
    src/views/DeviceView.cpp
    src/views/SettingView.cpp
    src/views/EEGView.cpp
    src/views/BandView.cpp
    src/views/CameraView.cpp
    src/components/BaseWidget.cpp
    src/components/BarCard.cpp
    src/components/IPv4Edit.cpp
    src/components/LogBox.cpp
    src/components/RecordingIndicator.cpp
    src/components/SvgIconHelper.cpp
    src/services/log.cpp
    src/services/CameraService.cpp
    src/services/BandServer.cpp
    src/services/EEGReceiver.cpp
    src/services/MqttPublisher.cpp
    src/services/SettingsManager.cpp
)

qt_add_resources(RESOURCES resources/resources.qrc)

qt_add_executable(${PROJECT_NAME}
    ${H_FILES} ${CPP_FILES} ${RESOURCES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(PACKAGE_OUTPUT_DIRECTORY ${APP_OUTPUT_DIRECTORY}/coapp)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${TARGET_OUTPUT_NAME}
    RUNTIME_OUTPUT_DIRECTORY ${PACKAGE_OUTPUT_DIRECTORY}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/macOS/Info.plist.in"
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Widgets Qt6::Svg Qt6::Network Qt6::Mqtt Qt6::Multimedia Qt6::MultimediaWidgets PlatformCommon
)

if (WIN32)
    find_program(QT_DEPLOY_QT NAMES windeployqt)
    set(APP_EXE_PATH ${PACKAGE_OUTPUT_DIRECTORY}/${TARGET_OUTPUT_NAME}.exe)
elseif(APPLE)
    find_program(QT_DEPLOY_QT NAMES macdeployqt)
    set(APP_EXE_PATH ${PACKAGE_OUTPUT_DIRECTORY}/${TARGET_OUTPUT_NAME}.app)
endif()

if (QT_DEPLOY_QT)
    set(QT_DEPLOY_ARGS
        ${APP_EXE_PATH}
        --translations en,zh_CN
        --translationdir i18n
    )
    add_custom_target(Deploy_${PROJECT_NAME}
        COMMAND ${QT_DEPLOY_QT} ${QT_DEPLOY_ARGS}
        COMMENT "Deploying Qt6 dependencies for ${PROJECT_NAME} after build ..."
        WORKING_DIRECTORY ${PACKAGE_OUTPUT_DIRECTORY}
        DEPENDS ${PROJECT_NAME}
    )
endif()