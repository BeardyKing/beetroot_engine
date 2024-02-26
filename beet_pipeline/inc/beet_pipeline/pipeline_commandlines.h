#ifndef BEETROOT_PIPELINE_COMMANDLINES_H
#define BEETROOT_PIPELINE_COMMANDLINES_H

#include <cstdint>

enum class CLArgs : int32_t {
    help,
    ignoreConvertCache,

    COUNT,
};

struct CLArg {
    const char *name;
    bool enabled;
};

void commandline_init(int32_t argc, char **argv);
CLArg commandline_get_arg(CLArgs clarg);
void commandline_show_commands();

#endif //BEETROOT_PIPELINE_COMMANDLINES_H
