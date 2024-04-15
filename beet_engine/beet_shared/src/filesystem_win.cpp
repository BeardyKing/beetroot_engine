#include <beet_shared/platform_defines.h>

#if PLATFORM_WINDOWS

#include <beet_shared/filesystem.h>
#include <beet_shared/c_string.h>

#include <cstring>

#include <sys/stat.h>
#include <direct.h>

bool fs_mkdir(const char *path) {
    return (_mkdir(path) == 0);
}

bool fs_rmdir(const char *path) {
    return (_rmdir(path) == 0);
}

bool fs_file_exists(const char *path) {
    struct stat buffer{};
    return (stat(path, &buffer) == 0);
}

size_t fs_file_size(const char* path){
    struct stat buffer{};
    stat(path, &buffer);
    return buffer.st_size;
}

bool fs_mkdir_recursive_internal(const char path[FS_MAX_PATH_SIZE]) {
    constexpr size_t cpySize = sizeof(char) * FS_MAX_PATH_SIZE;
    char currPath[FS_MAX_PATH_SIZE] = {};
    memcpy(currPath, path, cpySize);

    char *delimLocation = strrchr(currPath, '/');
    if (!fs_file_exists(currPath) && delimLocation != nullptr) {
        *delimLocation = '\0';
        fs_mkdir_recursive_internal(currPath);
    } else {
        return false;
    }

    return fs_mkdir(currPath);
}

//Note: This function can either take in a file path or a directory path
//      When using a directory path you MUST include a trailing '/'
bool fs_mkdir_recursive(const char path[FS_MAX_PATH_SIZE]) {
    if (c_str_empty(path)) {
        return false;
    }
    return fs_mkdir_recursive_internal(path);
}

#endif

