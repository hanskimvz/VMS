cmake_minimum_required(VERSION 3.21)
project(VMS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS 
    Widgets 
    OpenGLWidgets 
    Network 
    Sql
    Concurrent
)

# Find FFmpeg
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavcodec
    libavformat
    libavutil
    libswscale
    libswresample
)

# Source files
set(CORE_SOURCES
    src/core/camera.cpp
    src/core/camera_manager.cpp
    src/core/stream_receiver.cpp
    src/core/video_decoder.cpp
    src/core/recorder.cpp
    src/core/onvif_client.cpp
    src/core/playback_controller.cpp
    src/core/device_discovery.cpp
)

set(CORE_HEADERS
    src/core/camera.h
    src/core/camera_manager.h
    src/core/stream_receiver.h
    src/core/video_decoder.h
    src/core/recorder.h
    src/core/onvif_client.h
    src/core/playback_controller.h
    src/core/device_discovery.h
)

set(UI_SOURCES
    src/ui/main_window.cpp
    src/ui/video_widget.cpp
    src/ui/video_grid.cpp
    src/ui/camera_tree.cpp
    src/ui/playback_view.cpp
    src/ui/timeline_widget.cpp
    src/ui/ptz_control.cpp
    src/ui/add_camera_dialog.cpp
    src/ui/device_manage_widget.cpp
    src/ui/network_settings_dialog.cpp
)

set(UI_HEADERS
    src/ui/main_window.h
    src/ui/video_widget.h
    src/ui/video_grid.h
    src/ui/camera_tree.h
    src/ui/playback_view.h
    src/ui/timeline_widget.h
    src/ui/ptz_control.h
    src/ui/add_camera_dialog.h
    src/ui/device_manage_widget.h
    src/ui/network_settings_dialog.h
)

set(UTILS_SOURCES
    src/utils/database.cpp
)

set(UTILS_HEADERS
    src/utils/database.h
)

# Resources
set(RESOURCES
    resources/resources.qrc
)

# Main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${UI_SOURCES}
    ${UI_HEADERS}
    ${UTILS_SOURCES}
    ${UTILS_HEADERS}
    ${RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/ui
    ${CMAKE_SOURCE_DIR}/src/utils
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::Network
    Qt6::Sql
    Qt6::Concurrent
    PkgConfig::FFMPEG
)

# Windows specific
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
