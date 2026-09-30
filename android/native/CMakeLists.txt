cmake_minimum_required(VERSION 3.18)
project(muramasa_android CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -fvisibility=default")

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(XLIBS_ROOT "${CMAKE_CURRENT_LIST_DIR}/../xlibs" CACHE PATH "Android dependency prefix")
set(XLIBS "${XLIBS_ROOT}/${ANDROID_ABI}")
set(NPA "${REPO_ROOT}/libnpa")
set(NPE "${REPO_ROOT}/libnpengine")
set(MURA "${REPO_ROOT}/muramasa")

add_definitions(-DNP_GLES2 -D__ANDROID__)
add_definitions(-DFLEX_VERSION_MAJOR=2 -DFLEX_VERSION_MINOR=6 -DFLEX_VERSION_PATCH=4)

include_directories(
    ${NPA}/include
    ${NPE}/include
    ${NPE}/vendor
    ${MURA}/src
    ${XLIBS}/include
    ${XLIBS}/include/SDL2
    ${XLIBS}/include/libpng16
)

file(GLOB NPA_SRC ${NPA}/src/*.cpp)
file(GLOB NPE_SRC ${NPE}/src/*.cpp)
file(GLOB MURA_SRC ${MURA}/src/*.cpp)

add_library(main SHARED ${NPA_SRC} ${NPE_SRC} ${MURA_SRC})

add_library(SDL2 SHARED IMPORTED)
set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${XLIBS}/lib/libSDL2.so)

target_link_libraries(main
    SDL2
    ${XLIBS}/lib/libpng.a
    ${XLIBS}/lib/libjpeg.a
    z
    GLESv2
    log
    android
)
