#ifndef CORE_H
#define CORE_H

#include <stddef.h>
#include <stdint.h>

#define KB(n)  (((uint64_t)(n)) << 10)
#define MB(n)  (((uint64_t)(n)) << 20)
#define GB(n)  (((uint64_t)(n)) << 30)
#define TB(n)  (((uint64_t)(n)) << 40)

#define MIN(A,B) (((A)<(B)) ? (A):(B))
#define MAX(A,B) (((A)>(B)) ? (A):(B))

// ========
// - Arena
// ========

struct Arena {
    uint8_t *mem;
    uint64_t reserve_size;
    uint64_t commit_size;
    uint64_t pos;
};

Arena arena_alloc();
void arena_release(Arena *arena);
void *arena_push(Arena *arena, uint64_t size);
void *arena_push_zero(Arena *arena, uint64_t size);
uint8_t *arena_get_pos_ptr(Arena *arena);
uint64_t arena_commit_space_left(Arena *arena);

// ========
// - String
// ========

struct String {
    char *str;    // null terminated so that interop with c strings is easier when needed
    uint64_t len;
};

String *file_read_string(Arena *arena, const char *file);
void string_dump(String string);

// =============
// - Linked List
// =============

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