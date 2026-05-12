qt_add_library(CoApp_components STATIC)

target_sources(CoApp_components
    PRIVATE
        BarCard.h BarCard.cpp
        BaseWidget.h BaseWidget.cpp
        IPv4Edit.h IPv4Edit.cpp
        LogBox.h LogBox.cpp
        RecordingIndicator.h RecordingIndicator.cpp
        SvgIconHelper.h SvgIconHelper.cpp
)

target_include_directories(CoApp_components
    PRIVATE ${APP_SRC_DIR}
)

target_link_libraries(CoApp_components
    PUBLIC Qt6::Core Qt6::Widgets Qt6::Svg CoApp_services
)
