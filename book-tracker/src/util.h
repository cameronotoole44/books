#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

void trim_newline(char *s);
int file_exists(const char *path);
void safe_strcpy(char *dst, size_t dst_size, const char *src);

#endif
