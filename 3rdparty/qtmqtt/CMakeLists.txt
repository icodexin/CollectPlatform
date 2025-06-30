cmake_minimum_required(VERSION 3.20)

project(qtmqtt)

find_package(Qt6)

set(QT_INSTALL_DIR "${Qt6_DIR}/../../../")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(SYSTEM_FLAG win64)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(SYSTEM_FLAG mac)
endif ()

if (NOT EXISTS "${QT_INSTALL_DIR}/lib/cmake/Qt6Mqtt/Qt6MqttConfig.cmake")
    message(STATUS "Copying QtMqtt files to ${QT_INSTALL_DIR}")
    file(GLOB_RECURSE FILES_TO_COPY "${CMAKE_CURRENT_SOURCE_DIR}/${SYSTEM_FLAG}/*")
    foreach (FILE ${FILES_TO_COPY})
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/${SYSTEM_FLAG}/" "" RELATIVE_PATH ${FILE})
        configure_file(${FILE} "${QT_INSTALL_DIR}/${RELATIVE_PATH}" COPYONLY)
    endforeach ()
endif ()