qt_add_library(CoApp_services STATIC)

target_sources(CoApp_services
    PRIVATE
    AuthService.h AuthService.cpp
    BandServer.h BandServer.cpp
    CameraService.h CameraService.cpp
    CoSettingsMgr.h CoSettingsMgr.cpp
    DataPipe.h DataPipe.cpp
    DataSerializer.h DataSerializer.cpp
    EEGRecvService.h EEGRecvService.cpp
    FFmpegHelper.h FFmpegHelper.cpp
    log.h log.cpp
    MqttPublisher.h MqttPublisher.cpp
    UserApi.h UserApi.cpp
    VideoPushService.h VideoPushService.cpp
)

target_include_directories(CoApp_services
    PRIVATE ${APP_SRC_DIR}
)

target_link_libraries(CoApp_services
    PUBLIC Qt6::Core Qt6::Network Qt6::Mqtt Qt6::Multimedia Qt6::SerialPort PlatformCommon ${FFMPEG_LIBRARIES}
)