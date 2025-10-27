cmake_minimum_required(VERSION 3.20)

if (NOT APP_VERSION)
    set(APP_VERSION "0.1.0")
endif()

project(MuApp VERSION ${APP_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 COMPONENTS Core Quick Qml Graphs REQUIRED)
find_package(HuskarUI CONFIG REQUIRED)
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
    ui/Components/MuFlickablePage.qml
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
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Quick Qt6::Qml Qt6::Graphs HuskarUIBasic PlatformCommon
)