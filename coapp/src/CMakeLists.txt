set(APP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(CoApp
    PRIVATE ${APP_SRC_DIR}
)

add_subdirectory(services)
add_subdirectory(components)
add_subdirectory(views)