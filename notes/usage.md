# SynthFramework Usage Examples
## How Developers Would Use Your Library

---

## 🎯 Design Goal: From This → To This

### Current Usage (Application-Specific)
```cpp
// User must understand Windows API, ImGui, DirectX
#include "App.h"
#include "AudioManager.h"
#include <Windows.h>

App app;
app.Initialize();  // Sets up everything: audio, GUI, D3D
app.Run();         // Message loop, rendering, input
```

### Target Usage (Library)
```cpp
// Clean, cross-platform API
#include <SynthFramework.h>

SynthFramework::Synthesizer synth;
synth.Initialize();
synth.PlayNote(440.0);  // Just play a note!
```

---

## 📚 Example 1: Hello World Synthesizer

The simplest possible usage:

```cpp
#include <SynthFramework.h>
#include <thread>
#include <chrono>

int main()
{
    using namespace SynthFramework;

    // Create and initialize synthesizer
    Synthesizer synth;
    if (!synth.Initialize())
    {
        return 1;
    }

    // Play middle A (440 Hz) for 1 second
    synth.PlayNote(440.0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    synth.StopNote(440.0);

    return 0;
}
```

**Compilation:**
```bash
g++ hello_synth.cpp -lSynthFramework -o hello_synth
./hello_synth
```

---

## 📚 Example 2: Playing a Melody

```cpp
#include <SynthFramework.h>
#include <thread>
#include <vector>

struct Note
{
    double frequency;
    int durationMs;
};

int main()
{
    using namespace SynthFramework;

    Synthesizer synth;
    synth.Initialize();
    synth.SetWaveType(WaveType::Sine);

    // "Mary Had a Little Lamb" melody
    std::vector<Note> melody = {
        {Notes::NoteNameToFrequency("E4"), 500},
        {Notes::NoteNameToFrequency("D4"), 500},
        {Notes::NoteNameToFrequency("C4"), 500},
        {Notes::NoteNameToFrequency("D4"), 500},
        {Notes::NoteNameToFrequency("E4"), 500},
        {Notes::NoteNameToFrequency("E4"), 500},
        {Notes::NoteNameToFrequency("E4"), 1000},
    };

    for (const auto& note : melody)
    {
        synth.PlayNote(note.frequency);
        std::this_thread::sleep_for(std::chrono::milliseconds(note.durationMs));
        synth.StopNote(note.frequency);
    }

    return 0;
}
```

---

## 📚 Example 3: Interactive Keyboard Synthesizer

Recreating your current app, but using the library:

```cpp
#include <SynthFramework.h>
#include <iostream>
#include <map>

class KeyboardSynthApp
{
public:
    KeyboardSynthApp()
    {
        // Map keyboard keys to frequencies
        m_keyMap['a'] = Notes::NoteNameToFrequency("C4");
        m_keyMap['s'] = Notes::NoteNameToFrequency("D4");
        m_keyMap['d'] = Notes::NoteNameToFrequency("E4");
        m_keyMap['f'] = Notes::NoteNameToFrequency("F4");
        m_keyMap['g'] = Notes::NoteNameToFrequency("G4");
        m_keyMap['h'] = Notes::NoteNameToFrequency("A4");
        m_keyMap['j'] = Notes::NoteNameToFrequency("B4");
        m_keyMap['k'] = Notes::NoteNameToFrequency("C5");
    }

    void Run()
    {
        using namespace SynthFramework;

        if (!m_synth.Initialize())
        {
            std::cerr << "Failed to initialize\n";
            return;
        }

        m_synth.SetWaveType(WaveType::Sine);

        std::cout << "Keyboard Synthesizer\n";
        std::cout << "Keys: A S D F G H J K\n";
        std::cout << "      C D E F G A B C\n";
        std::cout << "Press Q to quit\n\n";

        while (true)
        {
            char key = GetKeyPress();

            if (key == 'q' || key == 'Q')
                break;

            auto it = m_keyMap.find(key);
            if (it != m_keyMap.end())
            {
                m_synth.PlayNote(it->second);

                // Hold note for 200ms
                std::this_thread::sleep_for(std::chrono::milliseconds(200));

                m_synth.StopNote(it->second);
            }
        }
    }

private:
    SynthFramework::Synthesizer m_synth;
    std::map<char, double> m_keyMap;

    char GetKeyPress()
    {
        // Cross-platform key input (simplified)
        char ch;
        std::cin >> ch;
        return ch;
    }
};

int main()
{
    KeyboardSynthApp app;
    app.Run();
    return 0;
}
```

---

## 📚 Example 4: Chord Player

```cpp
#include <SynthFramework.h>
#include <vector>
#include <string>

class ChordPlayer
{
public:
    void PlayChord(const std::string& chordName, int durationMs)
    {
        using namespace SynthFramework;

        std::vector<std::string> notes;

        // Define common chords
        if (chordName == "C major")
            notes = {"C4", "E4", "G4"};
        else if (chordName == "G major")
            notes = {"G4", "B4", "D5"};
        else if (chordName == "A minor")
            notes = {"A4", "C5", "E5"};
        else if (chordName == "F major")
            notes = {"F4", "A4", "C5"};

        // Play all notes simultaneously
        for (const auto& note : notes)
        {
            double freq = Notes::NoteNameToFrequency(note.c_str());
            m_synth.PlayNote(freq);
        }

        // Hold the chord
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));

        // Stop all notes
        for (const auto& note : notes)
        {
            double freq = Notes::NoteNameToFrequency(note.c_str());
            m_synth.StopNote(freq);
        }
    }

    bool Initialize()
    {
        return m_synth.Initialize();
    }

private:
    SynthFramework::Synthesizer m_synth;
};

int main()
{
    ChordPlayer player;
    if (!player.Initialize())
        return 1;

    // Play chord progression: C - G - Am - F
    player.PlayChord("C major", 1000);
    player.PlayChord("G major", 1000);
    player.PlayChord("A minor", 1000);
    player.PlayChord("F major", 1000);

    return 0;
}
```

---

## 📚 Example 5: MIDI File Player (Future Feature)

```cpp
#include <SynthFramework.h>
#include <SynthFramework/MIDI.h>
#include <iostream>

int main(int argc, char* argv[])
{
    using namespace SynthFramework;

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <midi_file.mid>\n";
        return 1;
    }

    // Initialize synthesizer
    Synthesizer synth;
    if (!synth.Initialize())
        return 1;

    // Load MIDI file
    MIDI::File midiFile;
    if (!midiFile.Load(argv[1]))
    {
        std::cerr << "Failed to load MIDI file\n";
        return 1;
    }

    // Create MIDI player
    MIDI::Player player(&synth);
    player.SetTempo(120);  // BPM

    // Set callbacks
    player.SetNoteOnCallback([&](int note, int velocity) {
        double freq = Notes::MIDINoteToFrequency(note);
        synth.PlayNote(freq);
    });

    player.SetNoteOffCallback([&](int note) {
        double freq = Notes::MIDINoteToFrequency(note);
        synth.StopNote(freq);
    });

    // Play the file
    std::cout << "Playing: " << argv[1] << "\n";
    player.Play(midiFile);

    // Wait for completion
    while (player.IsPlaying())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
```

---

## 📚 Example 6: Custom Waveform Generator

```cpp
#include <SynthFramework.h>
#include <cmath>

int main()
{
    using namespace SynthFramework;

    Synthesizer synth;
    synth.Initialize();

    // Define custom waveform: mix of sine and square
    auto customWaveform = [](double frequency, double time) -> double
    {
        double sine = Oscillator::Sine(frequency, time);
        double square = Oscillator::Square(frequency, time);
        return 0.7 * sine + 0.3 * square;  // 70% sine, 30% square
    };

    synth.SetCustomWaveform(customWaveform);

    // Play note with custom waveform
    synth.PlayNote(440.0);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    synth.StopNote(440.0);

    return 0;
}
```

---

## 📚 Example 7: Game Integration

```cpp
// game.h
#include <SynthFramework.h>

class Game
{
public:
    Game()
    {
        // Initialize audio
        m_synth.Initialize();
        m_synth.SetWaveType(SynthFramework::WaveType::Square);
        m_synth.SetVolume(0.5);
    }

    void OnPlayerJump()
    {
        // Play jump sound effect (ascending pitch)
        m_synth.PlayNote(220.0);  // A3
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        m_synth.StopNote(220.0);

        m_synth.PlayNote(440.0);  // A4
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        m_synth.StopNote(440.0);
    }

    void OnEnemyHit()
    {
        // Play hit sound (descending pitch)
        m_synth.PlayNote(880.0);  // A5
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        m_synth.StopNote(880.0);
    }

    void OnGameOver()
    {
        // Play game over melody
        PlayMelody({
            {262, 500}, // C4
            {247, 500}, // B3
            {220, 500}, // A3
            {196, 1000} // G3
        });
    }

private:
    SynthFramework::Synthesizer m_synth;

    void PlayMelody(const std::vector<std::pair<double, int>>& notes)
    {
        for (const auto& [freq, duration] : notes)
        {
            m_synth.PlayNote(freq);
            std::this_thread::sleep_for(std::chrono::milliseconds(duration));
            m_synth.StopNote(freq);
        }
    }
};
```

---

## 📚 Example 8: Multi-Instance Synthesizers

```cpp
#include <SynthFramework.h>

class DualSynthApp
{
public:
    void Run()
    {
        using namespace SynthFramework;

        // Create two independent synthesizers
        Synthesizer bass;
        Synthesizer lead;

        bass.Initialize();
        lead.Initialize();

        // Configure each differently
        bass.SetWaveType(WaveType::Sine);
        bass.SetVolume(0.7);

        lead.SetWaveType(WaveType::Square);
        lead.SetVolume(0.4);

        // Play bass line (low notes)
        bass.PlayNote(Notes::NoteNameToFrequency("C2"));

        // Play melody on top (high notes)
        lead.PlayNote(Notes::NoteNameToFrequency("C5"));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        lead.StopNote(Notes::NoteNameToFrequency("C5"));

        lead.PlayNote(Notes::NoteNameToFrequency("E5"));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        lead.StopNote(Notes::NoteNameToFrequency("E5"));

        bass.StopNote(Notes::NoteNameToFrequency("C2"));
    }
};
```

---

## 📚 Example 9: Real-Time Parameter Control

```cpp
#include <SynthFramework.h>
#include <thread>

void DemoLFOVibrato()
{
    using namespace SynthFramework;

    Synthesizer synth;
    synth.Initialize();

    double baseFreq = 440.0;  // A4

    // Vibrato effect using LFO (Low Frequency Oscillator)
    std::thread vibratoThread([&]() {
        while (true)
        {
            double time = GetTime();  // Get current time
            double lfo = std::sin(2 * 3.14159 * 5.0 * time);  // 5 Hz LFO
            double vibrato = baseFreq * (1.0 + 0.02 * lfo);   // ±2% pitch variation

            synth.StopNote(baseFreq);
            synth.PlayNote(vibrato);

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    // Let it run for 5 seconds
    std::this_thread::sleep_for(std::chrono::seconds(5));
    vibratoThread.detach();
}
```

---

## 📚 Example 10: Audio Plugin / VST-Style Interface

```cpp
// AudioPlugin.h - Your library as a plugin backend
#include <SynthFramework.h>

class SynthPlugin
{
public:
    // Plugin interface (VST-style)
    void Initialize(int sampleRate, int bufferSize)
    {
        AudioConfig config;
        config.sampleRate = sampleRate;
        config.bufferSize = bufferSize;
        m_synth.Initialize(config);
    }

    void ProcessMIDI(int midiNote, int velocity, bool noteOn)
    {
        double freq = Notes::MIDINoteToFrequency(midiNote);

        if (noteOn)
            m_synth.PlayNote(freq);
        else
            m_synth.StopNote(freq);
    }

    void SetParameter(int paramId, double value)
    {
        switch (paramId)
        {
        case 0:  // Waveform
            m_synth.SetWaveType(static_cast<WaveType>((int)value));
            break;
        case 1:  // Volume
            m_synth.SetVolume(value);
            break;
        case 2:  // Attack
            m_envelope.attack = value;
            m_synth.SetEnvelope(m_envelope);
            break;
        }
    }

private:
    SynthFramework::Synthesizer m_synth;
    SynthFramework::ADSREnvelope m_envelope;
};
```

---

## 🎨 Comparison: Before and After

### Your Current App (Application)
```cpp
// Tightly coupled to Windows, ImGui, D3D
int main()
{
    App app;  // Sets up everything: audio, GUI, rendering, input

    if (!app.Initialize())  // Windows-specific initialization
        return 1;

    app.Run();  // Message loop, key handling, rendering, audio all together
    app.Shutdown();

    return 0;
}
```

**Issues**:
- Can't reuse audio without GUI
- Can't reuse audio without Windows
- Must understand entire architecture to use

### Your Future Library (Reusable)
```cpp
// Clean, focused API
int main()
{
    SynthFramework::Synthesizer synth;
    synth.Initialize();
    synth.PlayNote(440.0);

    // User provides their own GUI, input, etc.
    // Library just handles audio synthesis

    return 0;
}
```

**Benefits**:
- Use in any project (games, DAWs, tools)
- Cross-platform (Windows, Mac, Linux)
- Easy to understand and integrate

---

## 🚀 Distribution: How Developers Get It

### Option 1: Header-Only (Easiest)
```cpp
// Single header include
#include "SynthFramework.h"

// Everything just works!
SynthFramework::Synthesizer synth;
```

### Option 2: Compiled Library (Faster build times)
```bash
# Install via package manager
vcpkg install synthframework

# Or build from source
git clone https://github.com/you/synthframework
cd synthframework && mkdir build && cd build
cmake ..
make install
```

```cpp
// Link against library
#include <SynthFramework.h>
// g++ app.cpp -lSynthFramework
```

### Option 3: Git Submodule (For developers)
```bash
# Add to your project
git submodule add https://github.com/you/synthframework libs/synthframework
```

```cmake
# CMakeLists.txt
add_subdirectory(libs/synthframework)
target_link_libraries(MyApp PRIVATE SynthFramework)
```

---

## 📊 Usage Comparison Table

| Task | Current (App) | Future (Library) |
|------|---------------|------------------|
| Play a note | Must run entire app, press keyboard | `synth.PlayNote(440.0)` |
| Change waveform | Click GUI button | `synth.SetWaveType(WaveType::Square)` |
| Use in game | Can't - it's a standalone app | Easy integration |
| Cross-platform | Windows only | Windows, Mac, Linux |
| Build time | 30+ seconds (ImGui, D3D) | <5 seconds |
| Dependencies | ImGui, DirectX SDK | None (or just platform audio) |
| Learning curve | Must understand GUI, D3D, Win32 | Just audio concepts |

---

**The Key Insight**: Your current app is like a **calculator program**. The library version is like a **math library** that any program can use.

Which approach do you want to pursue? I can help implement any of these!
