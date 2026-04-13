#include "core.h"

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

// ========
// - String
// ========

String string_alloc_copy_from_cstr(char *cstr) {
    String ret = { 0 };

    ret.size = strlen(cstr);
    ret.str = (char *) malloc(sizeof(char) * ret.size);
    for (uint64_t i = 0; i < ret.size; i++) {
        ret.str[i] = cstr[i];
    }

    return ret;
}

void string_free(String string) {
    string.size = 0;
    free(string.str);
    string.str = NULL;
}

void string_dump(String string) {
    for (uint64_t i = 0; i < string.size; i++) {
        putchar(string.str[i]);
    }
}
// =========
// - Windows
// =========

int win_print_last_error() {
    DWORD err_buf_sz = 512;
    char err_buf[512] = { 0 };
    DWORD last_error = GetLastError();
    DWORD chars_stored = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM,
        0,
        last_error,
        0, // @Note: use proper LANGID?
        err_buf,
        err_buf_sz,
        0
    );

    if (chars_stored == 0) {
        printf("Failed to format the resulting message for last error: %d\n", last_error);

        return -1;
    }

    printf("%s\n", err_buf);

    return 0;
}

// @Note: this would be a nice place to try arenas.
String file_read_string_alloc(const char *file) {
    String ret = { 0 };

    HANDLE handle = CreateFileA(
        file,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (handle == INVALID_HANDLE_VALUE) {
        printf("Failed to open %s: ", file);
        win_print_last_error();

        return ret;
    }

    uint32_t alloc_step_amt = 1024;
    char *buf = (char *) malloc(sizeof(char) * alloc_step_amt);
    uint64_t bytes_read_total = 0;
    DWORD bytes_read = 0;
    uint64_t alloc_counter = 1;
    while (true) {
        if (!ReadFile(handle, buf+bytes_read_total, (DWORD) alloc_step_amt, &bytes_read, NULL)) {
            win_print_last_error();

            return ret;
        }

        bytes_read_total += bytes_read;
        if (bytes_read < alloc_step_amt) {
            break;
        }

        alloc_counter += 1;
        buf = (char *) realloc(buf, alloc_step_amt * alloc_counter);
    }

    ret.str = buf;
    ret.size = bytes_read_total;

    return ret;
}