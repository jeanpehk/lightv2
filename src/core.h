#ifndef CORE_H
#define CORE_H

#include <stdint.h>

// ========
// - String
// ========

struct String {
    char *str;
    uint64_t size;
};

String file_read_string_alloc(const char *file);
String string_alloc_copy_from_cstr(char *cstr);
void string_free(String string);
void string_dump(String string);

#endif // CORE_H