#ifndef _TOKU_PTHREAD_H
#define _TOKU_PTHREAD_H

#if defined __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef pthread_attr_t toku_pthread_attr_t;
typedef pthread_t toku_pthread_t;
typedef pthread_mutexattr_t toku_pthread_mutexattr_t;
typedef pthread_mutex_t toku_pthread_mutex_t;
typedef pthread_condattr_t toku_pthread_condattr_t;
typedef pthread_cond_t toku_pthread_cond_t;

int toku_pthread_yield(void);

static inline
int toku_pthread_attr_init(toku_pthread_attr_t *attr) {
    return pthread_attr_init(attr);
}

static inline
int toku_pthread_attr_destroy(toku_pthread_attr_t *attr) {
    return pthread_attr_destroy(attr);
}

static inline
int toku_pthread_attr_getstacksize(toku_pthread_attr_t *attr, size_t *stacksize) {
    return pthread_attr_getstacksize(attr, stacksize);
}

static inline
int toku_pthread_attr_setstacksize(toku_pthread_attr_t *attr, size_t stacksize) {
    return pthread_attr_setstacksize(attr, stacksize);
}

static inline
int toku_pthread_create(toku_pthread_t *thread, const toku_pthread_attr_t *attr, void *(*start_function)(void *), void *arg) {
    return pthread_create(thread, attr, start_function, arg);
}

static inline
int toku_pthread_join(toku_pthread_t thread, void **value_ptr) {
    return pthread_join(thread, value_ptr);
}

static inline
toku_pthread_t toku_pthread_self(void) {
    return pthread_self();
}

#define TOKU_PTHREAD_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER

static inline
int toku_pthread_mutex_init(toku_pthread_mutex_t *mutex, const toku_pthread_mutexattr_t *attr) {
    return pthread_mutex_init(mutex, attr);
}

static inline
int toku_pthread_mutex_destroy(toku_pthread_mutex_t *mutex) {
    return pthread_mutex_destroy(mutex);
}

static inline
int toku_pthread_mutex_lock(toku_pthread_mutex_t *mutex) {
    return pthread_mutex_lock(mutex);
}

int toku_pthread_mutex_trylock(toku_pthread_mutex_t *mutex);

static inline
int toku_pthread_mutex_unlock(toku_pthread_mutex_t *mutex) {
    return pthread_mutex_unlock(mutex);
}

static inline
int toku_pthread_cond_init(toku_pthread_cond_t *cond, const toku_pthread_condattr_t *attr) {
    return pthread_cond_init(cond, attr);
}

static inline 
int toku_pthread_cond_destroy(toku_pthread_cond_t *cond) {
    return pthread_cond_destroy(cond);
}

static inline
int toku_pthread_cond_wait(toku_pthread_cond_t *cond, toku_pthread_mutex_t *mutex) {
    return pthread_cond_wait(cond, mutex);
}

static inline
int toku_pthread_cond_signal(toku_pthread_cond_t *cond) {
    return pthread_cond_signal(cond);
}

static inline
int toku_pthread_cond_broadcast(toku_pthread_cond_t *cond) {
    return pthread_cond_broadcast(cond);
}

#if defined __cplusplus
};
#endif

#endif