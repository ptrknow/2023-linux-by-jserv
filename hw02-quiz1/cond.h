#pragma once

#if USE_PTHREADS

#include <pthread.h>

#define cond_t pthread_cond_t
#define cond_init(c) pthread_cond_init(c, NULL)
#define COND_INITIALIZER PTHREAD_COND_INITIALIZER
#define cond_wait(c, m) pthread_cond_wait(c, m)
#define cond_signal(c, m) pthread_cond_signal(c)
#define cond_broadcast(c, m) pthread_cond_broadcast(c)

#else

#include <limits.h>
#include <stddef.h>
#include "atomic.h"
#include "futex.h"
#include "mutex.h"
#include "spinlock.h"

typedef struct {
    atomic int seq;
} cond_t;

static inline void cond_init(cond_t *cond)
{
    atomic_init(&cond->seq, 0);
}

static inline void cond_wait(cond_t *cond, mutex_t *mutex)
{
    /*
     * Q: Why must an initial |cond->seq| be loaded first
     * before unlocking the mutex?
     * A: TODO -- find out the answer
     */
    int seq = load(&cond->seq, relaxed);

    mutex_unlock(mutex);

    /*
     * The fast path: try to lock the mutex again right
     * after unlocking the mutex.
     *
     * We can lock the mutex in this fast path if some
     * other threads have called cond_[signal|broadcast]
     * that alters |cond->seq|.
     */
#define COND_SPINS 128
    for (int i = 0; i < COND_SPINS; ++i) {
        if (load(&cond->seq, relaxed) != seq) {
            mutex_lock(mutex);
            return;
        }
        spin_hint();
    }

    /*
     * Sleep on the futext associated with |cond->seq|
     * when we cannot lock the mutex during the above
     * spin.
     */
    futex_wait(&cond->seq, seq);

    /*
     * We come to here with one of two possibilities:
     * (1) we are woken up from the futex
     * (2) |cond->seq| is altered between the spin and
     *     the call to futex_wait()
     * We can now lock the mutex here.
     */
    mutex_lock(mutex);

    /*
     * This is a necessary code that "cooperates" with
     * cond_broadcast(), which will transfer threads from
     * the wait queue of |cond->seq| to the wait queue of
     * |mutex->state| as an optimization to wakeup.
     *
     * More specifically, threads that are waiting on
     * |cond->seq| by calling futex_wait() above might be
     * transferred to the wait queue of |mutex->state|
     * instead due to cond_broadcast().
     *
     * Basically, we will want those transferred threads
     * to be properly woken up by the mutex_unlock() call
     * that shall subsequently follow a cond_wait() call.
     */
    fetch_or(&mutex->state, MUTEX_SLEEPING, relaxed);
}

static inline void cond_signal(cond_t *cond, mutex_t *mutex)
{
    /*
     * Ignore integer overflow since cond_wait() is only
     * interested in the alteration of |cond->seq| instead
     * of relying on |cond->seq| being a particular value.
     */
    fetch_add(&cond->seq, 1, relaxed);
    futex_wake(&cond->seq, 1);
}

static inline void cond_broadcast(cond_t *cond, mutex_t *mutex)
{
    /*
     * Ignore integer overflow for the same reason as
     * cond_signal() above.
     */
    fetch_add(&cond->seq, 1, relaxed);
    futex_requeue(&cond->seq, 1, &mutex->state);
}

#endif