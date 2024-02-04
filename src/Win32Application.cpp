#include "Win32Application.h"
#include "../resource.h"

#include "D3D12App.h"

HWND Win32App::m_hwnd{ 0u };
DWORD Win32App::m_style{ 0u };

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); // Extern from IMGUI (used to get input data)

int Win32App::Run(D3D12App* gfxBackend, const HINSTANCE instance)
{
    m_style = WS_OVERLAPPEDWINDOW;
    
    // Initialize the window class.
    WNDCLASSEX windowClass;
    ZeroMemory(&windowClass, sizeof(windowClass));
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WndProc;
    windowClass.hInstance = instance;
    windowClass.hIcon = LoadIconW(instance, MAKEINTRESOURCE(IDI_ICON1));
    windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    windowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)); 
    windowClass.lpszClassName = L"DXGI Format Support Exporter";
    RegisterClassExW(&windowClass);

    RECT windowRect = { 0, 0, static_cast<LONG>(gfxBackend->GetWidth()), static_cast<LONG>(gfxBackend->GetHeight()) };
    AdjustWindowRect(&windowRect, m_style, FALSE);

    m_hwnd = CreateWindowW(
        windowClass.lpszClassName,
        L"DXGI Format Support Exporter - Developed by Joe Bevan",
        m_style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        
        nullptr,        
        instance,
        gfxBackend);

    gfxBackend->Initialise();

    ShowWindow(m_hwnd, SW_NORMAL);

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    gfxBackend->Shutdown();
    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK Win32App::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
        return true;

    D3D12App* gfxBackend = reinterpret_cast<D3D12App*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
    {
        // Save the D3D12App* passed in to CreateWindow.
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        return 0;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }

    case WM_SIZE:
    {
        if (gfxBackend)
        {
            RECT clientRect = {};
            GetClientRect(hwnd, &clientRect);
            gfxBackend->OnResize(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, wParam == SIZE_MINIMIZED);
        }
        return 0;
    }

    case WM_PAINT:
    {
        if (gfxBackend)
        {
            gfxBackend->OnRender();
        }
        return 0;
    }
    }
    // Handle any messages the switch statement didn't.
    return DefWindowProc(hwnd, message, wParam, lParam);
}