#include "D3D12App.h"
#include "Win32Application.h"

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    D3D12App gfxBackend(960u, 720u);
    return Win32App::Run(&gfxBackend, hInstance);
}