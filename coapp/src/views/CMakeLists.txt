qt_add_library(CoApp_views STATIC)

target_sources(CoApp_views PRIVATE
    BandView.h BandView.cpp
    CameraView.h CameraView.cpp
    DeviceView.h DeviceView.cpp
    EEGView.h EEGView.cpp
    LoginDialog.h LoginDialog.cpp
    MainWindow.h MainWindow.cpp
    NavigationView.h NavigationView.cpp
    SettingsDialog.h SettingsDialog.cpp
    UserInfoDialog.h UserInfoDialog.cpp
)

add_subdirectory(navigation)

target_include_directories(CoApp_views
    PRIVATE ${APP_SRC_DIR}
)

target_link_libraries(CoApp_views
    PUBLIC Qt6::Core PlatformCommon CoApp_components
)