#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

// D3D12 / DXGI stuff
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d12")
#include <d3d12.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

// Corresponds number of bit flags checked in CreateFormatSupportTable()
// or the number of column headers minus one
#define SUPPORT_FLAGS_COUNT (8)

#include "ImGuiLayer.h"

struct GPUInfo
{
    size_t VRAMBytes; 
    size_t SharedSystemMemBytes; // GPU resources in RAM aka things in upload heaps
    std::string MaxFeatureLevel;
    std::string Name;

    GPUInfo()
        : VRAMBytes(0), SharedSystemMemBytes(0) {}
};

class D3D12App
{
public:
    D3D12App(const uint32_t clientWidth, const uint32_t clientHeight);

    void Initialise(); // Creates resources & setups ImGui
    void Shutdown();

    void OnRender();
    void OnResize(const uint32_t width, const uint32_t height, const bool minimized);

    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }

    const GPUInfo& GetGPUInfo() const { return m_gpuInfo; }
    bool ExportFormatSupportTable();

    // Events
    void OpenFileDialogue() { m_openFileDialogue = true; }
    void SendFileSaveErrorEvent() { m_fileSaveError = true; }
    
    const std::vector<const char*>& GetTableHeaders() const { return m_tableHeaders; }
    const std::unordered_map<DXGI_FORMAT, std::array<FormatSupport, SUPPORT_FLAGS_COUNT>>& GetSupportTable() const { return m_supportTable; }
private:
    D3D_FEATURE_LEVEL FindHighestSupportedFLForDevice(ID3D12Device* device) const;
    const char* FeatureLevelToString(const D3D_FEATURE_LEVEL FL) const;

    void CreateDeviceAndQueue();
    void CreateSyncObjects();
    void CreateSwapChain();
    void CreateRenderTargets();
    void InitialiseImGui();

    void HandleEvents();
    void BeginFrame();
    void EndFrame();

    void MoveToNextFrame();
    void WaitForGPU();

    void CreateFormatSupportTable();

    GPUInfo m_gpuInfo;
    ImGuiLayer m_imguiLayer;

    bool m_fileSaveError : 1;
    bool m_openFileDialogue : 1;
    std::vector<const char*> m_tableHeaders;
    std::unordered_map<DXGI_FORMAT, std::array<FormatSupport, SUPPORT_FLAGS_COUNT>> m_supportTable;
    
    uint32_t m_width;
    uint32_t m_height;
    bool m_minimized;

    const uint8_t m_backBufferCount;
    uint8_t m_backBufferIdx;

    // D3D12 boilerplate
    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissor;

    D3D_FEATURE_LEVEL m_maxFL;
    ComPtr<IDXGIFactory7> m_factory;
    ComPtr<ID3D12Device8> m_device;
    ComPtr<ID3D12CommandQueue> m_cmdQueue;
    std::vector<ComPtr<ID3D12CommandAllocator>> m_cmdAllocators;
    ComPtr<ID3D12GraphicsCommandList> m_cmdList;

    HANDLE m_fenceEvent = INVALID_HANDLE_VALUE;
    ComPtr<ID3D12Fence> m_fence;
    std::vector<uint64_t> m_fenceValues;

    ComPtr<IDXGISwapChain3> m_swapChain;
    std::vector<ComPtr<ID3D12Resource>> m_backBufferTargets;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_backBufferRTHandles;

    ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
    ComPtr<ID3D12DescriptorHeap> m_imGuiDescriptorHeap;
};
