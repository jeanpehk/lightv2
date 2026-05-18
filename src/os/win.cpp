#include <stdio.h>

#include <windows.h>

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

uint32_t read_file(const char *file, uint8_t *buf, uint32_t buf_size) {
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

        return 0;
    }

    DWORD bytes_read = 0;
    if (!ReadFile(handle, buf, (DWORD) buf_size, &bytes_read, NULL)) {
        win_print_last_error();

        return 0;
    }

    // @Note: technically could be that there is nothing left to read -> are exactly the same size.
    if (bytes_read == buf_size) {
        printf("WARN: win_read_cstring, buf not large enough\n");
    }

    if (CloseHandle(handle) == 0) {
        win_print_last_error();
    }

    return bytes_read;
}