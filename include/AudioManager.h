#pragma once

#include "noiseMaker.h"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class AudioManager
{
public:
    enum class WaveType
    {
        Sine,
        Square
    };

    AudioManager();
    ~AudioManager();

    bool Initialize();
    void Shutdown();

    void HandleKeyDown(WPARAM wParam);
    void HandleKeyUp(WPARAM wParam);
    void SetWaveType(WaveType type);
    double MakeSineNoise(double dTime);
    double MakeSquareNoise(double dTime);

private:
    std::unique_ptr<NoiseMaker<int>> m_sound;
    std::unordered_map<WPARAM, double> m_activeNotes;
    mutable std::mutex m_notesMutex; // Protects m_activeNotes
    WaveType m_currentWaveType = WaveType::Sine;
    static double StaticNoiseCallback(double dTime);
    double SineSoundMaker(double freq, double dTime) const;
    double SquareSoundMaker(double freq, double dTime) const;
    void MapNoteFrequency(WPARAM wParam);
    static AudioManager* s_instance;
};