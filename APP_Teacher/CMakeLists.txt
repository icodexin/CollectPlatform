cmake_minimum_required(VERSION 3.20)

project(APP_Teacher)

set(TARGET_OUTPUT_NAME "APP")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Quick Qml Core5Compat Graphs REQUIRED)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_resources(RESOURCES resources/resources.qrc)

qt_add_executable(${PROJECT_NAME}
    main.cpp ${RESOURCES} resources/win_res.rc
)

qt_policy(SET QTP0004 NEW)

target_include_directories( ${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/services
)

qt_add_qml_module(${PROJECT_NAME}
    URI TeacherApp
    VERSION 1.0
    OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    RESOURCE_PREFIX "/"
    SOURCES
        src/QmlWatcher.h src/QmlWatcher.cpp
        src/MinMaxQueue.h src/MinMaxQueue.cpp
        src/services/WristbandData.h
        src/services/WristbandClientHandler.h src/services/WristbandClientHandler.cpp
        src/services/WristbandListener.h src/services/WristbandListener.cpp
    QML_FILES
        qml/App.qml
        qml/Controls/MyIconButton.qml
        qml/Controls/MyPage.qml
        qml/Controls/NumberInputer.qml
        qml/Controls/Charts/MultiLineChart.qml
        qml/Controls/Charts/MyDateTimeAxis.qml
        qml/Controls/Charts/MyGraphsTheme.qml
        qml/Controls/Charts/MyValueAxis.qml
        qml/Controls/Charts/SingleLineChart.qml
        qml/Controls/Charts/util.js
        qml/Pages/HomePage.qml
        qml/Pages/OfflinePage.qml
        qml/Pages/RealTimePage.qml
        qml/Pages/SettingPage.qml
        qml/Pages/StoragePage.qml
        qml/Views/WristbandView.qml
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${TARGET_OUTPUT_NAME}
#    WIN32_EXECUTABLE TRUE
)

#target_include_directories(${PROJECT_NAME}
#    PRIVATE DelegateUI/include
#)
#
#target_link_directories(${PROJECT_NAME}
#    PRIVATE DelegateUI/lib
#)

target_compile_definitions(
    ${PROJECT_NAME} PRIVATE QT_QML_DEBUG
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Quick Qt6::Qml Qt6::Core5Compat Qt6::Graphs DelegateUI
)