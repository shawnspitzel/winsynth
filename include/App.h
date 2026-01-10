#pragma once

#include <Windows.h>
#include <memory>
class GuiManager;
class D3DManager;
class AudioManager;

class App
{
public:
    App();
    ~App();

    bool Initialize();
    void Run();
    void Shutdown();
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    HWND m_hWnd = nullptr;
    WNDCLASSEXW m_wc = {};
    float m_mainScale = 1.0f;

    std::unique_ptr<D3DManager> m_d3dManager;
    std::unique_ptr<GuiManager> m_guiManager;
    std::unique_ptr<AudioManager> m_audioManager;

    bool CreateAppWindow();
    void CleanupAppWindow();
};

// Global instance pointer for window procedure callback
extern App* g_AppInstance;