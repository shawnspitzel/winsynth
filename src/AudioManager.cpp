#include "AudioManager.h"
#include "noiseMaker.h"
#include <cmath>

constexpr double TWO_PI = 2.0 * PI;

// Musical note frequencies (in Hz)
namespace NoteFrequencies
{
// Octave 4
constexpr double C4 = 261.626;
constexpr double D4 = 293.665;
constexpr double E4 = 329.628;
constexpr double F4 = 349.228;
constexpr double G4 = 392.000;
constexpr double A4 = 440.000;
constexpr double B4 = 493.883;

// Octave 5
constexpr double C5 = 523.25;
constexpr double D5 = 587.33;
constexpr double E5 = 659.25;
constexpr double F5 = 698.46;
constexpr double G5 = 783.99;
constexpr double A5 = 880.00;
constexpr double B5 = 987.77;

// Octave 6
constexpr double C6 = 1046.50;
constexpr double D6 = 1174.66;
constexpr double E6 = 1318.51;
} // namespace NoteFrequencies

// Virtual key codes for keyboard mapping
namespace VirtualKeys
{
constexpr WPARAM Q = 0x51;
constexpr WPARAM W = 0x57;
constexpr WPARAM E = 0x45;
constexpr WPARAM R = 0x52;
constexpr WPARAM T = 0x54;
constexpr WPARAM Y = 0x59;
constexpr WPARAM U = 0x55;
constexpr WPARAM I = 0x49;
constexpr WPARAM O = 0x4F;
constexpr WPARAM P = 0x50;
constexpr WPARAM Z = 0x5A;
constexpr WPARAM X = 0x58;
constexpr WPARAM C = 0x43;
constexpr WPARAM V = 0x56;
constexpr WPARAM B = 0x42;
constexpr WPARAM N = 0x4E;
constexpr WPARAM M = 0x4D;
} // namespace VirtualKeys

AudioManager* AudioManager::s_instance = nullptr;

AudioManager::AudioManager()
{
    s_instance = this;
}

AudioManager::~AudioManager()
{
    if (s_instance == this)
    {
        s_instance = nullptr;
    }
}

bool AudioManager::Initialize()
{
    std::vector<std::wstring> devices = NoiseMaker<int>::GetDevices();
    if (devices.empty())
    {
        return false;
    }

    m_sound = std::make_unique<NoiseMaker<int>>(devices[0]);
    m_sound->SetUserFunction(AudioManager::StaticNoiseCallback);
    return true;
}

void AudioManager::Shutdown()
{
    m_sound.reset();
}

void AudioManager::HandleKeyDown(WPARAM wParam)
{
    std::lock_guard<std::mutex> lock(m_notesMutex);
    if (m_activeNotes.find(wParam) == m_activeNotes.end())
    {
        MapNoteFrequency(wParam);
    }
}

void AudioManager::HandleKeyUp(WPARAM wParam)
{
    std::lock_guard<std::mutex> lock(m_notesMutex);
    m_activeNotes.erase(wParam);
}

void AudioManager::SetWaveType(WaveType type)
{
    std::lock_guard<std::mutex> lock(m_notesMutex);
    m_currentWaveType = type;
}

double AudioManager::StaticNoiseCallback(double dTime)
{
    if (s_instance)
    {
        WaveType currentType = s_instance->m_currentWaveType;
        if (currentType == WaveType::Sine)
        {
            return s_instance->MakeSineNoise(dTime);
        }
        else if (currentType == WaveType::Square)
        {
            return s_instance->MakeSquareNoise(dTime);
        }
    }
    return 0.0;
}

double AudioManager::SineSoundMaker(double freq, double dTime) const
{
    return sin(freq * TWO_PI * dTime);
}

double AudioManager::SquareSoundMaker(double freq, double dTime) const
{
    // Simple and efficient square wave: returns 1 or -1
    double phase = fmod(freq * dTime, 1.0);
    return (phase < 0.5) ? 1.0 : -1.0;
}

double AudioManager::MakeSineNoise(double dTime)
{
    std::lock_guard<std::mutex> lock(m_notesMutex);
    double dOutput = 0.0;
    for (const auto& key : m_activeNotes)
    {
        double freq = key.second;
        dOutput += SineSoundMaker(freq, dTime);
    }
    return dOutput * 0.5;
}

double AudioManager::MakeSquareNoise(double dTime)
{
    std::lock_guard<std::mutex> lock(m_notesMutex);
    double dOutput = 0.0;
    for (const auto& key : m_activeNotes)
    {
        double freq = key.second;
        dOutput += SquareSoundMaker(freq, dTime);
    }
    return dOutput * 0.5;
}

void AudioManager::MapNoteFrequency(WPARAM wParam)
{
    using namespace NoteFrequencies;
    using namespace VirtualKeys;

    double noteFreq = 0.0;

    // Top row: QWERTYUIOP maps to C5-E6
    if (wParam == Q)
        noteFreq = C5;
    else if (wParam == W)
        noteFreq = D5;
    else if (wParam == E)
        noteFreq = E5;
    else if (wParam == R)
        noteFreq = F5;
    else if (wParam == T)
        noteFreq = G5;
    else if (wParam == Y)
        noteFreq = A5;
    else if (wParam == U)
        noteFreq = B5;
    else if (wParam == I)
        noteFreq = C6;
    else if (wParam == O)
        noteFreq = D6;
    else if (wParam == P)
        noteFreq = E6;
    // Bottom row: ZXCVBNM maps to C4-B4
    else if (wParam == Z)
        noteFreq = C4;
    else if (wParam == X)
        noteFreq = D4;
    else if (wParam == C)
        noteFreq = E4;
    else if (wParam == V)
        noteFreq = F4;
    else if (wParam == B)
        noteFreq = G4;
    else if (wParam == N)
        noteFreq = A4;
    else if (wParam == M)
        noteFreq = B4;
    else
        return; // Unknown key

    m_activeNotes[wParam] = noteFreq;
}