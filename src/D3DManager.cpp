#include "D3DManager.h"

#include "imgui_impl_dx9.h"

D3DManager::D3DManager(HWND hWnd) : m_hWnd(hWnd) {}

D3DManager::~D3DManager() {}

bool D3DManager::Initialize()
{
    if ((m_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;
    ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
    m_d3dpp.Windowed = TRUE;
    m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    m_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    m_d3dpp.EnableAutoDepthStencil = TRUE;
    m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    if (m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
                             D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_d3dpp, &m_pd3dDevice) < 0)
        return false;

    return true;
}

void D3DManager::Shutdown()
{
    if (m_pd3dDevice)
    {
        m_pd3dDevice->Release();
        m_pd3dDevice = nullptr;
    }
    if (m_pD3D)
    {
        m_pD3D->Release();
        m_pD3D = nullptr;
    }
}

void D3DManager::ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = m_pd3dDevice->Reset(&m_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

HRESULT D3DManager::Present()
{
    return m_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
}

HRESULT D3DManager::TestDeviceCooperativeLevel()
{
    return m_pd3dDevice->TestCooperativeLevel();
}

void D3DManager::SetBackBufferSize(UINT width, UINT height)
{
    m_d3dpp.BackBufferWidth = width;
    m_d3dpp.BackBufferHeight = height;
}