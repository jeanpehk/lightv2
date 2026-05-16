#include "core.h"

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#define ARENA_DEFAULT_RESERVE_SIZE MB(64)
#define ARENA_DEFAULT_COMMIT_SIZE  KB(64)

// ========
// - String
// ========

void string_dump(String string) {
    printf("%s", string.str);
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

uint32_t win_read_file(const char *file, uint8_t *buf, uint32_t buf_size) {
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

String *file_read_string(Arena *arena, const char *file) {
    String *ret = (String *) arena_push_zero(arena, sizeof(String));

    uint8_t *buf = arena_get_pos_ptr(arena);
    uint64_t buf_size_64 = arena_commit_space_left(arena);
    uint32_t buf_size_32 = (uint32_t) MIN(UINT32_MAX, buf_size_64);
    uint32_t bytes_read = win_read_file(file, buf, buf_size_32);
    arena_push(arena, bytes_read+1);
    buf[bytes_read] = '\0';

    ret->str = (char *) buf;
    ret->len = bytes_read;

    return ret;
}

// =======
// - Arena
// =======

// @Todo: VirtualAlloc
Arena arena_alloc() {
    Arena arena = { 0 };

    arena.mem = (uint8_t *) malloc(ARENA_DEFAULT_COMMIT_SIZE);
    // @Note: change this when switching to valloc
    arena.reserve_size = ARENA_DEFAULT_COMMIT_SIZE;
    arena.commit_size = ARENA_DEFAULT_COMMIT_SIZE;
    arena.pos = 0;

    return arena;
}

void arena_release(Arena *arena) {
    // @Todo: VirtualAlloc
    free(arena->mem);
    arena->reserve_size = 0;
    arena->commit_size = 0;
    arena->pos = 0;
}

void *_arena_push(Arena *arena, uint64_t size, bool zero) {
    void *ret = arena->mem + arena->pos;

    if (arena->pos + size > arena->commit_size) {
        // @Todo: commit more if reserve allows.
        printf("exit: Arena full\n");
        exit(-1);
    }

    if (zero) {
        for (uint64_t i = 0; i < size; i++) {
            arena->mem[arena->pos+i] = 0;
        }
    }

    arena->pos += size;

    return ret;
}

void *arena_push(Arena *arena, uint64_t size) {
    return _arena_push(arena, size, false);
}

void *arena_push_zero(Arena *arena, uint64_t size) {
    return _arena_push(arena, size, true);
}

uint8_t *arena_get_pos_ptr(Arena *arena) {
    return arena->mem + arena->pos;
}

uint64_t arena_commit_space_left(Arena *arena) {
    return arena->commit_size - arena ->pos;
}