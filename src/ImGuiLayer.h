#pragma once
#include <vector>
#include <dxgi1_6.h>

class D3D12App;

enum class FormatSupport : int8_t
{
    UNKN = -1,
    FAIL = 0,
    PASS = 1,

    COUNT
};

class ImGuiLayer
{
public:
    ImGuiLayer();

    void BindBackend(D3D12App& backend) { m_gfxBackend = &backend; }
    void OnRender();

private:
    void CreateMenuBar();
    void CreateMainSupportTable();

    D3D12App* m_gfxBackend;
    bool m_onlyCommonFormats;
    std::vector<const char*> m_tableHeaders;
    std::vector<DXGI_FORMAT> m_commonFormats;
};