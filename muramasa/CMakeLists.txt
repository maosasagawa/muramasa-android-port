cmake_minimum_required(VERSION 3.10)
project(muramasa CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")

find_package(NPA REQUIRED)
find_package(NPENGINE REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(Threads REQUIRED)
find_package(GLEW REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(GStreamer REQUIRED)
include(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(libpng REQUIRED libpng)
PKG_SEARCH_MODULE(PANGOCAIRO REQUIRED pangocairo)

include_directories(
    ${NPA_INCLUDE_DIR}
    ${NPENGINE_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/../libnpa/include
    ${CMAKE_SOURCE_DIR}/../libnpengine/include
)

link_directories(
    ${CMAKE_SOURCE_DIR}/../libnpa/build
    ${CMAKE_SOURCE_DIR}/../libnpengine/build
)

add_executable(muramasa
    src/Main.cpp
    src/MuramasaResourceMgr.cpp
    src/MuramasaInterpreter.cpp
    src/MuramasaWindow.cpp
)

target_link_libraries(muramasa
    npengine
    npa
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${SDL2_LIBRARIES}
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_APP_LIBRARIES}
    ${GSTREAMER_VIDEO_LIBRARIES}
    ${PANGOCAIRO_LIBRARIES}
    ${JPEG_LIBRARY}
    GLEW
    ${PNG_LIBRARIES}
    GL
)
