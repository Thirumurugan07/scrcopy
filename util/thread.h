#ifndef THREAD_H
#define THREAD_H

#include <stdbool.h>
#include <SDL2/SDL_thread.h>
#include "log.h"

/* Forward declarations */
typedef struct SDL_Thread SDL_Thread;

/* Typedef for thread function */
typedef int sc_thread_fn(void*);

/* Structure for thread */
typedef struct sc_thread {
    SDL_Thread* thread;
} sc_thread;

typedef struct {
    SDL_mutex* mutex;
} sc_mutex;

typedef struct {
    SDL_cond* cond;
} sc_cond;

/* Enum for thread priority */
enum sc_thread_priority {
    SC_THREAD_PRIORITY_LOW,
    SC_THREAD_PRIORITY_NORMAL,
    SC_THREAD_PRIORITY_HIGH,
    SC_THREAD_PRIORITY_TIME_CRITICAL,
};

bool sc_thread_create(sc_thread* thread, sc_thread_fn fn, const char* name,
    void* userdata);

bool sc_thread_set_priority(enum sc_thread_priority priority);

void sc_thread_join(sc_thread* thread, int* status);

bool sc_mutex_init(sc_mutex* mutex);

void sc_mutex_destroy(sc_mutex* mutex);

void sc_mutex_lock(sc_mutex* mutex);

void sc_mutex_unlock(sc_mutex* mutex);

bool sc_cond_init(sc_cond* cond);

void sc_cond_destroy(sc_cond* cond);

void sc_cond_wait(sc_cond* cond, sc_mutex* mutex);

bool sc_cond_timedwait(sc_cond* cond, sc_mutex* mutex, uint32_t ms);

void sc_cond_signal(sc_cond* cond);

void sc_cond_broadcast(sc_cond* cond);
#endif  // THREAD_H
