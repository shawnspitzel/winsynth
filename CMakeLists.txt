cmake_minimum_required(VERSION 3.15)
project(winsynth VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(SYNTH_USE_SYSTEM_IMGUI "Use system-installed ImGui instead of bundled" OFF)

# platform detection
if(WIN32)
    set(SYNTH_PLATFORM_WINDOWS TRUE)
    message(STATUS "Building for Windows")
elseif(APPLE)
    set(SYNTH_PLATFORM_MACOS TRUE)
    message(STATUS "Building for macOS")
elseif(UNIX)
    set(SYNTH_PLATFORM_LINUX TRUE)
    message(STATUS "Building for Linux")
endif()

set(SYNTH_SOURCES
    src/main.cpp
    src/App.cpp
    src/AudioManager.cpp
    src/D3DManager.cpp
    src/GUIManager.cpp
)

set(SYNTH_HEADERS
    include/App.h
    include/AudioManager.h
    include/D3DManager.h
    include/GUIManager.h
    include/noiseMaker.h
)

if(SYNTH_PLATFORM_WINDOWS)
    # find DirectX 9
    if(NOT DEFINED ENV{DXSDK_DIR})
        message(WARNING "DXSDK_DIR environment variable not set. DirectX SDK may not be found.")
        message(WARNING "Download from: https://www.microsoft.com/en-us/download/details.aspx?id=6812")
    endif()

    set(DIRECTX_INCLUDE_DIR "$ENV{DXSDK_DIR}/Include")
    set(DIRECTX_LIB_DIR "$ENV{DXSDK_DIR}/Lib/x64")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(DIRECTX_LIB_DIR "$ENV{DXSDK_DIR}/Lib/x86")
    endif()

    # platform-specific libraries
    set(PLATFORM_LIBS
        winmm      # Windows Multimedia API
        d3d9       # Direct3D 9
    )

    # add DirectX include directory
    if(EXISTS "${DIRECTX_INCLUDE_DIR}")
        include_directories(${DIRECTX_INCLUDE_DIR})
        link_directories(${DIRECTX_LIB_DIR})
    else()
        message(WARNING "DirectX SDK include directory not found at: ${DIRECTX_INCLUDE_DIR}")
    endif()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

elseif(SYNTH_PLATFORM_MACOS)
    message(WARNING "macOS build requires porting from Direct3D to Metal/OpenGL and Windows MM to CoreAudio")
    message(FATAL_ERROR "macOS is not currently supported. This is a Windows-only application.")

    # TODO: future macOS support:
    # - Replace D3D9 with Metal or OpenGL
    # - Replace WMM API with CoreAudio
    # - Replace Win32 windowing with Cocoa or SDL

elseif(SYNTH_PLATFORM_LINUX)
    # Linux-specific configuration
    message(WARNING "Linux build requires porting from Direct3D to OpenGL and Windows MM to ALSA/PulseAudio")
    message(FATAL_ERROR "Linux is not currently supported. This is a Windows-only application.")

    # TODO: Future Linux support:
    # - Replace D3D9 with OpenGL
    # - Replace WMM API with ALSA or PulseAudio
    # - Replace Win32 windowing with X11 or Wayland
endif()

# ImGui
if(SYNTH_USE_SYSTEM_IMGUI)
    find_package(imgui CONFIG REQUIRED)
    set(IMGUI_LIBRARIES imgui::imgui)
else()
    set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")

    if(NOT EXISTS "${IMGUI_DIR}/imgui.h")
        message(FATAL_ERROR
            "ImGui not found at: ${IMGUI_DIR}\n"
            "Please install ImGui:\n"
            "  1. Download from: https://github.com/ocornut/imgui\n"
            "  2. Extract to: ${IMGUI_DIR}\n"
            "  OR use git submodule:\n"
            "     git submodule add https://github.com/ocornut/imgui.git external/imgui\n"
            "  OR use system ImGui:\n"
            "     cmake -DSYNTH_USE_SYSTEM_IMGUI=ON .."
        )
    endif()

    set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
    )

    if(SYNTH_PLATFORM_WINDOWS)
        list(APPEND IMGUI_SOURCES
            ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
            ${IMGUI_DIR}/backends/imgui_impl_dx9.cpp
        )
    endif()

    add_library(imgui STATIC ${IMGUI_SOURCES})

    target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
    )

    if(SYNTH_PLATFORM_WINDOWS)
        target_compile_definitions(imgui PUBLIC
            IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
        )
    endif()

    set(IMGUI_LIBRARIES imgui)
endif()

add_executable(${PROJECT_NAME} WIN32 ${SYNTH_SOURCES} ${SYNTH_HEADERS})


target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${IMGUI_LIBRARIES}
    ${PLATFORM_LIBS}
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4                 
        /permissive-        
        /Zc:__cplusplus   
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    source_group("Source Files" FILES ${SYNTH_SOURCES})
    source_group("Header Files" FILES ${SYNTH_HEADERS})

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(FILES
    README.md
    LICENSE
    CONTRIBUTING.md
    BUGS_FIXED.md
    ARCHITECTURE_NOTES.md
    LIBRARY_REFACTORING_ROADMAP.md
    LIBRARY_USAGE_EXAMPLES.md
    DESTINATION share/doc/${PROJECT_NAME}
)
