#pragma once

#pragma comment(lib, "winmm.lib")

#include <Windows.h>
#include <atomic>
#include <cmath>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

constexpr double PI = 3.14159265358979323846;

template <class T>
class NoiseMaker
{
public:
    NoiseMaker(std::wstring sOutputDevice, unsigned int nSampleRate = 44100,
               unsigned int nChannels = 1, unsigned int nBlocks = 8,
               unsigned int nBlockSamples = 512) // leave device name for user input
    {
        Create(sOutputDevice, nSampleRate, nChannels, nBlocks, nBlockSamples);
    }
    ~NoiseMaker()
    {
        Destroy();
        // Clean up allocated memory
        if (m_pBlockMemory != nullptr)
        {
            delete[] m_pBlockMemory;
            m_pBlockMemory = nullptr;
        }
        if (m_pWaveHeaders != nullptr)
        {
            delete[] m_pWaveHeaders;
            m_pWaveHeaders = nullptr;
        }
    }

    bool Create(std::wstring sOutputDevice, unsigned int nSampleRate = 44100,
                unsigned int nChannels = 1, unsigned int nBlocks = 8,
                unsigned int nBlockSamples = 512)
    {
        m_bReady = false;
        m_nSampleRate = nSampleRate;
        m_nChannels = nChannels;
        m_nBlockCount = nBlocks;
        m_nBlockSamples = nBlockSamples;
        m_nBlockFree = m_nBlockCount;
        m_nBlockCurrent = 0;
        m_pBlockMemory = nullptr;
        m_pWaveHeaders = nullptr;

        m_userFunction = nullptr;

        std::vector<std::wstring> devices = GetDevices(); // get list of all devices
        auto d = std::find(
            devices.begin(), devices.end(),
            sOutputDevice); // Find the given output device within list of available devices
        if (d != devices.end())
        {
            // Device is available
            int nDeviceID = distance(devices.begin(), d);
            WAVEFORMATEX waveFormat;                 // information regarding our audio
            waveFormat.wFormatTag = WAVE_FORMAT_PCM; // raw, uncompressed audio format, most common
            waveFormat.nSamplesPerSec = m_nSampleRate; // use our inputted sample rate
            waveFormat.wBitsPerSample = 32; // 16 bits is good, 24 is great, 32 is excellent
            waveFormat.nChannels =
                m_nChannels; // use our inputted number of channels (1 mono, 2 stereo)
            waveFormat.nBlockAlign =
                (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels; // standard formula
            waveFormat.nAvgBytesPerSec =
                waveFormat.nSamplesPerSec * waveFormat.nBlockAlign; // standard formula
            waveFormat.cbSize = 0;

            if (waveOutOpen(&m_hwDevice, nDeviceID, &waveFormat, (DWORD_PTR)waveOutProcWrap,
                            (DWORD_PTR)this, CALLBACK_FUNCTION) != S_OK)
                return Destroy();
        }

        // Allocate Wave|Block Memory
        m_pBlockMemory = new T[m_nBlockCount * m_nBlockSamples];
        if (m_pBlockMemory == nullptr)
            return Destroy();
        ZeroMemory(m_pBlockMemory, sizeof(T) * m_nBlockCount * m_nBlockSamples);

        m_pWaveHeaders = new WAVEHDR[m_nBlockCount];
        if (m_pWaveHeaders == nullptr)
            return Destroy();
        ZeroMemory(m_pWaveHeaders, sizeof(WAVEHDR) * m_nBlockCount);
        for (unsigned int n = 0; n < m_nBlockCount; n++)
        {
            m_pWaveHeaders[n].dwBufferLength = m_nBlockSamples * sizeof(T);
            m_pWaveHeaders[n].lpData = (LPSTR)(m_pBlockMemory + (n * m_nBlockSamples));
        }

        m_bReady = true;

        m_thread = std::thread(&NoiseMaker::MainThread, this);

        // Start the ball rolling
        std::unique_lock<std::mutex> lm(m_muxBlockNotZero);
        m_cvBlockNotZero.notify_one();

        return true;
    }

    bool Destroy()
    {
        if (this->m_bReady)
        {
            Stop();
        }
        return false;
    };

    void Stop()
    {
        m_bReady = false;
        m_thread.join();
    }
    virtual double UserProcess(double dTime)
    {
        return 0.0;
    }

    double GetTime()
    {
        return m_dGlobalTime;
    }

public:
    static std::vector<std::wstring>
    GetDevices() // Use wstring to hold wide character strings for device names
    {
        u_int nDeviceCount =
            waveOutGetNumDevs(); // get a list of all available audio devices on the system
        std::vector<std::wstring> sDevices;
        WAVEOUTCAPS deviceInfo;                  // Device info struct
        for (u_int n = 0; n < nDeviceCount; n++) // iterate through all available devices
            if (waveOutGetDevCaps(n, &deviceInfo, sizeof(WAVEOUTCAPS)) ==
                S_OK) // get information for each device via device ID, store within device object
                sDevices.push_back(deviceInfo.szPname);
        return sDevices;
    }

    void SetUserFunction(double (*func)(double))
    {
        m_userFunction = func;
    }

    double clip(double dSample, double dMax)
    {
        if (dSample >= 0.0)
            return fmin(dSample, dMax);
        else
            return fmax(dSample, -dMax);
    }

private:
    double (*m_userFunction)(double);

    unsigned int m_nSampleRate;
    unsigned int m_nChannels;
    unsigned int m_nBlockCount;
    unsigned int m_nBlockSamples;
    unsigned int m_nBlockCurrent;

    T* m_pBlockMemory;
    WAVEHDR* m_pWaveHeaders;
    HWAVEOUT m_hwDevice; // output device

    std::thread m_thread;
    bool m_bReady;
    std::atomic<unsigned int> m_nBlockFree;
    std::condition_variable m_cvBlockNotZero;
    std::mutex m_muxBlockNotZero;

    std::atomic<double> m_dGlobalTime;
    void waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwParam1, DWORD dwParam2)
    {
        if (uMsg != WOM_DONE)
            return;

        m_nBlockFree++;
        std::unique_lock<std::mutex> lm(m_muxBlockNotZero);
        m_cvBlockNotZero.notify_one();
    }
    static void CALLBACK waveOutProcWrap(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance,
                                         DWORD dwParam1, DWORD dwParam2)
    {
        ((NoiseMaker*)dwInstance)->waveOutProc(hWaveOut, uMsg, dwParam1, dwParam2);
    }
    void MainThread()
    {
        m_dGlobalTime = 0.0;
        double dTimeStep = 1.0 / (double)m_nSampleRate;

        // Goofy hack to get maximum integer for a type at run-time
        T nMaxSample = (T)pow(2, (sizeof(T) * 8) - 1) - 1;
        double dMaxSample = (double)nMaxSample;
        T nPreviousSample = 0;

        while (m_bReady)
        {
            if (m_nBlockFree == 0)
            {
                std::unique_lock<std::mutex> lm(m_muxBlockNotZero);
                m_cvBlockNotZero.wait(lm);
            }

            m_nBlockFree--;
            if (m_pWaveHeaders[m_nBlockCurrent].dwFlags & WHDR_PREPARED)
                waveOutUnprepareHeader(m_hwDevice, &m_pWaveHeaders[m_nBlockCurrent],
                                       sizeof(WAVEHDR));

            T nNewSample = 0;
            int nCurrentBlock = m_nBlockCurrent * m_nBlockSamples;

            for (unsigned int n = 0; n < m_nBlockSamples; n++)
            {
                if (m_userFunction == nullptr)
                    nNewSample = (T)(clip(UserProcess(m_dGlobalTime), 1.0) * dMaxSample);
                else
                    nNewSample = (T)(clip(m_userFunction(m_dGlobalTime), 1.0) * dMaxSample);

                m_pBlockMemory[nCurrentBlock + n] = nNewSample;
                nPreviousSample = nNewSample;
                m_dGlobalTime = m_dGlobalTime + dTimeStep;
            }
            waveOutPrepareHeader(m_hwDevice, &m_pWaveHeaders[m_nBlockCurrent], sizeof(WAVEHDR));
            waveOutWrite(m_hwDevice, &m_pWaveHeaders[m_nBlockCurrent], sizeof(WAVEHDR));
            m_nBlockCurrent++;
            m_nBlockCurrent %= m_nBlockCount;
        }
    }
};
