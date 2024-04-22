#ifndef BEETROOT_SHADER_COMPILE_H
#define BEETROOT_SHADER_COMPILE_H

#include <string>

//===API================================================================================================================
void pipeline_build_shader_spv(const std::string &readPath, const std::string &writePath);
void pipeline_shader_log();
//======================================================================================================================

#endif //BEETROOT_SHADER_COMPILE_H
