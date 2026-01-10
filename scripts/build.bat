@echo off
REM Quick build script for Windows
REM Usage: build.bat [debug|release] [clean]

setlocal enabledelayedexpansion

REM Parse arguments
set BUILD_TYPE=Release
set DO_CLEAN=0

if "%1"=="debug" set BUILD_TYPE=Debug
if "%1"=="Debug" set BUILD_TYPE=Debug
if "%1"=="release" set BUILD_TYPE=Release
if "%1"=="Release" set BUILD_TYPE=Release
if "%2"=="clean" set DO_CLEAN=1
if "%1"=="clean" set DO_CLEAN=1

echo ========================================
echo Keyboard Synthesizer Build Script
echo ========================================
echo Build Type: %BUILD_TYPE%
echo.

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake not found!
    echo Please install CMake from https://cmake.org/download/
    echo Or via winget: winget install Kitware.CMake
    exit /b 1
)

REM Check for ImGui
if not exist "ImGui\ImGui\imgui.h" (
    echo ERROR: ImGui not found!
    echo.
    echo Please install ImGui first:
    echo   1. As git submodule:
    echo      git submodule add https://github.com/ocornut/imgui.git ImGui/ImGui
    echo.
    echo   2. Or download from: https://github.com/ocornut/imgui
    echo      Extract to: ImGui\ImGui\
    echo.
    exit /b 1
)

REM Check for DirectX SDK
if not defined DXSDK_DIR (
    echo WARNING: DXSDK_DIR environment variable not set
    echo DirectX SDK may not be found
    echo Download from: https://www.microsoft.com/en-us/download/details.aspx?id=6812
    echo.
)

REM Clean if requested
if %DO_CLEAN%==1 (
    echo Cleaning build directory...
    if exist build rmdir /s /q build
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure
echo.
echo Configuring with CMake...
cmake -G "Visual Studio 17 2022" -A x64 ..
if %ERRORLEVEL% neq 0 (
    echo.
    echo ERROR: CMake configuration failed!
    echo.
    echo Try older Visual Studio version:
    echo   cmake -G "Visual Studio 16 2019" -A x64 ..
    echo.
    echo Or use Ninja for faster builds:
    echo   cmake -G Ninja -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..
    cd ..
    exit /b 1
)

REM Build
echo.
echo Building %BUILD_TYPE% configuration...
cmake --build . --config %BUILD_TYPE% --parallel
if %ERRORLEVEL% neq 0 (
    echo.
    echo ERROR: Build failed!
    cd ..
    exit /b 1
)

cd ..

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo Executable: build\bin\%BUILD_TYPE%\KeyboardSynthesizer.exe
echo.
echo To run:
echo   cd build\bin\%BUILD_TYPE%
echo   KeyboardSynthesizer.exe
echo.
echo Or double-click: build\bin\%BUILD_TYPE%\KeyboardSynthesizer.exe
echo ========================================

exit /b 0
