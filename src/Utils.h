#pragma once

#include <sstream>
#include <comdef.h>
#include <codecvt> // UTF-16 to UTF-8

#define D3D_VERIFY(x) ThrowIfFailed(x)

inline void ThrowIfFailed(const HRESULT hr)
{
    if (FAILED(hr))
    {
        const _com_error err(hr);
        std::wstringstream wss;
        wss << err.ErrorMessage() << L"\nHRESULT code: 0x" << std::hex << std::uppercase << hr << '\n';
        MessageBoxW(NULL, wss.str().c_str(), L"Error", MB_OK | MB_ICONERROR);

        PostQuitMessage(hr);
    }
}

constexpr float BytesToGigbibytes(const uint32_t bytes)
{
    return static_cast<float>(bytes) / 1.074e+9f;
}

constexpr double BytesToGigbibytes(const uint64_t bytes)
{
    return static_cast<double>(bytes) / 1.074e+9f;
}

inline std::string UTF16toUTF8(const std::wstring& wideString)
{
    // Only suitable for windows platform, see below.
    //https://stackoverflow.com/questions/42734715/how-can-i-convert-wstring-to-u16string
    const std::u16string u16str(wideString.begin(), wideString.end()); 
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    return convert.to_bytes(u16str);
}

inline const char* D3DFeatureLevelToString(const D3D_FEATURE_LEVEL FL)
{
    switch (FL)
    {
    case D3D_FEATURE_LEVEL_1_0_CORE: return "1_0_CORE";
    case D3D_FEATURE_LEVEL_9_1:      return "9_1";
    case D3D_FEATURE_LEVEL_9_2:      return "9_2";
    case D3D_FEATURE_LEVEL_9_3:      return "9_3";
    case D3D_FEATURE_LEVEL_10_0:     return "10_0";
    case D3D_FEATURE_LEVEL_10_1:     return "10_1";
    case D3D_FEATURE_LEVEL_11_0:     return "11_0";
    case D3D_FEATURE_LEVEL_11_1:     return "11_1";
    case D3D_FEATURE_LEVEL_12_0:     return "12_0";
    case D3D_FEATURE_LEVEL_12_1:     return "12_1";
    case D3D_FEATURE_LEVEL_12_2:     return "12_2";
    default: return "UNKNOWN FEATURE LEVEL";
    }
}

inline const char* D3DFormatToString(const DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_UNKNOWN:                              return "DXGI_FORMAT_UNKNOWN";
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:                return "DXGI_FORMAT_R32G32B32A32_TYPELESS";
    case DXGI_FORMAT_R32G32B32A32_FLOAT:                   return "DXGI_FORMAT_R32G32B32A32_FLOAT";
    case DXGI_FORMAT_R32G32B32A32_UINT:                    return "DXGI_FORMAT_R32G32B32A32_UINT";
    case DXGI_FORMAT_R32G32B32A32_SINT:                    return "DXGI_FORMAT_R32G32B32A32_SINT";
    case DXGI_FORMAT_R32G32B32_TYPELESS:                   return "DXGI_FORMAT_R32G32B32_TYPELESS";
    case DXGI_FORMAT_R32G32B32_FLOAT:                      return "DXGI_FORMAT_R32G32B32_FLOAT";
    case DXGI_FORMAT_R32G32B32_UINT:                       return "DXGI_FORMAT_R32G32B32_UINT";
    case DXGI_FORMAT_R32G32B32_SINT:                       return "DXGI_FORMAT_R32G32B32_SINT";
    case DXGI_FORMAT_R16G16B16A16_TYPELESS:                return "DXGI_FORMAT_R16G16B16A16_TYPELESS";
    case DXGI_FORMAT_R16G16B16A16_FLOAT:                   return "DXGI_FORMAT_R16G16B16A16_FLOAT";
    case DXGI_FORMAT_R16G16B16A16_UNORM:                   return "DXGI_FORMAT_R16G16B16A16_UNORM";
    case DXGI_FORMAT_R16G16B16A16_UINT:                    return "DXGI_FORMAT_R16G16B16A16_UINT";
    case DXGI_FORMAT_R16G16B16A16_SNORM:                   return "DXGI_FORMAT_R16G16B16A16_SNORM";
    case DXGI_FORMAT_R16G16B16A16_SINT:                    return "DXGI_FORMAT_R16G16B16A16_SINT";
    case DXGI_FORMAT_R32G32_TYPELESS:                      return "DXGI_FORMAT_R32G32_TYPELESS";
    case DXGI_FORMAT_R32G32_FLOAT:                         return "DXGI_FORMAT_R32G32_FLOAT";
    case DXGI_FORMAT_R32G32_UINT:                          return "DXGI_FORMAT_R32G32_UINT";
    case DXGI_FORMAT_R32G32_SINT:                          return "DXGI_FORMAT_R32G32_SINT";
    case DXGI_FORMAT_R32G8X24_TYPELESS:                    return "DXGI_FORMAT_R32G8X24_TYPELESS";
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:                 return "DXGI_FORMAT_D32_FLOAT_S8X24_UINT";
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:             return "DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS";
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:              return "DXGI_FORMAT_X32_TYPELESS_G8X24_UINT";
    case DXGI_FORMAT_R10G10B10A2_TYPELESS:                 return "DXGI_FORMAT_R10G10B10A2_TYPELESS";
    case DXGI_FORMAT_R10G10B10A2_UNORM:                    return "DXGI_FORMAT_R10G10B10A2_UNORM";
    case DXGI_FORMAT_R10G10B10A2_UINT:                     return "DXGI_FORMAT_R10G10B10A2_UINT";
    case DXGI_FORMAT_R11G11B10_FLOAT:                      return "DXGI_FORMAT_R11G11B10_FLOAT";
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:                    return "DXGI_FORMAT_R8G8B8A8_TYPELESS";
    case DXGI_FORMAT_R8G8B8A8_UNORM:                       return "DXGI_FORMAT_R8G8B8A8_UNORM";
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:                  return "DXGI_FORMAT_R8G8B8A8_UNORM_SRGB";
    case DXGI_FORMAT_R8G8B8A8_UINT:                        return "DXGI_FORMAT_R8G8B8A8_UINT";
    case DXGI_FORMAT_R8G8B8A8_SNORM:                       return "DXGI_FORMAT_R8G8B8A8_SNORM";
    case DXGI_FORMAT_R8G8B8A8_SINT:                        return "DXGI_FORMAT_R8G8B8A8_SINT";
    case DXGI_FORMAT_R16G16_TYPELESS:                      return "DXGI_FORMAT_R16G16_TYPELESS";
    case DXGI_FORMAT_R16G16_FLOAT:                         return "DXGI_FORMAT_R16G16_FLOAT";
    case DXGI_FORMAT_R16G16_UNORM:                         return "DXGI_FORMAT_R16G16_UNORM";
    case DXGI_FORMAT_R16G16_UINT:                          return "DXGI_FORMAT_R16G16_UINT";
    case DXGI_FORMAT_R16G16_SNORM:                         return "DXGI_FORMAT_R16G16_SNORM";
    case DXGI_FORMAT_R16G16_SINT:                          return "DXGI_FORMAT_R16G16_SINT";
    case DXGI_FORMAT_R32_TYPELESS:                         return "DXGI_FORMAT_R32_TYPELESS";
    case DXGI_FORMAT_D32_FLOAT:                            return "DXGI_FORMAT_D32_FLOAT";
    case DXGI_FORMAT_R32_FLOAT:                            return "DXGI_FORMAT_R32_FLOAT";
    case DXGI_FORMAT_R32_UINT:                             return "DXGI_FORMAT_R32_UINT";
    case DXGI_FORMAT_R32_SINT:                             return "DXGI_FORMAT_R32_SINT";
    case DXGI_FORMAT_R24G8_TYPELESS:                       return "DXGI_FORMAT_R24G8_TYPELESS";
    case DXGI_FORMAT_D24_UNORM_S8_UINT:                    return "DXGI_FORMAT_D24_UNORM_S8_UINT";
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:                return "DXGI_FORMAT_R24_UNORM_X8_TYPELESS";
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:                 return "DXGI_FORMAT_X24_TYPELESS_G8_UINT";
    case DXGI_FORMAT_R8G8_TYPELESS:                        return "DXGI_FORMAT_R8G8_TYPELESS";
    case DXGI_FORMAT_R8G8_UNORM:                           return "DXGI_FORMAT_R8G8_UNORM";
    case DXGI_FORMAT_R8G8_UINT:                            return "DXGI_FORMAT_R8G8_UINT";
    case DXGI_FORMAT_R8G8_SNORM:                           return "DXGI_FORMAT_R8G8_SNORM";
    case DXGI_FORMAT_R8G8_SINT:                            return "DXGI_FORMAT_R8G8_SINT";
    case DXGI_FORMAT_R16_TYPELESS:                         return "DXGI_FORMAT_R16_TYPELESS";
    case DXGI_FORMAT_R16_FLOAT:                            return "DXGI_FORMAT_R16_FLOAT";
    case DXGI_FORMAT_D16_UNORM:                            return "DXGI_FORMAT_D16_UNORM";
    case DXGI_FORMAT_R16_UNORM:                            return "DXGI_FORMAT_R16_UNORM";
    case DXGI_FORMAT_R16_UINT:                             return "DXGI_FORMAT_R16_UINT";
    case DXGI_FORMAT_R16_SNORM:                            return "DXGI_FORMAT_R16_SNORM";
    case DXGI_FORMAT_R16_SINT:                             return "DXGI_FORMAT_R16_SINT";
    case DXGI_FORMAT_R8_TYPELESS:                          return "DXGI_FORMAT_R8_TYPELESS";
    case DXGI_FORMAT_R8_UNORM:                             return "DXGI_FORMAT_R8_UNORM";
    case DXGI_FORMAT_R8_UINT:                              return "DXGI_FORMAT_R8_UINT";
    case DXGI_FORMAT_R8_SNORM:                             return "DXGI_FORMAT_R8_SNORM";
    case DXGI_FORMAT_R8_SINT:                              return "DXGI_FORMAT_R8_SINT";
    case DXGI_FORMAT_A8_UNORM:                             return "DXGI_FORMAT_A8_UNORM";
    case DXGI_FORMAT_R1_UNORM:                             return "DXGI_FORMAT_R1_UNORM";
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:                   return "DXGI_FORMAT_R9G9B9E5_SHAREDEXP";
    case DXGI_FORMAT_R8G8_B8G8_UNORM:                      return "DXGI_FORMAT_R8G8_B8G8_UNORM";
    case DXGI_FORMAT_G8R8_G8B8_UNORM:                      return "DXGI_FORMAT_G8R8_G8B8_UNORM";
    case DXGI_FORMAT_BC1_TYPELESS:                         return "DXGI_FORMAT_BC1_TYPELESS";
    case DXGI_FORMAT_BC1_UNORM:                            return "DXGI_FORMAT_BC1_UNORM";
    case DXGI_FORMAT_BC1_UNORM_SRGB:                       return "DXGI_FORMAT_BC1_UNORM_SRGB";
    case DXGI_FORMAT_BC2_TYPELESS:                         return "DXGI_FORMAT_BC2_TYPELESS";
    case DXGI_FORMAT_BC2_UNORM:                            return "DXGI_FORMAT_BC2_UNORM";
    case DXGI_FORMAT_BC2_UNORM_SRGB:                       return "DXGI_FORMAT_BC2_UNORM_SRGB";
    case DXGI_FORMAT_BC3_TYPELESS:                         return "DXGI_FORMAT_BC3_TYPELESS";
    case DXGI_FORMAT_BC3_UNORM:                            return "DXGI_FORMAT_BC3_UNORM";
    case DXGI_FORMAT_BC3_UNORM_SRGB:                       return "DXGI_FORMAT_BC3_UNORM_SRGB";
    case DXGI_FORMAT_BC4_TYPELESS:                         return "DXGI_FORMAT_BC4_TYPELESS";
    case DXGI_FORMAT_BC4_UNORM:                            return "DXGI_FORMAT_BC4_UNORM";
    case DXGI_FORMAT_BC4_SNORM:                            return "DXGI_FORMAT_BC4_SNORM";
    case DXGI_FORMAT_BC5_TYPELESS:                         return "DXGI_FORMAT_BC5_TYPELESS";
    case DXGI_FORMAT_BC5_UNORM:                            return "DXGI_FORMAT_BC5_UNORM";
    case DXGI_FORMAT_BC5_SNORM:                            return "DXGI_FORMAT_BC5_SNORM";
    case DXGI_FORMAT_B5G6R5_UNORM:                         return "DXGI_FORMAT_B5G6R5_UNORM";
    case DXGI_FORMAT_B5G5R5A1_UNORM:                       return "DXGI_FORMAT_B5G5R5A1_UNORM";
    case DXGI_FORMAT_B8G8R8A8_UNORM:                       return "DXGI_FORMAT_B8G8R8A8_UNORM";
    case DXGI_FORMAT_B8G8R8X8_UNORM:                       return "DXGI_FORMAT_B8G8R8X8_UNORM";
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:           return "DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM";
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:                    return "DXGI_FORMAT_B8G8R8A8_TYPELESS";
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:                  return "DXGI_FORMAT_B8G8R8A8_UNORM_SRGB";
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:                    return "DXGI_FORMAT_B8G8R8X8_TYPELESS";
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:                  return "DXGI_FORMAT_B8G8R8X8_UNORM_SRGB";
    case DXGI_FORMAT_BC6H_TYPELESS:                        return "DXGI_FORMAT_BC6H_TYPELESS";
    case DXGI_FORMAT_BC6H_UF16:                            return "DXGI_FORMAT_BC6H_UF16";
    case DXGI_FORMAT_BC6H_SF16:                            return "DXGI_FORMAT_BC6H_SF16";
    case DXGI_FORMAT_BC7_TYPELESS:                         return "DXGI_FORMAT_BC7_TYPELESS";
    case DXGI_FORMAT_BC7_UNORM:                            return "DXGI_FORMAT_BC7_UNORM";
    case DXGI_FORMAT_BC7_UNORM_SRGB:                       return "DXGI_FORMAT_BC7_UNORM_SRGB";
    case DXGI_FORMAT_AYUV:                                 return "DXGI_FORMAT_AYUV";
    case DXGI_FORMAT_Y410:                                 return "DXGI_FORMAT_Y410";
    case DXGI_FORMAT_Y416:                                 return "DXGI_FORMAT_Y416";
    case DXGI_FORMAT_NV12:                                 return "DXGI_FORMAT_NV12";
    case DXGI_FORMAT_P010:                                 return "DXGI_FORMAT_P010";
    case DXGI_FORMAT_P016:                                 return "DXGI_FORMAT_P016";
    case DXGI_FORMAT_420_OPAQUE:                           return "DXGI_FORMAT_420_OPAQUE";
    case DXGI_FORMAT_YUY2:                                 return "DXGI_FORMAT_YUY2";
    case DXGI_FORMAT_Y210:                                 return "DXGI_FORMAT_Y210";
    case DXGI_FORMAT_Y216:                                 return "DXGI_FORMAT_Y216";
    case DXGI_FORMAT_NV11:                                 return "DXGI_FORMAT_NV11";
    case DXGI_FORMAT_AI44:                                 return "DXGI_FORMAT_AI44";
    case DXGI_FORMAT_IA44:                                 return "DXGI_FORMAT_IA44";
    case DXGI_FORMAT_P8:                                   return "DXGI_FORMAT_P8";
    case DXGI_FORMAT_A8P8:                                 return "DXGI_FORMAT_A8P8";
    case DXGI_FORMAT_B4G4R4A4_UNORM:                       return "DXGI_FORMAT_B4G4R4A4_UNORM";
    case DXGI_FORMAT_P208:                                 return "DXGI_FORMAT_P208";
    case DXGI_FORMAT_V208:                                 return "DXGI_FORMAT_V208";
    case DXGI_FORMAT_V408:                                 return "DXGI_FORMAT_V408";

    default:
        return "Unknown format";
    }
}