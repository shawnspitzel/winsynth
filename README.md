# Winsynth

**Overview:** A low-level audio synthesizer written from scratch in modern C++20, using only Windows API directly for maximum control and minimum overhead. 

The motivation for this project was to learn more about and showcase how real-time audio synthesis works at the system level. An interface is also available, written with ImGui + Direct3D.

**Design:** Built around a custom template class (noiseMaker.h) that manages audio buffers and threading without external dependencies. The architecture prioritizes low latency through zero-copy buffer management and minimal lock contention. 

## Key Mappings
```
Q  W  E  R  T  Y  U  I  O  P
C5 D5 E5 F5 G5 A5 B5 C6 D6 E6

Z  X  C  V  B  N  M
C4 D4 E4 F4 G4 A4 B4
```
## Quick Start

```bash
git clone https://github.com/shawnspitzel/winsynth.git
cd winsynth
git submodule add https://github.com/ocornut/imgui.git external/imgui

mkdir build && cd build
cmake -S . -B build
cmake --build build --config Release

.\bin\Release\winsynth.exe
```

**Requirements:** Windows 10+, DirectX SDK


## Building from Source

### Requirements

1. **DirectX SDK** - [Download here](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
2. **CMake** 3.15+ - `winget install Kitware.CMake`
3. **ImGui** (handled by git submodule)

### Build Commands

**Windows (Quick)**
```bash
git submodule add https://github.com/ocornut/imgui.git external/imgui
.\scripts\build.bat
```

**Windows (Manual)**
```bash
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```

**Output:** `build/bin/Release/winsynth.exe`

For detailed build instructions, see [docs/BUILD.md](docs/BUILD.md)


## Some things I'd like to add:

- Remove ImGui dependency, using only DirectX
- Attempt full lock-free threading
- ADSR envelope generator
- Improved maximum polyphony (more sophisticated algorithm)
- Additional waveforms (saw, triangle, noise)

