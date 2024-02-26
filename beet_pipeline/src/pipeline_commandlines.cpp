#ifndef BEETROOT_PIPELINE_COMMANDLINES_CPP
#define BEETROOT_PIPELINE_COMMANDLINES_CPP

#include <beet_pipeline/pipeline_commandlines.h>
#include <beet_shared/log.h>

#include <cstring>

CLArg s_commandLines[(size_t) CLArgs::COUNT] = {};

void commandline_set_args(const int32_t argc, char **argv) {
    for (uint32_t i = 0; i < argc; ++i) {
        if (argv[i][0] != '-') {
            continue;
        }
        for (int j = 0; j < (uint32_t) CLArgs::COUNT; ++j) {
            if (strcmp(s_commandLines[j].name, argv[i]) == 0) {
                s_commandLines[j].enabled = !s_commandLines[j].enabled;
            }
        }
    }
}

void commandline_init(const int32_t argc, char **argv) {
    {
        s_commandLines[(size_t) CLArgs::help] = {.name = "-help", .enabled = false};
        s_commandLines[(size_t) CLArgs::ignoreConvertCache] = {.name = "-ignoreConvertCache", .enabled = false};
    }

    commandline_set_args(argc, argv);
}

CLArg commandline_get_arg(CLArgs clarg) {
    return s_commandLines[(size_t) clarg];
}

void commandline_show_commands() {
    log_debug(MSG_PIPELINE, "Pipeline commands & current state:\n");
    for (int32_t i = 0; i < (size_t) CLArgs::COUNT; ++i) {
        log_debug(MSG_PIPELINE, "\t[%u] \"%s\" = %u\n", i, s_commandLines[i].name, s_commandLines[i].enabled)
    }
};

#endif //BEETROOT_PIPELINE_COMMANDLINES_CPP
