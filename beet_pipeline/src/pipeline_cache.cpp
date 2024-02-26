#include <beet_pipeline/pipeline_cache.h>
#include <beet_pipeline/pipeline_commandlines.h>

#include <beet_shared/log.h>

#include <sys/stat.h>

bool pipeline_cache_should_convert(const std::string &toPath, const std::string &fromPath) {
    if (commandline_get_arg(CLArgs::ignoreConvertCache).enabled) {
        return true;
    }

    struct stat toFileStat{};
    struct stat fromFileStat{};

    stat(toPath.c_str(), &toFileStat);
    stat(fromPath.c_str(), &fromFileStat);

    if (toFileStat.st_mtime > fromFileStat.st_mtime) {
        log_info(MSG_PIPELINE, "cached skipping convert: %s \n", fromPath.c_str())
        return false;
    }
    return true;
}