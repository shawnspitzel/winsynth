# Library Refactoring Roadmap
## Transforming into a Reusable Audio Synthesis Framework

---

## 🎯 Vision: From Application to Library

### Current State: Standalone Application
```
User App (Music Synthesizer.exe)
└── Tightly coupled to ImGui, Direct3D, Win32
```

### Target State: Reusable Framework
```
Developer's App
└── YourAudioLib (headers + .lib/.dll)
    ├── Core synthesis engine
    ├── Optional GUI components
    └── Platform abstraction
```

---

## 📊 Phase-by-Phase Transformation

### Phase 1: Separate Core from Application (1-2 weeks)
**Goal**: Extract audio synthesis logic from UI/rendering

#### Step 1.1: Create Library Structure
```
SynthFramework/                    # New library project
├── include/                       # Public API headers
│   ├── SynthFramework.h          # Main header
│   ├── AudioEngine.h             # Core audio engine
│   ├── Synthesizer.h             # Synthesizer interface
│   ├── Oscillator.h              # Waveform generators
│   └── NoteManager.h             # Note/frequency management
├── src/                          # Implementation files
│   ├── AudioEngine.cpp
│   ├── Synthesizer.cpp
│   ├── Oscillator.cpp
│   └── NoteManager.cpp
├── platform/                     # Platform-specific code
│   ├── IAudioBackend.h          # Abstract interface
│   ├── WindowsMMBackend.h       # Windows implementation
│   └── WindowsMMBackend.cpp
└── examples/                     # Example applications
    ├── SimpleKeyboard/           # Your current app (refactored)
    └── MIDIPlayer/              # Future example

MusicSynthesizerApp/              # Example using the library
├── App.cpp                       # UI-specific code
├── GUIManager.cpp
└── main.cpp
```

#### Step 1.2: Extract Core Classes

**Current monolithic design:**
```cpp
// Everything in one app
App → AudioManager → NoiseMaker<int>
     ↓
  D3DManager, GUIManager
```

**New library design:**
```cpp
// Library layer (platform-agnostic)
Synthesizer → AudioEngine → IAudioBackend
    ↓                            ↑
NoteManager              WindowsMMBackend

// Application layer (user's code)
App → Synthesizer (library)
    ↓
  D3DManager, GUIManager (optional)
```

#### Step 1.3: Define Public API

Create `include/SynthFramework.h` (main header):
```cpp
#pragma once

// Main library header - includes everything users need
#include "AudioEngine.h"
#include "Synthesizer.h"
#include "Oscillator.h"
#include "NoteManager.h"

namespace SynthFramework
{
    // Version information
    constexpr int VERSION_MAJOR = 1;
    constexpr int VERSION_MINOR = 0;
    constexpr int VERSION_PATCH = 0;

    // Library initialization
    bool Initialize();
    void Shutdown();

    // Get version string
    const char* GetVersion();
}
```

Create `include/Synthesizer.h` (main interface):
```cpp
#pragma once
#include <memory>
#include <string>
#include <functional>

namespace SynthFramework
{
    enum class WaveType
    {
        Sine,
        Square,
        Sawtooth,
        Triangle,
        Noise
    };

    // Configuration for audio engine
    struct AudioConfig
    {
        unsigned int sampleRate = 44100;
        unsigned int channels = 1;
        unsigned int bufferSize = 512;
        unsigned int numBuffers = 8;
    };

    // Main synthesizer class - this is what users interact with
    class Synthesizer
    {
    public:
        Synthesizer();
        ~Synthesizer();

        // Initialization
        bool Initialize(const AudioConfig& config = AudioConfig());
        void Shutdown();

        // Playback control
        void PlayNote(double frequency);
        void StopNote(double frequency);
        void StopAllNotes();

        // Waveform control
        void SetWaveType(WaveType type);
        WaveType GetWaveType() const;

        // Advanced: Custom waveform function
        using WaveformFunction = std::function<double(double frequency, double time)>;
        void SetCustomWaveform(WaveformFunction func);

        // Volume control
        void SetVolume(double volume); // 0.0 to 1.0
        double GetVolume() const;

        // Audio device enumeration
        static std::vector<std::string> GetAvailableDevices();
        bool SelectDevice(const std::string& deviceName);

        // Status
        bool IsInitialized() const;
        int GetActiveNoteCount() const;

    private:
        class Impl; // Pimpl idiom for ABI stability
        std::unique_ptr<Impl> m_impl;
    };
}
```

Create `include/NoteManager.h`:
```cpp
#pragma once
#include <unordered_map>
#include <mutex>

namespace SynthFramework
{
    // Musical note frequencies
    namespace Notes
    {
        // Standard pitch (A4 = 440 Hz)
        constexpr double A4 = 440.0;

        // Helper to convert MIDI note to frequency
        double MIDINoteToFrequency(int midiNote);

        // Helper to convert note name to frequency
        double NoteNameToFrequency(const char* noteName); // e.g., "C4", "A#5"
    }

    // Manages active notes and polyphony
    class NoteManager
    {
    public:
        NoteManager(int maxPolyphony = 16);

        void PlayNote(double frequency, double velocity = 1.0);
        void StopNote(double frequency);
        void StopAllNotes();

        // Get all active notes for rendering
        std::vector<std::pair<double, double>> GetActiveNotes() const;

        int GetActiveCount() const;
        int GetMaxPolyphony() const;
        void SetMaxPolyphony(int maxNotes);

    private:
        mutable std::mutex m_mutex;
        std::unordered_map<double, double> m_activeNotes; // frequency -> velocity
        int m_maxPolyphony;
    };
}
```

Create `include/Oscillator.h`:
```cpp
#pragma once

namespace SynthFramework
{
    // Pure waveform generators (stateless, thread-safe)
    class Oscillator
    {
    public:
        // Basic waveforms
        static double Sine(double frequency, double time);
        static double Square(double frequency, double time);
        static double Sawtooth(double frequency, double time);
        static double Triangle(double frequency, double time);
        static double Noise(double frequency, double time);

        // Advanced waveforms
        static double Pulse(double frequency, double time, double dutyCycle = 0.5);
        static double SuperSaw(double frequency, double time, int voices = 7, double detune = 0.1);
    };
}
```

---

### Phase 2: Platform Abstraction Layer (1 week)
**Goal**: Decouple from Windows-specific APIs

#### Step 2.1: Define Platform Interface

Create `platform/IAudioBackend.h`:
```cpp
#pragma once
#include <vector>
#include <string>
#include <functional>

namespace SynthFramework
{
    // Abstract audio backend interface
    class IAudioBackend
    {
    public:
        virtual ~IAudioBackend() = default;

        // Device management
        virtual std::vector<std::string> EnumerateDevices() = 0;
        virtual bool SelectDevice(const std::string& deviceName) = 0;

        // Initialization
        virtual bool Initialize(unsigned int sampleRate,
                              unsigned int channels,
                              unsigned int bufferSize,
                              unsigned int numBuffers) = 0;
        virtual void Shutdown() = 0;

        // Audio callback
        using AudioCallback = std::function<double(double time)>;
        virtual void SetCallback(AudioCallback callback) = 0;

        // Status
        virtual bool IsInitialized() const = 0;
        virtual unsigned int GetSampleRate() const = 0;
    };

    // Factory function to create platform-specific backend
    std::unique_ptr<IAudioBackend> CreateAudioBackend();
}
```

#### Step 2.2: Implement Windows Backend

Create `platform/WindowsMMBackend.h`:
```cpp
#pragma once
#include "IAudioBackend.h"
#include <Windows.h>
#include <memory>

namespace SynthFramework
{
    class WindowsMMBackend : public IAudioBackend
    {
    public:
        WindowsMMBackend();
        ~WindowsMMBackend() override;

        // IAudioBackend implementation
        std::vector<std::string> EnumerateDevices() override;
        bool SelectDevice(const std::string& deviceName) override;
        bool Initialize(unsigned int sampleRate,
                       unsigned int channels,
                       unsigned int bufferSize,
                       unsigned int numBuffers) override;
        void Shutdown() override;
        void SetCallback(AudioCallback callback) override;
        bool IsInitialized() const override;
        unsigned int GetSampleRate() const override;

    private:
        // Implementation details hidden from public API
        class Impl;
        std::unique_ptr<Impl> m_impl;
    };
}
```

#### Step 2.3: Add Platform Detection

Create `platform/PlatformDetection.h`:
```cpp
#pragma once

// Platform detection macros
#if defined(_WIN32) || defined(_WIN64)
    #define SYNTH_PLATFORM_WINDOWS
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC
        #define SYNTH_PLATFORM_MACOS
    #endif
#elif defined(__linux__)
    #define SYNTH_PLATFORM_LINUX
#endif

// Factory implementation
namespace SynthFramework
{
    inline std::unique_ptr<IAudioBackend> CreateAudioBackend()
    {
        #ifdef SYNTH_PLATFORM_WINDOWS
            return std::make_unique<WindowsMMBackend>();
        #elif defined(SYNTH_PLATFORM_MACOS)
            return std::make_unique<CoreAudioBackend>();
        #elif defined(SYNTH_PLATFORM_LINUX)
            return std::make_unique<ALSABackend>();
        #else
            #error "Unsupported platform"
        #endif
    }
}
```

---

### Phase 3: Build System & Distribution (3-4 days)
**Goal**: Make it easy for developers to integrate

#### Step 3.1: CMake Build System

Create `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.15)
project(SynthFramework VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library options
option(SYNTH_BUILD_SHARED "Build shared library" ON)
option(SYNTH_BUILD_EXAMPLES "Build example applications" ON)
option(SYNTH_BUILD_TESTS "Build unit tests" OFF)

# Library source files
set(SYNTH_SOURCES
    src/AudioEngine.cpp
    src/Synthesizer.cpp
    src/Oscillator.cpp
    src/NoteManager.cpp
)

# Platform-specific sources
if(WIN32)
    list(APPEND SYNTH_SOURCES platform/WindowsMMBackend.cpp)
    set(PLATFORM_LIBS winmm)
elseif(APPLE)
    list(APPEND SYNTH_SOURCES platform/CoreAudioBackend.cpp)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    set(PLATFORM_LIBS ${COREAUDIO_LIBRARY} ${COREFOUNDATION_LIBRARY})
elseif(UNIX)
    list(APPEND SYNTH_SOURCES platform/ALSABackend.cpp)
    set(PLATFORM_LIBS asound)
endif()

# Create library
if(SYNTH_BUILD_SHARED)
    add_library(SynthFramework SHARED ${SYNTH_SOURCES})
else()
    add_library(SynthFramework STATIC ${SYNTH_SOURCES})
endif()

# Include directories
target_include_directories(SynthFramework
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link platform libraries
target_link_libraries(SynthFramework PRIVATE ${PLATFORM_LIBS})

# Installation
install(TARGETS SynthFramework
    EXPORT SynthFrameworkTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Export configuration
install(EXPORT SynthFrameworkTargets
    FILE SynthFrameworkTargets.cmake
    NAMESPACE SynthFramework::
    DESTINATION lib/cmake/SynthFramework
)

# Build examples
if(SYNTH_BUILD_EXAMPLES)
    add_subdirectory(examples/SimpleKeyboard)
endif()
```

#### Step 3.2: Package Configuration

Create `SynthFrameworkConfig.cmake.in`:
```cmake
@PACKAGE_INIT@

include("${CMAKE_CURRENT_LIST_DIR}/SynthFrameworkTargets.cmake")

check_required_components(SynthFramework)
```

#### Step 3.3: Usage Documentation

Create `USAGE.md`:
```markdown
# Using SynthFramework in Your Project

## CMake Integration

### Option 1: Add as Subdirectory
```cmake
add_subdirectory(path/to/SynthFramework)
target_link_libraries(YourApp PRIVATE SynthFramework)
```

### Option 2: Install and Find
```bash
cd SynthFramework
mkdir build && cd build
cmake ..
cmake --build .
sudo cmake --install .
```

Then in your project:
```cmake
find_package(SynthFramework REQUIRED)
target_link_libraries(YourApp PRIVATE SynthFramework::SynthFramework)
```

## Visual Studio Integration

### Option 1: NuGet Package (future)
```
Install-Package SynthFramework
```

### Option 2: Manual Integration
1. Add `include/` to Additional Include Directories
2. Link against `SynthFramework.lib`
3. Copy `SynthFramework.dll` to output directory

## Basic Usage Example

```cpp
#include <SynthFramework.h>
#include <iostream>

int main()
{
    using namespace SynthFramework;

    // Initialize library
    if (!SynthFramework::Initialize())
    {
        std::cerr << "Failed to initialize library\n";
        return 1;
    }

    // Create synthesizer
    Synthesizer synth;

    // Configure audio settings
    AudioConfig config;
    config.sampleRate = 44100;
    config.channels = 1;

    if (!synth.Initialize(config))
    {
        std::cerr << "Failed to initialize synthesizer\n";
        return 1;
    }

    // Set waveform
    synth.SetWaveType(WaveType::Sine);

    // Play a chord (C major)
    synth.PlayNote(Notes::NoteNameToFrequency("C4"));
    synth.PlayNote(Notes::NoteNameToFrequency("E4"));
    synth.PlayNote(Notes::NoteNameToFrequency("G4"));

    // Keep playing for 2 seconds
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Stop all notes
    synth.StopAllNotes();

    // Cleanup
    synth.Shutdown();
    SynthFramework::Shutdown();

    return 0;
}
```
```

---

### Phase 4: Advanced Features (Ongoing)
**Goal**: Make the library more powerful and flexible

#### Feature Set Expansion

1. **ADSR Envelope**
```cpp
namespace SynthFramework
{
    struct ADSREnvelope
    {
        double attack = 0.01;    // seconds
        double decay = 0.1;      // seconds
        double sustain = 0.7;    // level (0-1)
        double release = 0.3;    // seconds
    };

    class Synthesizer
    {
    public:
        void SetEnvelope(const ADSREnvelope& envelope);
        ADSREnvelope GetEnvelope() const;
    };
}
```

2. **Effects Chain**
```cpp
namespace SynthFramework
{
    class IEffect
    {
    public:
        virtual ~IEffect() = default;
        virtual double Process(double sample) = 0;
    };

    class ReverbEffect : public IEffect { /* ... */ };
    class DelayEffect : public IEffect { /* ... */ };
    class FilterEffect : public IEffect { /* ... */ };

    class Synthesizer
    {
    public:
        void AddEffect(std::unique_ptr<IEffect> effect);
        void RemoveEffect(size_t index);
        void ClearEffects();
    };
}
```

3. **MIDI Support**
```cpp
namespace SynthFramework
{
    class MIDIInput
    {
    public:
        using NoteOnCallback = std::function<void(int note, int velocity)>;
        using NoteOffCallback = std::function<void(int note)>;

        bool Initialize();
        void SetNoteOnCallback(NoteOnCallback callback);
        void SetNoteOffCallback(NoteOffCallback callback);

        std::vector<std::string> GetAvailableDevices();
        bool SelectDevice(const std::string& deviceName);
    };
}
```

4. **Preset System**
```cpp
namespace SynthFramework
{
    struct SynthPreset
    {
        std::string name;
        WaveType waveType;
        ADSREnvelope envelope;
        double volume;
        std::vector<EffectSettings> effects;
    };

    class Synthesizer
    {
    public:
        void LoadPreset(const SynthPreset& preset);
        SynthPreset SavePreset() const;

        bool LoadPresetFromFile(const std::string& filename);
        bool SavePresetToFile(const std::string& filename) const;
    };
}
```

---

### Phase 5: Quality & Distribution (Ongoing)

#### Step 5.1: Unit Testing
```cpp
// tests/OscillatorTests.cpp
#include <gtest/gtest.h>
#include "Oscillator.h"

TEST(OscillatorTest, SineWaveAmplitude)
{
    double sample = Oscillator::Sine(440.0, 0.0);
    EXPECT_NEAR(sample, 0.0, 0.0001);

    sample = Oscillator::Sine(440.0, 0.25 / 440.0);
    EXPECT_NEAR(sample, 1.0, 0.0001);
}
```

#### Step 5.2: Documentation Generation
```bash
# Doxygen configuration
doxygen Doxyfile

# Generates:
docs/html/index.html  # API documentation
```

#### Step 5.3: Continuous Integration
```yaml
# .github/workflows/build.yml
name: Build and Test

on: [push, pull_request]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [windows-latest, macos-latest, ubuntu-latest]

    steps:
    - uses: actions/checkout@v2
    - name: Configure CMake
      run: cmake -B build
    - name: Build
      run: cmake --build build
    - name: Test
      run: cd build && ctest
```

#### Step 5.4: Distribution Channels

**1. GitHub Releases**
- Binary distributions (Windows .dll/.lib, macOS .framework)
- Header-only option
- Example projects

**2. Package Managers**
- **vcpkg**: Windows/cross-platform
  ```bash
  vcpkg install synthframework
  ```
- **Homebrew**: macOS
  ```bash
  brew install synthframework
  ```
- **Conan**: Cross-platform
  ```bash
  conan install synthframework/1.0.0@
  ```

**3. NuGet** (Windows-specific)
```xml
<package>
  <metadata>
    <id>SynthFramework</id>
    <version>1.0.0</version>
    <description>Audio synthesis framework</description>
  </metadata>
</package>
```

---

## 📋 Migration Checklist

### Core Refactoring
- [ ] Extract `AudioManager` → `Synthesizer` class
- [ ] Extract `noiseMaker.h` → `AudioEngine` + `IAudioBackend`
- [ ] Create `Oscillator` static class from wave functions
- [ ] Create `NoteManager` from note tracking logic
- [ ] Move constants to `Notes` namespace

### Platform Abstraction
- [ ] Define `IAudioBackend` interface
- [ ] Implement `WindowsMMBackend`
- [ ] Add platform detection
- [ ] Abstract all Windows.h dependencies

### Build System
- [ ] Create CMakeLists.txt
- [ ] Add platform-specific build rules
- [ ] Create installation targets
- [ ] Generate config files

### Documentation
- [ ] API documentation (Doxygen)
- [ ] Usage examples
- [ ] Migration guide
- [ ] Architecture diagrams

### Quality Assurance
- [ ] Unit tests
- [ ] Integration tests
- [ ] Performance benchmarks
- [ ] CI/CD pipeline

### Distribution
- [ ] GitHub releases
- [ ] Package manager submissions
- [ ] Example projects
- [ ] Tutorial videos/blog posts

---

## 🎓 Example: Refactoring AudioManager to Synthesizer

### Before (Current Code)
```cpp
// AudioManager.h - Application-specific
class AudioManager
{
public:
    bool Initialize();
    void HandleKeyDown(WPARAM wParam);  // Windows-specific!
    void SetWaveType(WaveType type);

private:
    std::unique_ptr<NoiseMaker<int>> m_sound;  // Platform-specific!
    std::unordered_map<WPARAM, double> m_activeNotes;
    static AudioManager* s_instance;  // Global state!
};
```

### After (Library Code)
```cpp
// Synthesizer.h - Reusable library
namespace SynthFramework
{
    class Synthesizer
    {
    public:
        bool Initialize(const AudioConfig& config = AudioConfig());
        void PlayNote(double frequency);  // Platform-agnostic!
        void StopNote(double frequency);
        void SetWaveType(WaveType type);

    private:
        class Impl;  // Pimpl idiom hides implementation
        std::unique_ptr<Impl> m_impl;
    };
}

// Application code using the library
void OnKeyDown(int key)
{
    double freq = KeyToFrequency(key);
    m_synthesizer.PlayNote(freq);  // Clean, platform-agnostic!
}
```

---

## 🚀 Quick Start: Minimal Viable Library

If you want to start small, here's a 1-day minimal refactor:

### Create 3 Files

**1. SynthFramework.h** (Public API)
```cpp
#pragma once
#include <memory>
#include <functional>

namespace SynthFramework
{
    enum class WaveType { Sine, Square };

    class Synthesizer
    {
    public:
        Synthesizer();
        ~Synthesizer();

        bool Initialize();
        void PlayNote(double frequency);
        void StopNote(double frequency);
        void SetWaveType(WaveType type);

    private:
        class Impl;
        std::unique_ptr<Impl> m_impl;
    };
}
```

**2. Synthesizer.cpp** (Implementation)
```cpp
#include "SynthFramework.h"
#include "noiseMaker.h"  // Reuse existing code!
#include "AudioManager.h"

namespace SynthFramework
{
    // Pimpl implementation
    class Synthesizer::Impl
    {
    public:
        std::unique_ptr<AudioManager> audioManager;
    };

    Synthesizer::Synthesizer() : m_impl(std::make_unique<Impl>()) {}
    Synthesizer::~Synthesizer() = default;

    bool Synthesizer::Initialize()
    {
        m_impl->audioManager = std::make_unique<AudioManager>();
        return m_impl->audioManager->Initialize();
    }

    void Synthesizer::PlayNote(double frequency)
    {
        // Map frequency to internal note management
        m_impl->audioManager->AddNote(frequency);
    }

    // ... etc
}
```

**3. Example App**
```cpp
#include "SynthFramework.h"

int main()
{
    SynthFramework::Synthesizer synth;
    synth.Initialize();
    synth.SetWaveType(SynthFramework::WaveType::Sine);
    synth.PlayNote(440.0);  // A4

    Sleep(1000);
    synth.StopNote(440.0);
    return 0;
}
```

This wraps your existing code with a clean API layer!

---

## 📚 Resources

- **Audio Libraries for Inspiration**:
  - PortAudio: http://www.portaudio.com/
  - RtAudio: https://www.music.mcgill.ca/~gary/rtaudio/
  - JUCE: https://juce.com/

- **C++ Library Design**:
  - API Design for C++ (Martin Reddy)
  - Pimpl idiom: https://en.cppreference.com/w/cpp/language/pimpl

- **Build Systems**:
  - CMake Tutorial: https://cmake.org/cmake/help/latest/guide/tutorial/
  - vcpkg: https://vcpkg.io/

---

**Next Step**: Choose your approach:
1. **Minimal** (1 day): Wrap existing code with clean API
2. **Moderate** (1-2 weeks): Full refactor with platform abstraction
3. **Complete** (1 month): Production-ready framework with all features

Let me know which path you want to take and I can provide detailed implementation!
