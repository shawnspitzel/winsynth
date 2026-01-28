# Build Instructions

This document provides detailed instructions for building using CMake.

## Prerequisites


- **CMake** 3.15 or later
  - Download: https://cmake.org/download/
  - Or via package manager:
    - Windows: `winget install Kitware.CMake`
    - macOS: `brew install cmake`
    - Linux: `sudo apt install cmake`
- **DirectX SDK** 
  - Download: https://www.microsoft.com/en-us/download/details.aspx?id=6812
  - Install location: `C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\`
  - Set `DXSDK_DIR` environment variable (installer usually does this)

- **ImGui** (Dear ImGui)
  - See [Installing ImGui](#installing-imgui) below

## Installing ImGui

ImGui is required but not included in this repository. Choose one of these methods:

### Method 1: Git Submodule
```bash
cd sound-synthesizer
git submodule add https://github.com/ocornut/imgui.git ImGui/ImGui
git submodule update --init --recursive
```

### Method 2: Manual Download
1. Download ImGui from: https://github.com/ocornut/imgui
2. Extract to: `sound-synthesizer/ImGui/ImGui`
3. Verify structure:
   ```
   ImGui/
   └── ImGui/
       ├── imgui.h
       ├── imgui.cpp
       ├── backends/
       │   ├── imgui_impl_win32.h
       │   ├── imgui_impl_win32.cpp
       │   ├── imgui_impl_dx9.h
       │   └── imgui_impl_dx9.cpp
       └── ... (other ImGui files)
   ```

## Building Instructions
```bash
# Navigate to project directory
cd sound-synthesizer
mkdir build
cd build
cmake ..
cmake --build . --config 
.\bin\Debug\winsynth.exe
# or
.\bin\Release\winsynth.exe
```
