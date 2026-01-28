# Winsynth

**Overview:** A low-level audio synthesizer written from scratch in modern C++20, using only Win32 calls for maximum control and minimum overhead. 

The motivation for this project was to learn more about and showcase how real-time audio synthesis works at the system level. An interface is also available, written with ImGui + Direct3D.

**Design:** Built around a header-only audio engine that manages audio buffers and threading without external dependencies. The architecture prioritizes low latency through zero-copy buffer management and minimal lock contention. 

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

mkdir build
cmake -S . -B build
cmake --build build --config Release

cd build
.\bin\Release\winsynth.exe
```

**Requirements:** Windows 10+, DirectX SDK


## Building from Source

### Requirements

1. **DirectX SDK** - [Download here](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
2. **CMake** 
3. **ImGui**

For detailed build instructions, see [docs/BUILD.md](docs/BUILD.md)


## Some things I'd like to add:

- Remove ImGui dependency
- Lock-free threading on hot-paths
- ADSR envelope generator
- Improved polyphony
- Additional waveforms (saw, triangle, noise)

