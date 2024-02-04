#include "D3D12App.h"

#include "Win32Application.h"
#include "Utils.h"

#include "d3dx12.h"
#include "../ImGui/imgui.h"
#include "../ImGui/backends/imgui_impl_dx12.h"
#include "../ImGui/backends/imgui_impl_win32.h"

D3D12App::D3D12App(const uint32_t clientWidth, const uint32_t clientHeight)
    : m_width(clientWidth)
    , m_height(clientHeight)
    , m_minimized(false)
    , m_backBufferCount(2u)
    , m_maxFL(D3D_FEATURE_LEVEL_11_0)
    , m_backBufferIdx(0)
    , m_viewport{ 0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height) }
    , m_scissor{ 0u, 0u, static_cast<LONG>(m_width), static_cast<LONG>(m_height) }
{
    m_imguiLayer.BindBackend(*this);
}

void D3D12App::Initialise()
{
    CreateDeviceAndQueue();
    CreateSyncObjects();
    CreateSwapChain();
    InitialiseImGui();
    CreateFormatSupportTable();
}

void D3D12App::Shutdown()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CloseHandle(m_fenceEvent);
}

void D3D12App::BeginFrame()
{
    // Reset command allocator. Must be done *after* the GPU is finished with it
    D3D_VERIFY(m_cmdAllocators[m_backBufferIdx]->Reset());

    // Reset the command list (it should be closed by this point)
    D3D_VERIFY(m_cmdList->Reset(m_cmdAllocators[m_backBufferIdx].Get(), nullptr));

    const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferTargets[m_backBufferIdx].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_cmdList->ResourceBarrier(1, &barrier);
    m_cmdList->RSSetViewports(1u, &m_viewport);
    m_cmdList->RSSetScissorRects(1u, &m_scissor);
    m_cmdList->OMSetRenderTargets(1u, &m_backBufferRTHandles[m_backBufferIdx], FALSE, nullptr);
    m_cmdList->SetDescriptorHeaps(1u, m_imGuiDescriptorHeap.GetAddressOf());

    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX12_NewFrame();
    ImGui::NewFrame();
}

void D3D12App::OnRender()
{
    if (m_minimized)
        return;

    BeginFrame();

    m_imguiLayer.OnRender();

    EndFrame();
}

void D3D12App::OnResize(const uint32_t width, const uint32_t height, const bool minimized)
{
    m_minimized = minimized;
    if ((width != m_width || height != m_height) && !minimized)
    {
        // Flush GPU commands
        WaitForGPU();

        // Release the resources holding references to the swap chain 
        // and reset the frame fence values to the current fence value.
        for (UINT n = 0; n < m_backBufferCount; n++)
        {
            m_backBufferTargets[n].Reset();
            m_fenceValues[n] = m_fenceValues[m_backBufferIdx];
        }
        m_backBufferTargets.clear();

        // Resize swap chain
        DXGI_SWAP_CHAIN_DESC desc = {};
        m_swapChain->GetDesc(&desc);
        D3D_VERIFY(m_swapChain->ResizeBuffers(m_backBufferCount, width, height, desc.BufferDesc.Format, desc.Flags));

        // Reset the frame index to the current back buffer index.
        m_backBufferIdx = m_swapChain->GetCurrentBackBufferIndex();

        // Update variables to match new size
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));

        m_width = width;
        m_height = height;
        m_viewport = D3D12_VIEWPORT{0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height) };
        m_scissor = D3D12_RECT{ 0u, 0u, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };

        CreateRenderTargets();
    }
}

void D3D12App::EndFrame()
{
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_cmdList.Get());

    const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferTargets[m_backBufferIdx].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_cmdList->ResourceBarrier(1, &barrier);

    D3D_VERIFY(m_cmdList->Close());
    m_cmdQueue->ExecuteCommandLists(1u, reinterpret_cast<ID3D12CommandList**>(m_cmdList.GetAddressOf()));

    m_swapChain->Present(1u, 0u);

    MoveToNextFrame();
}

void D3D12App::MoveToNextFrame()
{
    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_fenceValues[m_backBufferIdx];
    D3D_VERIFY(m_cmdQueue->Signal(m_fence.Get(), currentFenceValue));

    // Update the frame index.
    m_backBufferIdx = m_swapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (m_fence->GetCompletedValue() < m_fenceValues[m_backBufferIdx])
    {
        D3D_VERIFY(m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIdx], m_fenceEvent));
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }

    // Set the fence value for the next frame.
    m_fenceValues[m_backBufferIdx] = currentFenceValue + 1;
}

void D3D12App::WaitForGPU()
{
    // Schedule a Signal command in the queue.
    D3D_VERIFY(m_cmdQueue->Signal(m_fence.Get(), m_fenceValues[m_backBufferIdx]));

    // Wait until the fence has been processed.
    D3D_VERIFY(m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIdx], m_fenceEvent));
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    // Increment the fence value for the current frame.
    m_fenceValues[m_backBufferIdx]++;
}

void D3D12App::CreateFormatSupportTable()
{
    // Cast the last enum we care about to int to get its index in the enum list.
    const size_t numberFormats = static_cast<size_t>(DXGI_FORMAT_B4G4R4A4_UNORM);
    for (size_t i = 0u; i < numberFormats; ++i)
    {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { static_cast<DXGI_FORMAT>(i), D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
        if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
        {
            // Note: If you wish to add anything here, also update "m_tableHeaders" in ImGuiLayer.cpp
            std::array<FormatSupport, SUPPORT_FLAGS_COUNT> supportFlags{};
            supportFlags[0] = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE1D ? FormatSupport::PASS : FormatSupport::FAIL);
            supportFlags[1] = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE2D ? FormatSupport::PASS : FormatSupport::FAIL);
            supportFlags[2] = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE3D ? FormatSupport::PASS : FormatSupport::FAIL);
            supportFlags[3] = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURECUBE ? FormatSupport::PASS : FormatSupport::FAIL);
            supportFlags[4] = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET ? FormatSupport::PASS : FormatSupport::FAIL);
            supportFlags[5] = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL ? FormatSupport::PASS : FormatSupport::FAIL);
            supportFlags[6] = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_DISPLAY ? FormatSupport::PASS : FormatSupport::FAIL);
            supportFlags[7] = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_MIP ? FormatSupport::PASS : FormatSupport::FAIL);

            m_supportTable[static_cast<DXGI_FORMAT>(i)] = supportFlags;
        }
        else
        {
            // Could not check feature support
            std::array<FormatSupport, SUPPORT_FLAGS_COUNT> supportFlags{};
            supportFlags.fill(FormatSupport::UNKN);
            m_supportTable[static_cast<DXGI_FORMAT>(i)] = supportFlags;
        }

    }

}

D3D_FEATURE_LEVEL D3D12App::FindHighestSupportedFLForDevice(ID3D12Device* device) const
{
    const static D3D_FEATURE_LEVEL featureLevelsArr[] =
    {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevels = {
        _countof(featureLevelsArr), featureLevelsArr, D3D_FEATURE_LEVEL_11_0
    };

    D3D_FEATURE_LEVEL maxSupportedFeatureLevel = D3D_FEATURE_LEVEL_11_0;
    if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevels, sizeof(featureLevels))))
    {
        maxSupportedFeatureLevel = featureLevels.MaxSupportedFeatureLevel;
    }
    return maxSupportedFeatureLevel;
}

const char* D3D12App::FeatureLevelToString(const D3D_FEATURE_LEVEL FL) const
{
    switch (FL)
    {
    case D3D_FEATURE_LEVEL_12_1: return "12_1";
    case D3D_FEATURE_LEVEL_12_0: return "12_0";
    case D3D_FEATURE_LEVEL_11_1: return "11_1";
    case D3D_FEATURE_LEVEL_11_0: return "11_0";
    default:
        return "Unknown feature level";
    }
}

void D3D12App::CreateDeviceAndQueue()
{
    constexpr uint32_t factoryFlags{ 0u };
    D3D_VERIFY(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory)));

    // Find the highest performance adapter
    ComPtr<IDXGIAdapter1> adapter;
    for (UINT adapterIndex = 0;
        DXGI_ERROR_NOT_FOUND != m_factory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
            IID_PPV_ARGS(&adapter)); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        D3D_VERIFY(adapter->GetDesc1(&desc));

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)  // Don't select the Basic Render Driver adapter.
            continue;

        // Check to see if the adapter supports Direct3D 12,
        // but don't create the actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device8), nullptr)))
        {
            m_gpuInfo.VRAMBytes = desc.DedicatedVideoMemory;
            m_gpuInfo.SharedSystemMemBytes = desc.SharedSystemMemory;
            const std::wstring gpuName(desc.Description);
            m_gpuInfo.Name = UTF16toUTF8(gpuName);
            break;
        }
    }

    // Create device at lowest feature level for D3D12. 
    D3D_VERIFY(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
    m_maxFL = FindHighestSupportedFLForDevice(m_device.Get());
    m_gpuInfo.MaxFeatureLevel = D3DFeatureLevelToString(m_maxFL);

    // Recreate our device with the highest feature level this device has
    m_device.Detach();
    D3D_VERIFY(D3D12CreateDevice(adapter.Get(), m_maxFL, IID_PPV_ARGS(&m_device)));

    // Create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0u;
    D3D_VERIFY(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_cmdQueue)));

    m_cmdAllocators.resize(m_backBufferCount);
    for (auto& allocator : m_cmdAllocators)
    {
        D3D_VERIFY(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)));
    }
    D3D_VERIFY(m_device->CreateCommandList(0u, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_cmdList)));
    D3D_VERIFY(m_cmdList->Close());
}

void D3D12App::CreateSyncObjects()
{
    D3D_VERIFY(m_device->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
    m_fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    m_fenceValues.resize(m_backBufferCount);
    m_fenceValues[m_backBufferIdx]++;
}

void D3D12App::CreateSwapChain()
{
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = m_backBufferCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1u;
    swapChainDesc.Flags = 0u;

    ComPtr<IDXGISwapChain1> swapChain;
    D3D_VERIFY(m_factory->CreateSwapChainForHwnd(
        m_cmdQueue.Get(),
        Win32App::GetHandle(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ));

    D3D_VERIFY(swapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain)));

    // Create RTV heap
    D3D12_DESCRIPTOR_HEAP_DESC descriptorDesc;
    descriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    descriptorDesc.NumDescriptors = m_backBufferCount;
    descriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    descriptorDesc.NodeMask = 1;
    D3D_VERIFY(m_device->CreateDescriptorHeap(&descriptorDesc, IID_PPV_ARGS(&m_descriptorHeap)));

    // Get RTV handles from heap
    size_t rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_backBufferRTHandles.resize(m_backBufferCount);
    for (uint8_t i = 0u; i < m_backBufferCount; ++i)
    {
        m_backBufferRTHandles[i] = rtvHandle;
        rtvHandle.ptr += rtvDescriptorSize;
    }

    CreateRenderTargets();
}

void D3D12App::CreateRenderTargets()
{
    m_backBufferTargets.resize(m_backBufferCount);
    for (uint8_t i = 0u; i < m_backBufferCount; ++i)
    {
        m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBufferTargets[i]));
        m_device->CreateRenderTargetView(m_backBufferTargets[i].Get(), nullptr, m_backBufferRTHandles[i]);
    }
}

void D3D12App::InitialiseImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.DisplaySize = ImVec2(static_cast<float>(m_width), static_cast<float>(m_height));
    io.IniFilename = nullptr; // Dont save out any .inis
    io.LogFilename = nullptr;

    // Make descriptor heap for ImGui
    D3D12_DESCRIPTOR_HEAP_DESC desc;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; 
    desc.NumDescriptors = 1u; // ImGui needs at least one SRV for font atlas
    desc.NodeMask = 0u;
    D3D_VERIFY(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_imGuiDescriptorHeap)));

    ImGui_ImplWin32_Init(Win32App::GetHandle());
    ImGui_ImplDX12_Init(
        m_device.Get(),
        m_backBufferCount,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        m_imGuiDescriptorHeap.Get(),
        m_imGuiDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        m_imGuiDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

    ImGui::StyleColorsDark();
}
