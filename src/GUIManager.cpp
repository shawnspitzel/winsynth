#include "GuiManager.h"

#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#include <d3d9.h>

GuiManager::GuiManager(HWND hWnd, IDirect3DDevice9* device, float dpiScale)
    : m_hWnd(hWnd), m_device(device), m_dpiScale(dpiScale)
{
}

GuiManager::~GuiManager() {}

bool GuiManager::Initialize()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(m_dpiScale);
    style.FontScaleDpi = m_dpiScale;

    ImGui_ImplWin32_Init(m_hWnd);
    ImGui_ImplDX9_Init(m_device);

    return true;
}

void GuiManager::NewFrame()
{
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void GuiManager::Render(IDirect3DDevice9* device, ImVec4 clearColor)
{
    ImGui::EndFrame();
    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(
        (int)(clearColor.x * clearColor.w * 255.0f), (int)(clearColor.y * clearColor.w * 255.0f),
        (int)(clearColor.z * clearColor.w * 255.0f), (int)(clearColor.w * 255.0f));
    device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
    if (device->BeginScene() >= 0)
    {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        device->EndScene();
    }
}

void GuiManager::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}