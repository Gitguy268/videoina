cmake_minimum_required(VERSION 3.21)
project(Videoina VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 OpenGL ShaderTools Test)
qt_standard_project_setup(REQUIRES 6.5)

# libmpv: pkg-config on Linux/macOS; on Windows pass -DMPV_DIR=<unpacked mpv-dev archive>.
if(WIN32)
    set(MPV_DIR "" CACHE PATH "Folder with include/mpv, libmpv-2.dll and its import lib")
    add_library(mpv SHARED IMPORTED)
    # MSVC needs mpv.lib, made from the DLL's exports (see .github/workflows/build.yml).
    if(MSVC)
        set(MPV_IMPLIB "${MPV_DIR}/mpv.lib")
    else()
        set(MPV_IMPLIB "${MPV_DIR}/libmpv.dll.a")
    endif()
    set_target_properties(mpv PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${MPV_DIR}/include"
        IMPORTED_IMPLIB "${MPV_IMPLIB}"
        IMPORTED_LOCATION "${MPV_DIR}/libmpv-2.dll")
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)
    add_library(mpv ALIAS PkgConfig::MPV)
endif()

qt_add_executable(videoina WIN32 MACOSX_BUNDLE src/main.cpp src/mpvitem.cpp src/mpvitem.h src/keys.cpp src/keys.h)

qt_add_qml_module(videoina
    URI Videoina
    QML_FILES qml/Main.qml qml/Glass.qml qml/ControlBar.qml qml/Playlist.qml qml/Icon.qml
    RESOURCE_PREFIX /)

# IINA key bindings, read by MpvItem from :/input.conf.
qt_add_resources(videoina "config" PREFIX "/" BASE config FILES config/input.conf)
qt_add_shaders(videoina "shaders" PREFIX "/" FILES shaders/glass.frag)

target_include_directories(videoina PRIVATE src)
target_link_libraries(videoina PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::OpenGL mpv)

enable_testing()
qt_add_executable(keys_test tests/keys_test.cpp src/keys.cpp)
target_include_directories(keys_test PRIVATE src)
target_link_libraries(keys_test PRIVATE Qt6::Test)
add_test(NAME keys COMMAND keys_test)

install(TARGETS videoina BUNDLE DESTINATION . RUNTIME DESTINATION bin)
