#ifndef BEETROOT_FILESYSTEM_H
#define BEETROOT_FILESYSTEM_H

#include <cstdint>

constexpr uint32_t FS_MAX_PATH_SIZE = 256;

bool fs_mkdir(const char *path);
bool fs_mkdir_recursive(const char *path);

bool fs_rmdir(const char *path);

bool fs_file_exists(const char *path);

#endif //BEETROOT_FILESYSTEM_H
