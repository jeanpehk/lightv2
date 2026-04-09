#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

const size_t ARRAY_DEFAULT_CAPACITY = 16;

// Array type, no constructors/destructors.
template <typename T> struct Array {
    size_t len;
    size_t capacity;
    T *data;

    T get(size_t index) {
        assert(data);

        return data[index];
    }

    T *get_ref(size_t index) {
        assert(data);

        return &data[index];
    }

    void push(T item) {
        assert(data);

        if (len >= capacity) {
            capacity *= 2;
            data = (T *) realloc(data, sizeof(T) * capacity);
        }

        data[len] = item;
        len++;
    }

    void pop() {
        assert(data);

        if (len > 0) {
            len--;
        }
    }

    void clear() {
        assert(data);

        // @Note: should we manage data here?
        len = 0;
    }
};

// Explicit creation/deletion functions.

template <typename T> Array<T> array_create() {
    Array<T> a;

    a.len = 0;
    a.capacity = ARRAY_DEFAULT_CAPACITY;
    a.data = (T *) malloc(sizeof(T) * a.capacity);

    return a;
}

template <typename T> void array_shrink(Array<T> *a, size_t shrink_amount) {
    size_t new_len = a->len - shrink_amount < 0 ? 0 : a->len - shrink_amount;

    // @TODO: capacity management

    a->len = new_len;
}

template <typename T> void array_clear(Array<T> *a) {
    a->len = 0;
}

template <typename T> void array_destroy(Array<T> a) {
    free(a.data);
}