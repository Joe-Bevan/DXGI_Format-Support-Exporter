#pragma once

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class D3D12App;

class Win32App
{
public:
    static int Run(D3D12App* gfxBackend, const HINSTANCE instance);
    static HWND GetHandle() { return m_hwnd; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    static HWND m_hwnd;
    static DWORD m_style;
};