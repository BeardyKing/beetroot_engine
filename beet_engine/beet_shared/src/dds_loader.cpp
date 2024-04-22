#include <beet_shared/dds_loader.h>
#include <beet_shared/log.h>
#include <beet_shared/assert.h>
#include <beet_shared/texture_formats.h>
#include <beet_shared/memory.h>

#include <iostream>
#include <fstream>
#include <cstring>

//===INTERNAL_STRUCTS===================================================================================================
struct PixelFormatDDS {
    uint32_t dwSize;                // dwSize:          Structure size
    uint32_t dwFlags;               // dwFlags:         Values which indicate what type of data is in the surface.
    uint32_t dwFourCC;              // dwFourCC:        Four-character codes for specifying compressed or custom formats. i.e. {'D','X','T','2'}{'B','C','6','H'}
    uint32_t dwRGBBitCount;         // dwRGBBitCount:   Number of bits in an RGB.
    uint32_t dwRBitMask;            // dwRBitMask:      Red (or luminance or Y) mask for reading color data.
    uint32_t dwGBitMask;            // dwGBitMask:      Green (or U) mask for reading color data.
    uint32_t dwBBitMask;            // dwBBitMask:      Blue (or V) mask for reading color data.
    uint32_t dwABitMask;            // dwABitMask:      Alpha mask for reading alpha data.
};

struct HeaderDDS {
    uint32_t dwSize;                // dwSize:              Size of structure
    uint32_t dwFlags;               // dwFlags:             Flags to indicate which members contain valid data.
    uint32_t dwHeight;              // dwHeight:            Surface height (in pixels).
    uint32_t dwWidth;               // dwWidth:             Surface width (in pixels).
    uint32_t dwPitchOrLinearSize;   // dwPitchOrLinearSize: The pitch or number of bytes per scan line in an uncompressed texture
    uint32_t dwDepth;               // dwDepth:             Depth of a volume texture (in pixels), otherwise unused.
    uint32_t dwMipMapCount;         // dwMipMapCount:       Number of mipmap levels, otherwise unused.
    uint32_t dwReserved1[11];       // dwReserved1:         Unused.
    PixelFormatDDS ddspf;           // ddspf:               The pixel format see BEET_DXGI_FORMAT
    uint32_t dwCaps;                // dwCaps:              Specifies the complexity of the surfaces stored. i.e. is cube map and or volume texture
    uint32_t dwCaps2;               // dwCaps2:             Additional detail about the surfaces stored. i.e. cube map and or volume info
    uint32_t dwCaps3;               // dwCaps3:             Unused.
    uint32_t dwCaps4;               // dwCaps4:             Unused.
    uint32_t dwReserved2;           // dwReserved2:         Unused.
};

struct HeaderDDSDXT10 {
    uint32_t dxgiFormat;            // The surface pixel format
    uint32_t resourceDimension;     // Identifies the type of resource i.e. 1/2/3D texture
    uint32_t miscFlag;              // Identifies other, less common options for resources.
    uint32_t arraySize;             // The number of elements in the array.
    uint32_t miscFlags2;            // Contains additional metadata. The lower 3 bits indicate the alpha mode
};

enum class TextureFormatDXGI : uint32_t {
    DXGI_FORMAT_UNKNOWN = 0,
    DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
    DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
    DXGI_FORMAT_R32G32B32A32_UINT = 3,
    DXGI_FORMAT_R32G32B32A32_SINT = 4,

    DXGI_FORMAT_R32G32B32_TYPELESS = 5,
    DXGI_FORMAT_R32G32B32_FLOAT = 6,
    DXGI_FORMAT_R32G32B32_UINT = 7,
    DXGI_FORMAT_R32G32B32_SINT = 8,
    DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
    DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
    DXGI_FORMAT_R16G16B16A16_UNORM = 11,
    DXGI_FORMAT_R16G16B16A16_UINT = 12,
    DXGI_FORMAT_R16G16B16A16_SNORM = 13,
    DXGI_FORMAT_R16G16B16A16_SINT = 14,
    DXGI_FORMAT_R32G32_TYPELESS = 15,
    DXGI_FORMAT_R32G32_FLOAT = 16,
    DXGI_FORMAT_R32G32_UINT = 17,
    DXGI_FORMAT_R32G32_SINT = 18,
    DXGI_FORMAT_R32G8X24_TYPELESS = 19,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
    DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
    DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
    DXGI_FORMAT_R10G10B10A2_UNORM = 24,
    DXGI_FORMAT_R10G10B10A2_UINT = 25,
    DXGI_FORMAT_R11G11B10_FLOAT = 26,
    DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
    DXGI_FORMAT_R8G8B8A8_UNORM = 28,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
    DXGI_FORMAT_R8G8B8A8_UINT = 30,
    DXGI_FORMAT_R8G8B8A8_SNORM = 31,
    DXGI_FORMAT_R8G8B8A8_SINT = 32,
    DXGI_FORMAT_R16G16_TYPELESS = 33,
    DXGI_FORMAT_R16G16_FLOAT = 34,
    DXGI_FORMAT_R16G16_UNORM = 35,
    DXGI_FORMAT_R16G16_UINT = 36,
    DXGI_FORMAT_R16G16_SNORM = 37,
    DXGI_FORMAT_R16G16_SINT = 38,
    DXGI_FORMAT_R32_TYPELESS = 39,
    DXGI_FORMAT_D32_FLOAT = 40,
    DXGI_FORMAT_R32_FLOAT = 41,
    DXGI_FORMAT_R32_UINT = 42,
    DXGI_FORMAT_R32_SINT = 43,
    DXGI_FORMAT_R24G8_TYPELESS = 44,
    DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
    DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
    DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
    DXGI_FORMAT_R8G8_TYPELESS = 48,
    DXGI_FORMAT_R8G8_UNORM = 49,
    DXGI_FORMAT_R8G8_UINT = 50,
    DXGI_FORMAT_R8G8_SNORM = 51,
    DXGI_FORMAT_R8G8_SINT = 52,
    DXGI_FORMAT_R16_TYPELESS = 53,
    DXGI_FORMAT_R16_FLOAT = 54,
    DXGI_FORMAT_D16_UNORM = 55,
    DXGI_FORMAT_R16_UNORM = 56,
    DXGI_FORMAT_R16_UINT = 57,
    DXGI_FORMAT_R16_SNORM = 58,
    DXGI_FORMAT_R16_SINT = 59,
    DXGI_FORMAT_R8_TYPELESS = 60,
    DXGI_FORMAT_R8_UNORM = 61,
    DXGI_FORMAT_R8_UINT = 62,
    DXGI_FORMAT_R8_SNORM = 63,
    DXGI_FORMAT_R8_SINT = 64,
    DXGI_FORMAT_A8_UNORM = 65,
    DXGI_FORMAT_R1_UNORM = 66,
    DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
    DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
    DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
    DXGI_FORMAT_BC1_TYPELESS = 70,
    DXGI_FORMAT_BC1_UNORM = 71,
    DXGI_FORMAT_BC1_UNORM_SRGB = 72,
    DXGI_FORMAT_BC2_TYPELESS = 73,
    DXGI_FORMAT_BC2_UNORM = 74,
    DXGI_FORMAT_BC2_UNORM_SRGB = 75,
    DXGI_FORMAT_BC3_TYPELESS = 76,
    DXGI_FORMAT_BC3_UNORM = 77,
    DXGI_FORMAT_BC3_UNORM_SRGB = 78,
    DXGI_FORMAT_BC4_TYPELESS = 79,
    DXGI_FORMAT_BC4_UNORM = 80,
    DXGI_FORMAT_BC4_SNORM = 81,
    DXGI_FORMAT_BC5_TYPELESS = 82,
    DXGI_FORMAT_BC5_UNORM = 83,
    DXGI_FORMAT_BC5_SNORM = 84,
    DXGI_FORMAT_B5G6R5_UNORM = 85,
    DXGI_FORMAT_B5G5R5A1_UNORM = 86,
    DXGI_FORMAT_B8G8R8A8_UNORM = 87,
    DXGI_FORMAT_B8G8R8X8_UNORM = 88,
    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
    DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
    DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
    DXGI_FORMAT_BC6H_TYPELESS = 94,
    DXGI_FORMAT_BC6H_UF16 = 95,
    DXGI_FORMAT_BC6H_SF16 = 96,
    DXGI_FORMAT_BC7_TYPELESS = 97,
    DXGI_FORMAT_BC7_UNORM = 98,
    DXGI_FORMAT_BC7_UNORM_SRGB = 99,
    DXGI_FORMAT_AYUV = 100,
    DXGI_FORMAT_Y410 = 101,
    DXGI_FORMAT_Y416 = 102,
    DXGI_FORMAT_NV12 = 103,
    DXGI_FORMAT_P010 = 104,
    DXGI_FORMAT_P016 = 105,
    DXGI_FORMAT_420_OPAQUE = 106,
    DXGI_FORMAT_YUY2 = 107,
    DXGI_FORMAT_Y210 = 108,
    DXGI_FORMAT_Y216 = 109,
    DXGI_FORMAT_NV11 = 110,
    DXGI_FORMAT_AI44 = 111,
    DXGI_FORMAT_IA44 = 112,
    DXGI_FORMAT_P8 = 113,
    DXGI_FORMAT_A8P8 = 114,
    DXGI_FORMAT_B4G4R4A4_UNORM = 115,
    DXGI_FORMAT_P208 = 130,
    DXGI_FORMAT_V208 = 131,
    DXGI_FORMAT_V408 = 132,
    DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE,
    DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE,
    DXGI_FORMAT_FORCE_UINT = 0xffffffff
};
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static constexpr uint32_t constexpr_make_four_cc(const char index0, const char index1, const char index2, const char index3) {
    return ((uint32_t) (uint8_t) (index0) |
            ((uint32_t) (uint8_t) (index1) << 8) |
            ((uint32_t) (uint8_t) (index2) << 16) |
            ((uint32_t) (uint8_t) (index3) << 24));
}

static TextureFormat internal_dxgi_to_beet_texture_format(const TextureFormatDXGI textureFormat) {
    switch (textureFormat) {
        case TextureFormatDXGI::DXGI_FORMAT_R8G8B8A8_UNORM:
            return TextureFormat::RGBA8;
        case TextureFormatDXGI::DXGI_FORMAT_R16G16B16A16_UNORM:
            return TextureFormat::RGBA16;

        case TextureFormatDXGI::DXGI_FORMAT_BC1_UNORM:
            return TextureFormat::BC1RGBA;
        case TextureFormatDXGI::DXGI_FORMAT_BC2_UNORM:
            return TextureFormat::BC2;
        case TextureFormatDXGI::DXGI_FORMAT_BC3_UNORM:
            return TextureFormat::BC3;
        case TextureFormatDXGI::DXGI_FORMAT_BC4_UNORM:
            return TextureFormat::BC4;
        case TextureFormatDXGI::DXGI_FORMAT_BC5_UNORM:
            return TextureFormat::BC5;
        case TextureFormatDXGI::DXGI_FORMAT_BC6H_UF16:
            return TextureFormat::BC6H;
        case TextureFormatDXGI::DXGI_FORMAT_BC7_UNORM:
            return TextureFormat::BC7;
        default: SANITY_CHECK();
    };
    return TextureFormat::INVALID_FORMAT;
}


static uint32_t bits_per_pixel(const TextureFormatDXGI textureFormat) {
    switch (textureFormat) {
        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32A32_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32A32_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32A32_SINT:
            return 128;

        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32B32_SINT:
            return 96;

        case TextureFormatDXGI::DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16B16A16_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16B16A16_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16B16A16_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16B16A16_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16B16A16_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G32_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32G8X24_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return 64;

        case TextureFormatDXGI::DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R10G10B10A2_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R10G10B10A2_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R11G11B10_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8B8A8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8B8A8_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8B8A8_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8B8A8_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R16G16_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_D32_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R32_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_R32_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R32_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_R24G8_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_D24_UNORM_S8_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8_B8G8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_G8R8_G8B8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_B8G8R8A8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_B8G8R8X8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return 32;

        case TextureFormatDXGI::DXGI_FORMAT_R8G8_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R8G8_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_R16_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R16_FLOAT:
        case TextureFormatDXGI::DXGI_FORMAT_D16_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R16_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R16_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R16_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R16_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_B5G6R5_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_B5G5R5A1_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_B4G4R4A4_UNORM:
            return 16;

        case TextureFormatDXGI::DXGI_FORMAT_R8_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_R8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R8_UINT:
        case TextureFormatDXGI::DXGI_FORMAT_R8_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_R8_SINT:
        case TextureFormatDXGI::DXGI_FORMAT_A8_UNORM:
            return 8;

        case TextureFormatDXGI::DXGI_FORMAT_R1_UNORM:
            return 1;

        case TextureFormatDXGI::DXGI_FORMAT_BC1_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC1_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC1_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_BC4_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC4_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC4_SNORM:
            return 4;

        case TextureFormatDXGI::DXGI_FORMAT_BC2_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC2_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC2_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_BC3_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC3_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC3_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_BC5_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC5_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC5_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC6H_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC6H_UF16:
        case TextureFormatDXGI::DXGI_FORMAT_BC6H_SF16:
        case TextureFormatDXGI::DXGI_FORMAT_BC7_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC7_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC7_UNORM_SRGB:
            return 8;

        default:
            return 0;
    }
}

static void get_image_info(
        const uint32_t width,
        const uint32_t height,
        const TextureFormatDXGI textureFormat,
        uint32_t *outNumBytes,
        uint32_t *outRowBytes,
        uint32_t *outNumRows) {
    uint32_t numBytes = 0;
    uint32_t rowBytes = 0;
    uint32_t numRows = 0;

    bool bc = false;
    bool packed = false;
    uint32_t bcnumBytesPerBlock = 0;
    switch (textureFormat) {
        case TextureFormatDXGI::DXGI_FORMAT_BC1_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC1_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC1_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_BC4_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC4_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC4_SNORM:
            bc = true;
            bcnumBytesPerBlock = 8;
            break;

        case TextureFormatDXGI::DXGI_FORMAT_BC2_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC2_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC2_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_BC3_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC3_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC3_UNORM_SRGB:
        case TextureFormatDXGI::DXGI_FORMAT_BC5_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC5_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC5_SNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC6H_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC6H_UF16:
        case TextureFormatDXGI::DXGI_FORMAT_BC6H_SF16:
        case TextureFormatDXGI::DXGI_FORMAT_BC7_TYPELESS:
        case TextureFormatDXGI::DXGI_FORMAT_BC7_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_BC7_UNORM_SRGB:
            bc = true;
            bcnumBytesPerBlock = 16;
            break;

        case TextureFormatDXGI::DXGI_FORMAT_R8G8_B8G8_UNORM:
        case TextureFormatDXGI::DXGI_FORMAT_G8R8_G8B8_UNORM:
            packed = true;
            break;
        default:
            break; // not bc or packed format
    }

    if (bc) {
        size_t numBlocksWide = 0;
        if (width > 0) {
            numBlocksWide = std::max<size_t>(1, (width + 3) / 4);
        }
        size_t numBlocksHigh = 0;
        if (height > 0) {
            numBlocksHigh = std::max<size_t>(1, (height + 3) / 4);
        }
        rowBytes = numBlocksWide * bcnumBytesPerBlock;
        numRows = numBlocksHigh;
    } else if (packed) {
        rowBytes = ((width + 1) >> 1) * 4;
        numRows = height;
    } else {
        size_t bpp = bits_per_pixel(textureFormat);
        rowBytes = (width * bpp + 7) / 8; // Round up to the nearest byte.
        numRows = height;
    }

    numBytes = rowBytes * numRows;
    if (outNumBytes) {
        *outNumBytes = numBytes;
    }
    if (outRowBytes) {
        *outRowBytes = rowBytes;
    }
    if (outNumRows) {
        *outNumRows = numRows;
    }
}
//======================================================================================================================

//===API================================================================================================================
void load_dds_image_alloc(const char *path, RawImage *outRawImage) {
    log_verbose(MSG_DDS, "loading dds image : %s \n", path);

    std::ifstream file{path, std::ios::ate | std::ios::binary};
    ASSERT_MSG(file.is_open(), "Err: failed to find path: %s \n", path);
    const size_t fileSize = file.tellg();

    char *rawFileData = (char *) mem_zalloc(fileSize);
    char ext[4] = {};
    file.seekg(0);
    file.read(ext, sizeof(uint32_t));
    file.read(rawFileData, fileSize - sizeof(uint32_t));
    const char expectedHeaderFmt[4] = {'D', 'D', 'S', ' '};
    ASSERT_MSG(memcmp(ext, expectedHeaderFmt, sizeof(char) * 4) == 0,
               "Err: input header did not match [D][D][S][] received [%c][%c][%c][%c] for path : %s \n", ext[0], ext[1], ext[2], ext[3], path);
    file.close();

    const HeaderDDS *header = reinterpret_cast<const HeaderDDS *> (rawFileData);

    const uint32_t width = header->dwWidth;
    const uint32_t height = header->dwHeight;
    const uint32_t depth = header->dwDepth;
    uint32_t mipCount = header->dwMipMapCount;
    TextureFormatDXGI format{};

    ASSERT_MSG(mipCount < BEET_MAX_MIP_COUNT, "Err: mip count exceeded max supported mip count of %u", BEET_MAX_MIP_COUNT)
    if (mipCount == 0) {
        mipCount = 1;
    }

    const HeaderDDSDXT10 *d3d10ext = reinterpret_cast<const HeaderDDSDXT10 *>((const char *) header + sizeof(HeaderDDS));

    uint32_t outNumBytes{};
    uint32_t outRowBytes{};
    uint32_t outNumRows{};
    size_t sumOfMipData = 0;
    for (int i = 0; i < mipCount; ++i) {
        if (constexpr_make_four_cc('D', 'X', '1', '0') == header->ddspf.dwFourCC) {
            format = (TextureFormatDXGI) d3d10ext->dxgiFormat;
            get_image_info(width >> i, height >> i, format, &outNumBytes, &outRowBytes, &outNumRows);
            outRawImage->mipDataSizes[i] = outNumBytes;
            sumOfMipData += outNumBytes;
        }
    }

    const void *imageStartPos = (unsigned char *) header + sizeof(HeaderDDS) + sizeof(HeaderDDSDXT10);
    outRawImage->textureFormat = internal_dxgi_to_beet_texture_format(format);
    outRawImage->mipMapCount = mipCount;
    outRawImage->width = width;
    outRawImage->height = height;
    outRawImage->depth = depth;
    outRawImage->dataSize = sumOfMipData;

    outRawImage->data = (unsigned char *) mem_zalloc(outRawImage->dataSize);
    memcpy(outRawImage->data, imageStartPos, outRawImage->dataSize);

    mem_free(rawFileData);
}
//======================================================================================================================