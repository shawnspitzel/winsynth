#include "App.h"

#include "AudioManager.h"
#include "D3DManager.h"
#include "GuiManager.h"

#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam,
                                                             LPARAM lParam);

App::App()
{
    ImGui_ImplWin32_EnableDpiAwareness();
    m_mainScale = ImGui_ImplWin32_GetDpiScaleForMonitor(
        ::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));
}

App::~App() {}

bool App::Initialize()
{
    if (!CreateAppWindow())
    {
        return false;
    }

    m_d3dManager = std::make_unique<D3DManager>(m_hWnd);
    if (!m_d3dManager->Initialize())
    {
        CleanupAppWindow();
        return false;
    }

    m_guiManager = std::make_unique<GuiManager>(m_hWnd, m_d3dManager->GetDevice(), m_mainScale);
    if (!m_guiManager->Initialize())
    {
        m_d3dManager->Shutdown();
        CleanupAppWindow();
        return false;
    }

    m_audioManager = std::make_unique<AudioManager>();
    if (!m_audioManager->Initialize())
    {
        m_guiManager->Shutdown();
        m_d3dManager->Shutdown();
        CleanupAppWindow();
        return false;
    }

    return true;
}

void App::Run()
{
    ::ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    ::UpdateWindow(m_hWnd);

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;
        if (m_d3dManager->IsDeviceLost())
        {
            HRESULT hr = m_d3dManager->TestDeviceCooperativeLevel();
            if (hr == D3DERR_DEVICELOST)
            {
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET)
                m_d3dManager->ResetDevice();
            m_d3dManager->ClearDeviceLostFlag();
        }
        UINT resizeWidth = m_d3dManager->GetResizeWidth();
        UINT resizeHeight = m_d3dManager->GetResizeHeight();
        if (resizeWidth != 0 && resizeHeight != 0)
        {
            m_d3dManager->SetBackBufferSize(resizeWidth, resizeHeight);
            m_d3dManager->ResetDevice();
            m_d3dManager->ClearResizeFlags();
        }
        m_guiManager->NewFrame();
        if (ImGui::Begin("Synthesizer Control", nullptr))
        {
            if (ImGui::Button("Sine Wave"))
            {
                m_audioManager->SetWaveType(AudioManager::WaveType::Sine);
            }
            ImGui::SameLine();
            if (ImGui::Button("Square Wave"))
            {
                m_audioManager->SetWaveType(AudioManager::WaveType::Square);
            }
        }
        ImGui::End();
        m_guiManager->Render(m_d3dManager->GetDevice(), m_d3dManager->GetClearColor());
        HRESULT result = m_d3dManager->Present();
        if (result == D3DERR_DEVICELOST)
            m_d3dManager->SetDeviceLostFlag(true);
    }
}

void App::Shutdown()
{
    m_audioManager->Shutdown();
    m_guiManager->Shutdown();
    m_d3dManager->Shutdown();
    CleanupAppWindow();
}

bool App::CreateAppWindow()
{
    m_wc = {sizeof(m_wc),
            CS_CLASSDC,
            WndProc,
            0L,
            0L,
            GetModuleHandle(nullptr),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            L"Keyboard Synthesizer",
            nullptr};
    ::RegisterClassExW(&m_wc);
    m_hWnd = ::CreateWindowW(m_wc.lpszClassName, L"Keyboard Sound Synthesizer", WS_OVERLAPPEDWINDOW,
                             100, 100, (int)(1280 * m_mainScale), (int)(800 * m_mainScale), nullptr,
                             nullptr, m_wc.hInstance, nullptr);
    return m_hWnd != nullptr;
}

void App::CleanupAppWindow()
{
    if (m_hWnd)
    {
        ::DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
    if (m_wc.hInstance)
    {
        ::UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
        m_wc.hInstance = nullptr;
    }
}

LRESULT CALLBACK App::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    if (g_AppInstance)
    {
        if (g_AppInstance->m_audioManager)
        {
            if (msg == WM_KEYDOWN)
            {
                g_AppInstance->m_audioManager->HandleKeyDown(wParam);
            }
            else if (msg == WM_KEYUP)
            {
                g_AppInstance->m_audioManager->HandleKeyUp(wParam);
            }
        }

        if (g_AppInstance->m_d3dManager)
        {
            if (msg == WM_SIZE)
            {
                if (wParam == SIZE_MINIMIZED)
                    return 0;
                g_AppInstance->m_d3dManager->SetResizeFlags((UINT)LOWORD(lParam),
                                                            (UINT)HIWORD(lParam));
                return 0;
            }
        }
    }

    switch (msg)
    {
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}