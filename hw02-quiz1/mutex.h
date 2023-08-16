#pragma once

#if USE_PTHREADS

#include <pthread.h>

#define mutex_t pthread_mutex_t
#define mutex_init(m) pthread_mutex_init(m, NULL)
#define MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#define mutex_trylock(m) (!pthread_mutex_trylock(m))
#define mutex_lock pthread_mutex_lock
#define mutex_unlock pthread_mutex_unlock

#else

#include <stdbool.h>
#include "atomic.h"
#include "futex.h"
#include "spinlock.h"

typedef struct {
    atomic int state;
} mutex_t;

enum {
    MUTEX_LOCKED = 1 << 0,
    MUTEX_SLEEPING = 1 << 1,
};

#define MUTEX_INITIALIZER \
    {                     \
        .state = 0        \
    }

static inline void mutex_init(mutex_t *mutex)
{
    atomic_init(&mutex->state, 0);
}

static bool mutex_trylock(mutex_t *mutex)
{
    /*
     * Is the mutex already locked?
     * Fails trylock if already locked.
     */
    int state = load(&mutex->state, relaxed);
    if (state & MUTEX_LOCKED)
        return false;

    /*
     * Change the state to LOCKED to lock the mutex.
     *
     * During the interval between the previous check and the
     * following atomic instruction, there may have been another
     * thread taking the lock earlier than we could. Guard against
     * this case by checking the returned |state| again. If the
     * returned |state| is LOCKED, we lose the race and fail trylock.
     */
    state = fetch_or(&mutex->state, MUTEX_LOCKED, relaxed);
    if (state & MUTEX_LOCKED)
        return false;

    /*
     * Successfully lock the mutex.
     *
     * Put an acquire fence here so that atomic loads above must
     * happen-before the atomic loads and writes after this fence.
     * Without this happen-before relation, there could be, for
     * example, two threads that read an unlocked |state| and
     * determine that both of them have locked the mutex, resulting
     * in race condition hereafter.
     */
    thread_fence(&mutex->state, acquire);
    return true;
}

static inline void mutex_lock(mutex_t *mutex)
{
#define MUTEX_SPINS 128
    for (int i = 0; i < MUTEX_SPINS; ++i) {
        if (mutex_trylock(mutex))
            return;
        spin_hint();
    }

    /*
     * Try locking the mutex and indicate that some thread might
     * have slept on a |futex_wait| call so that later |mutex_unlock|
     * can invoke |futex_wake| to wake up the sleeping thread.
     */
    int state = exchange(&mutex->state, MUTEX_LOCKED | MUTEX_SLEEPING, relaxed);

    /*
     * As long as another thread has already locked the mutex, as
     * indicated by the returned |state|, this thread shall sleep
     * on futex.
     */
    while (state & MUTEX_LOCKED) {
        futex_wait(&mutex->state, MUTEX_LOCKED | MUTEX_SLEEPING);
        state = exchange(&mutex->state, MUTEX_LOCKED | MUTEX_SLEEPING, relaxed);
    }

    /*
     * Successfully lock the mutex.
     *
     * Put an acquire fence here so that no more than one thread
     * shall see an unlocked mutex at the same time, as commented
     * in mutex_trylock() function.
     */
    thread_fence(&mutex->state, acquire);
}

static inline void mutex_unlock(mutex_t *mutex)
{
    int state = exchange(&mutex->state, 0, release);
    if (state & MUTEX_SLEEPING)
        futex_wake(&mutex->state, 1);
}

#endif