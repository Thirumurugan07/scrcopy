#ifndef  SC_ATOMIC_H
#define SC_ATOMIC_H

#include <Windows.h>

// Define a structure to hold the atomic bool
typedef struct {
    LONG value;
} atomic_bool;

// Function to initialize the atomic bool
void atomic_bool_init(atomic_bool* atomic) {
    atomic->value = 0;
}

// Function to set the atomic bool to a new value
void atomic_bool_store(atomic_bool* atomic, int value) {
    InterlockedExchange(&atomic->value, value ? 1 : 0);
}

// Function to retrieve the current value of the atomic bool
int atomic_bool_load(atomic_bool* atomic) {
    return InterlockedCompareExchange(&atomic->value, 0, 0) != 0;
}
#endif // ! SC_ATOMIC_H
