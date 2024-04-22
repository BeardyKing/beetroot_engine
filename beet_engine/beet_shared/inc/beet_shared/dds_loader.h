#ifndef BEETROOT_DDS_LOADER_H
#define BEETROOT_DDS_LOADER_H

#include <cstdint>
#include <beet_shared//texture_formats.h>
//INFO: .dds file format spec
// https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-header
// https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-pixelformat
// https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
// https://learn.microsoft.com/en-us/windows/uwp/gaming/complete-code-for-ddstextureloader

//===API================================================================================================================
void load_dds_image_alloc(const char *path, RawImage *outRawImage);
//======================================================================================================================

#endif //BEETROOT_DDS_LOADER_H
