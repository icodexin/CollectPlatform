cmake_minimum_required(VERSION 3.20)

project(APP_Student VERSION 0.1)

set(TARGET_OUTPUT_NAME "Student APP")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#find_package(msgpack-cxx CONFIG REQUIRED)

find_package(Qt6 COMPONENTS Core Widgets Network Mqtt Multimedia MultimediaWidgets REQUIRED)
qt_standard_project_setup(REQUIRES 6.5)

set(H_FILES
    src/services/EEGDataReceiver.h
    src/services/WristbandServer.h
    src/services/WristbandHandler.h
    src/services/MqttPublisher.h
    src/services/DataQueue.h
    src/services/SettingsManager.h
    src/windows/MainWindow.h
    src/components/BarCard.h
    src/components/BaseWidget.h
    src/components/IPv4Edit.h
    src/components/LogEdit.h
    src/components/StatusIndicator.h
    src/views/MqttPublishView.h
    src/views/SettingView.h
    src/views/LogView.h
    src/views/CameraView.h
)

set(CPP_FILES
    main.cpp
    src/services/EEGDataReceiver.cpp
    src/services/WristbandServer.cpp
    src/services/WristbandHandler.cpp
    src/services/MqttPublisher.cpp
    src/services/SettingsManager.cpp
    src/windows/MainWindow.cpp
    src/components/BarCard.cpp
    src/components/BaseWidget.cpp
    src/components/IPv4Edit.cpp
    src/components/LogEdit.cpp
    src/components/StatusIndicator.cpp
    src/views/MqttPublishView.cpp
    src/views/SettingView.cpp
    src/views/LogView.cpp
    src/views/CameraView.cpp
)

qt_add_resources(RESOURCES resources/resources.qrc)

qt_add_executable(${PROJECT_NAME}
    ${H_FILES} ${CPP_FILES} ${RESOURCES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${TARGET_OUTPUT_NAME}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/MacOS/Info.plist.in"
    MACOSX_BUNDLE TRUE
    #    WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Widgets Qt6::Network Qt6::Mqtt Qt6::Multimedia Qt6::MultimediaWidgets common
)