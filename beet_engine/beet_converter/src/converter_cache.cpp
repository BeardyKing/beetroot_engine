#include <beet_converter/converter_interface.h>
#include <beet_converter/converter_types.h>
#include <beet_shared/log.h>

#include <sys/stat.h>

extern ConverterOptions g_converterOptions;

bool converter_cache_check_needs_convert(const char *toPath, const char *fromPath) {
    if (g_converterOptions.ignoreConvertCache) {
        return true;
    }
    struct stat toFileStat{};
    struct stat fromFileStat{};

    stat(toPath, &toFileStat);
    stat(fromPath, &fromFileStat);

    if (toFileStat.st_mtime > fromFileStat.st_mtime) {
        log_verbose(MSG_CONVERTER, "cached skipping convert: %s \n", fromPath)
        return false;
    }
    return true;
}