#include "ImGuiLayer.h"
#include "D3D12App.h"
#include "Utils.h"

#include <cstdint>

#include "../ImGui/imgui.h"
#include "../ImGui/backends/imgui_impl_dx12.h"
#include "../ImGui/backends/imgui_impl_win32.h"

ImGuiLayer::ImGuiLayer()
    : m_gfxBackend(nullptr)
    , m_onlyCommonFormats(false)
{
    m_tableHeaders.reserve(9u);
    m_tableHeaders.emplace_back("DXGI Format");
    m_tableHeaders.emplace_back("Texture1D");
    m_tableHeaders.emplace_back("Texture2D");
    m_tableHeaders.emplace_back("Texture3D");
    m_tableHeaders.emplace_back("Texture Cube");
    m_tableHeaders.emplace_back("Render Target");
    m_tableHeaders.emplace_back("Depth Target");
    m_tableHeaders.emplace_back("Display");
    m_tableHeaders.emplace_back("Mipmaps");

    m_commonFormats.reserve(11u);
    m_commonFormats.emplace_back(DXGI_FORMAT_R8_UINT);
    m_commonFormats.emplace_back(DXGI_FORMAT_D16_UNORM);
    m_commonFormats.emplace_back(DXGI_FORMAT_R16_FLOAT);
    m_commonFormats.emplace_back(DXGI_FORMAT_R16G16_FLOAT);
    m_commonFormats.emplace_back(DXGI_FORMAT_R16G16B16A16_FLOAT);
    m_commonFormats.emplace_back(DXGI_FORMAT_D24_UNORM_S8_UINT);
    m_commonFormats.emplace_back(DXGI_FORMAT_R24G8_TYPELESS);
    m_commonFormats.emplace_back(DXGI_FORMAT_D32_FLOAT);
    m_commonFormats.emplace_back(DXGI_FORMAT_R8G8B8A8_UNORM);
    m_commonFormats.emplace_back(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
    m_commonFormats.emplace_back(DXGI_FORMAT_R10G10B10A2_UNORM);
}

void ImGuiLayer::OnRender()
{
    CreateMenuBar();
    CreateMainSupportTable();
}

void ImGuiLayer::CreateMenuBar()
{
    ImGui::BeginMainMenuBar();
    const ImVec2 menuBarSize = ImGui::GetWindowSize();

    if (ImGui::BeginMenu("File"))
    {
        ImGui::MenuItem("Export to text file");
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Config"))
    {
        ImGui::MenuItem("Only show 'common' formats", nullptr, &m_onlyCommonFormats);
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    // Start table at end of menu bar
    ImGui::SetNextWindowPos(ImVec2{ 0.0f, menuBarSize.y });
    ImGui::SetNextWindowSize(ImVec2{ static_cast<float>(m_gfxBackend->GetWidth()), static_cast<float>(m_gfxBackend->GetHeight()) - menuBarSize.y });
}

void ImGuiLayer::CreateMainSupportTable()
{
    ImGui::Begin("#table", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    ImGui::Columns(2);
    ImGui::Text("GPU: '%s'", m_gfxBackend->GetGPUInfo().Name.c_str());
    ImGui::SameLine();
    ImGui::NextColumn();
    ImGui::Text("Max DirectX version: %s", m_gfxBackend->GetGPUInfo().MaxFeatureLevel.c_str());
    ImGui::NextColumn();
    ImGui::Text("VRAM: %.2f GB", BytesToGigbibytes(m_gfxBackend->GetGPUInfo().VRAMBytes));
    ImGui::NextColumn();
    ImGui::Text("Shared memory: %.2f GB", BytesToGigbibytes(m_gfxBackend->GetGPUInfo().SharedSystemMemBytes));
    ImGui::Columns(1);

    constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
    ImGui::BeginTable("Formats", static_cast<int>(m_tableHeaders.size()), tableFlags);

    // Setup table header and freeze it 
    ImGui::TableSetupScrollFreeze(0, 1);
    for (uint32_t column = 0; column < m_tableHeaders.size(); column++)
        ImGui::TableSetupColumn(m_tableHeaders[column]);
    ImGui::TableHeadersRow();

    // Main table render
    const auto& supportTable = m_gfxBackend->GetSupportTable();

    for (uint32_t row = 0; row < supportTable.size(); row++)
    {
        if (m_onlyCommonFormats)
        {
            bool commonNotFound = true;
            for (const DXGI_FORMAT format : m_commonFormats)
            {
                if (static_cast<DXGI_FORMAT>(row) == format)
                {
                    commonNotFound = false;
                    break;
                }
            }
            if (commonNotFound)
                continue;
        }

        ImGui::TableNextRow();
        for (uint32_t column = 0; column < m_tableHeaders.size(); column++)
        {
            ImGui::TableSetColumnIndex(column);
            if (column == 0)
            {
                ImGui::Text("%s", D3DFormatToString(static_cast<DXGI_FORMAT>(row)));  // Print DXGI format
                continue;
            }

            const auto found = supportTable.find(static_cast<DXGI_FORMAT>(row));
            if (found == supportTable.end())
                continue;

            const std::array<FormatSupport, SUPPORT_FLAGS_COUNT>& supportFlags = found->second;
            const FormatSupport support = supportFlags[column - 1u]; // -1 as column 0 is taken for format text. 

            switch (support)
            {
            case FormatSupport::PASS:
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
                ImGui::Button("PASS");
                break;
            }
            case FormatSupport::FAIL:
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
                ImGui::Button("FAIL");
                break;
            }
            case FormatSupport::UNKN:
            default:
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f });
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f });
                ImGui::Button("UNKN");
                break;
            }
            }
            ImGui::PopStyleColor(3);
            
        }
    }
    ImGui::EndTable();
    ImGui::End();
}
