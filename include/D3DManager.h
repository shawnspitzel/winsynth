#pragma once

#include "imgui.h"

#include <d3d9.h>

class D3DManager
{
public:
    D3DManager(HWND hWnd);
    ~D3DManager();

    bool Initialize();
    void Shutdown();
    void ResetDevice();
    HRESULT Present();
    HRESULT TestDeviceCooperativeLevel();

    IDirect3DDevice9* GetDevice() const
    {
        return m_pd3dDevice;
    }
    ImVec4 GetClearColor() const
    {
        return m_clearColor;
    }

    bool IsDeviceLost() const
    {
        return m_DeviceLost;
    }
    void SetDeviceLostFlag(bool lost)
    {
        m_DeviceLost = lost;
    }
    void ClearDeviceLostFlag()
    {
        m_DeviceLost = false;
    }

    UINT GetResizeWidth() const
    {
        return m_ResizeWidth;
    }
    UINT GetResizeHeight() const
    {
        return m_ResizeHeight;
    }
    void SetResizeFlags(UINT width, UINT height)
    {
        m_ResizeWidth = width;
        m_ResizeHeight = height;
    }
    void ClearResizeFlags()
    {
        m_ResizeWidth = 0;
        m_ResizeHeight = 0;
    }
    void SetBackBufferSize(UINT width, UINT height);

private:
    HWND m_hWnd;
    LPDIRECT3D9 m_pD3D = nullptr;
    LPDIRECT3DDEVICE9 m_pd3dDevice = nullptr;
    D3DPRESENT_PARAMETERS m_d3dpp = {};
    bool m_DeviceLost = false;
    UINT m_ResizeWidth = 0, m_ResizeHeight = 0;
    ImVec4 m_clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};