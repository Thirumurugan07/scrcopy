#include <Windows.h>

typedef struct {
    LONG flag;
} atomic_flag;

void atomic_flag_init(atomic_flag* flag) {
    flag->flag = 0;
}

int atomic_flag_test_and_set(atomic_flag* flag) {
    return InterlockedExchange(&flag->flag, 1);
}

void atomic_flag_clear(atomic_flag* flag) {
    InterlockedExchange(&flag->flag, 0);
}


typedef struct {
    volatile LONG value;
} atomic_uint;

inline void atomic_uint_init(atomic_uint* a, uint_fast32_t value) {
    InterlockedExchange(&a->value, value);
}

inline uint_fast32_t atomic_uint_load(atomic_uint* a) {
    return InterlockedCompareExchange(&a->value, 0, 0);
}

inline void atomic_uint_store(atomic_uint* a, uint_fast32_t value) {
    InterlockedExchange(&a->value, value);
}

inline void atomic_uint_add(atomic_uint* a, uint_fast32_t value) {
    InterlockedExchangeAdd(&a->value, value);
}

typedef atomic_uint sc_atomic_thread_id;