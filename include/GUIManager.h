#pragma once

#include "imgui.h"

#include <Windows.h>
struct IDirect3DDevice9;

class GuiManager
{
public:
    GuiManager(HWND hWnd, IDirect3DDevice9* device, float dpiScale);
    ~GuiManager();

    bool Initialize();
    void NewFrame();
    void Render(IDirect3DDevice9* device, ImVec4 clearColor);
    void Shutdown();

private:
    HWND m_hWnd;
    IDirect3DDevice9* m_device;
    float m_dpiScale;
    ImVec4 m_clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};