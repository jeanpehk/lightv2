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

struct Arena {
    uint8_t *mem;
    uint64_t size;
    uint64_t pos;
};

Arena arena_alloc(uint64_t size);
void arena_release(Arena *arena);
void *arena_push(Arena *arena, uint64_t size);
void *arena_push_zero(Arena *arena, uint64_t size);

template <typename T> struct List_Node {
    T item;
    List_Node *next;
};

template <typename T> struct List {
    uint64_t count;
    List_Node<T> *first;
    List_Node<T> *last;
};

template <typename T> void list_add(Arena *arena, List<T> *list, T item) {
    List_Node<T> *next = (List_Node<T> *) arena_push_zero(arena, sizeof(List_Node<T>));
    next->item = item;
    next->next = NULL;

    if (list->count == 0) {
        list->first = next;
        list->last = next;
    }

    else if (list->count == 1) {
        list->first->next = next;
        list->last = next;
    }

    else {
        list->last->next = next;
        list->last = next;
    }

    list->count++;
}

#endif // CORE_H