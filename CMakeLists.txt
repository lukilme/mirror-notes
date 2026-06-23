cmake_minimum_required(VERSION 3.16)
project(GuitarVisualizer CXX)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FREETYPE REQUIRED freetype2)
pkg_check_modules(FONTCONFIG REQUIRED fontconfig)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
find_package(OpenSSL REQUIRED)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── openFrameworks root (set via -DOF_ROOT=... or env variable) ──────────────
# In fact, the real hell
if(NOT DEFINED OF_ROOT)
    if(DEFINED ENV{OF_ROOT})
        set(OF_ROOT $ENV{OF_ROOT})
    else()
        # Default install path on Linux
        set(OF_ROOT "/opt/openFrameworks")
    endif()
endif()

message(STATUS "OF_ROOT = ${OF_ROOT}")

# ─── openFrameworks include dirs ──────────────────────────────────────────────
include_directories(
    ${OF_ROOT}/libs/openFrameworks
    ${OF_ROOT}/libs/openFrameworks/3d
    ${OF_ROOT}/libs/openFrameworks/app
    ${OF_ROOT}/libs/openFrameworks/communication
    ${OF_ROOT}/libs/openFrameworks/events
    ${OF_ROOT}/libs/openFrameworks/gl
    ${OF_ROOT}/libs/openFrameworks/graphics
    ${OF_ROOT}/libs/openFrameworks/math
    ${OF_ROOT}/libs/openFrameworks/sound
    ${OF_ROOT}/libs/openFrameworks/types
    ${OF_ROOT}/libs/openFrameworks/utils
    ${OF_ROOT}/libs/openFrameworks/video
    ${OF_ROOT}/libs/glfw/include
    ${OF_ROOT}/libs/glm/include
    ${OF_ROOT}/libs/FreeImage/include
    ${OF_ROOT}/libs/rtAudio/include
    ${OF_ROOT}/libs/utf8/include
    ${OF_ROOT}/libs/tess2/include
    ${OF_ROOT}/libs/cairo/include/cairo
    ${OF_ROOT}/libs/openssl/include
    ${OF_ROOT}/libs/pugixml/include
)

# ─── Source files ─────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/ofApp.cpp
    src/AudioAnalyzer.cpp
    src/VisualMapper.cpp
    src/ParticleSystem.cpp
)

add_executable(GuitarVisualizer ${SOURCES})

# ─── openFrameworks static library ────────────────────────────────────────────
target_link_libraries(GuitarVisualizer
    PRIVATE
    ${OF_ROOT}/libs/openFrameworksCompiled/lib/linux64/libopenFrameworks.a
    ${OF_ROOT}/libs/tess2/lib/linux64/libtess2.a

    freeimage
    kissfft-float

    ${FREETYPE_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto

    rtaudio
    openal
    uriparser
    sndfile
    mpg123
    curl

    gstreamer-1.0
    gstbase-1.0
    gstapp-1.0
    gstvideo-1.0
    glib-2.0
    gobject-2.0
    gio-2.0
    gmodule-2.0

    GL
    GLU
    GLEW
    glfw
    Xrandr
    Xi
    X11
    Xxf86vm
    Xcursor
    Xinerama

    asound
    udev
    pthread
    dl
)

# ─── Compiler flags ───────────────────────────────────────────────────────────
target_compile_options(GuitarVisualizer PRIVATE
    -O2 -Wall -Wextra -Wno-unused-parameter
    -DGLM_FORCE_RADIANS
)
