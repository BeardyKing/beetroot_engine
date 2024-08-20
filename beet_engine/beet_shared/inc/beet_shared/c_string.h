#ifndef BEETROOT_C_STRING_H
#define BEETROOT_C_STRING_H

#include <cstdint>

//===API================================================================================================================
bool c_str_empty(const char *inStr);
bool c_str_equal(const char *lhs, const char *rhs);
bool c_str_n_equal(const char *lhs, const char *rhs, size_t count);

const char *c_str_search_reverse(const char *src, const char *subStr);
char *c_str_search_reverse(char *src, const char *subStr);

const char *c_str_n_search_reverse(const char *src, int32_t srcLen, const char *subStr, int32_t subStrLen);
char *c_str_n_search_reverse(char *src, int32_t srcLen, const char *subStr, int32_t subStrLen);

bool c_str_replace_after_delim_reverse(char *existingPath, const char *replaceTarget, const char *subStr);
bool c_string_replace_extension(char *existingPath, const char *newExtension);
bool c_string_remove_file_from_path(const char *inPath, char *outPath);
bool c_string_extract_file_name(const char *inPath, char *outFilename);
//======================================================================================================================

#endif //BEETROOT_C_STRING_H
