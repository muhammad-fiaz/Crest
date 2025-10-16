/**
 * @file thread_pool.c
 * @brief Thread pool implementation for concurrent request processing
 */

#include "crest/crest.h"
#include "crest/types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <windows.h>
#include <process.h>
#define pthread_t HANDLE
#define pthread_create(thread, attr, func, arg) \
    (*thread = (HANDLE)_beginthreadex(NULL, 0, (unsigned int(__stdcall*)(void*))func, arg, 0, NULL))
/* Wait for thread then close handle to avoid leaks */
#define pthread_join(thread, retval) do { WaitForSingleObject(thread, INFINITE); CloseHandle(thread); } while(0)
#define pthread_mutex_t CRITICAL_SECTION
#define pthread_mutex_init(mutex, attr) InitializeCriticalSection(mutex)
#define pthread_mutex_destroy(mutex) DeleteCriticalSection(mutex)
#define pthread_mutex_lock(mutex) EnterCriticalSection(mutex)
#define pthread_mutex_unlock(mutex) LeaveCriticalSection(mutex)
#define pthread_cond_t CONDITION_VARIABLE
#define pthread_cond_init(cond, attr) InitializeConditionVariable(cond)
#define pthread_cond_destroy(cond) /* No-op on Windows */
#define pthread_cond_wait(cond, mutex) SleepConditionVariableCS(cond, mutex, INFINITE)
#define pthread_cond_signal(cond) WakeConditionVariable(cond)
#define pthread_cond_broadcast(cond) WakeAllConditionVariable(cond)
#else
#include <pthread.h>
#endif

/* ====================================================================
 * TASK QUEUE
 * ==================================================================== */

typedef struct task_node {
    void (*function)(void *arg);
    void *arg;
    struct task_node *next;
} task_node_t;

typedef struct {
    task_node_t *head;
    task_node_t *tail;
    size_t size;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} task_queue_t;

static task_queue_t* task_queue_create(void) {
    task_queue_t *queue = (task_queue_t*)malloc(sizeof(task_queue_t));
    if (!queue) return NULL;

    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;

#ifdef _WIN32
    /* Windows: InitializeCriticalSection and InitializeConditionVariable return void */
    InitializeCriticalSection(&queue->mutex);
    InitializeConditionVariable(&queue->cond);
#else
    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        free(queue);
        return NULL;
    }

    if (pthread_cond_init(&queue->cond, NULL) != 0) {
        pthread_mutex_destroy(&queue->mutex);
        free(queue);
        return NULL;
    }
#endif

    return queue;
}

static void task_queue_destroy(task_queue_t *queue) {
    if (!queue) return;

    pthread_mutex_lock(&queue->mutex);
    task_node_t *node = queue->head;
    while (node) {
        task_node_t *next = node->next;
        free(node);
        node = next;
    }
    pthread_mutex_unlock(&queue->mutex);

    pthread_cond_destroy(&queue->cond);
    pthread_mutex_destroy(&queue->mutex);
    free(queue);
}

static bool task_queue_push(task_queue_t *queue, void (*function)(void *), void *arg) {
    if (!queue || !function) return false;

    task_node_t *node = (task_node_t*)malloc(sizeof(task_node_t));
    if (!node) return false;

    node->function = function;
    node->arg = arg;
    node->next = NULL;

    pthread_mutex_lock(&queue->mutex);

    if (queue->tail) {
        queue->tail->next = node;
    } else {
        queue->head = node;
    }
    queue->tail = node;
    queue->size++;

    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);

    return true;
}

static bool task_queue_pop(task_queue_t *queue, void (**function)(void *), void **arg) {
    if (!queue || !function || !arg) return false;

    pthread_mutex_lock(&queue->mutex);

    while (queue->size == 0) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    task_node_t *node = queue->head;
    *function = node->function;
    *arg = node->arg;

    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    queue->size--;

    pthread_mutex_unlock(&queue->mutex);

    free(node);
    return true;
}

/* ====================================================================
 * THREAD POOL
 * ==================================================================== */

struct crest_thread_pool {
    pthread_t *threads;
    size_t thread_count;
    task_queue_t *queue;
    bool shutdown;
    pthread_mutex_t shutdown_mutex;
};

typedef struct {
    crest_thread_pool_t *pool;
    size_t thread_id;
} thread_worker_args_t;

/* Sentinel task used to wake workers during shutdown */
static void thread_sentinel(void *arg) {
    (void)arg; /* no-op */
}

#ifdef _WIN32
static unsigned __stdcall thread_worker_wrapper(void *arg) {
    thread_worker(arg);
    return 0;
}
#endif

static void* thread_worker(void *arg) {
    thread_worker_args_t *args = (thread_worker_args_t*)arg;
    crest_thread_pool_t *pool = args->pool;
    size_t thread_id = args->thread_id;
    free(args);

    while (true) {
        void (*function)(void *) = NULL;
        void *task_arg = NULL;

        // Check if we should shutdown
        pthread_mutex_lock(&pool->shutdown_mutex);
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->shutdown_mutex);
            break;
        }
        pthread_mutex_unlock(&pool->shutdown_mutex);

        // Get next task
        if (!task_queue_pop(pool->queue, &function, &task_arg)) {
            continue;
        }

        // Execute task
        if (function) {
            function(task_arg);
        }
    }

    return NULL;
}

crest_thread_pool_t* crest_thread_pool_create(size_t thread_count) {
    if (thread_count == 0) {
#ifdef _WIN32
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        thread_count = sysinfo.dwNumberOfProcessors;
#else
        thread_count = sysconf(_SC_NPROCESSORS_ONLN);
#endif
        if (thread_count <= 0) thread_count = 4;
    }

    crest_thread_pool_t *pool = (crest_thread_pool_t*)malloc(sizeof(crest_thread_pool_t));
    if (!pool) return NULL;

    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_count);
    if (!pool->threads) {
        free(pool);
        return NULL;
    }

    pool->queue = task_queue_create();
    if (!pool->queue) {
        free(pool->threads);
        free(pool);
        return NULL;
    }

    pool->thread_count = thread_count;
    pool->shutdown = false;

#ifdef _WIN32
    InitializeCriticalSection(&pool->shutdown_mutex);
#else
    if (pthread_mutex_init(&pool->shutdown_mutex, NULL) != 0) {
        task_queue_destroy(pool->queue);
        free(pool->threads);
        free(pool);
        return NULL;
    }
#endif

    // Start worker threads
    for (size_t i = 0; i < thread_count; i++) {
        thread_worker_args_t *args = (thread_worker_args_t*)malloc(sizeof(thread_worker_args_t));
        if (!args) {
            // Cleanup and return NULL
            pool->shutdown = true;
            for (size_t j = 0; j < i; j++) {
                pthread_join(pool->threads[j], NULL);
            }
            pthread_mutex_destroy(&pool->shutdown_mutex);
            task_queue_destroy(pool->queue);
            free(pool->threads);
            free(pool);
            return NULL;
        }

        args->pool = pool;
        args->thread_id = i;

    /* Use wrapper on Windows because _beginthreadex expects unsigned __stdcall */
#ifdef _WIN32
    if ((pool->threads[i] = (pthread_t)_beginthreadex(NULL, 0, thread_worker_wrapper, args, 0, NULL)) == 0) {
#else
    if (pthread_create(&pool->threads[i], NULL, thread_worker, args) != 0) {
#endif
            free(args);
            // Cleanup and return NULL
            pool->shutdown = true;
            for (size_t j = 0; j < i; j++) {
                pthread_join(pool->threads[j], NULL);
            }
            pthread_mutex_destroy(&pool->shutdown_mutex);
            task_queue_destroy(pool->queue);
            free(pool->threads);
            free(pool);
            return NULL;
        }
    }

    return pool;
}

void crest_thread_pool_destroy(crest_thread_pool_t *pool) {
    if (!pool) return;

    // Signal shutdown
    pthread_mutex_lock(&pool->shutdown_mutex);
    pool->shutdown = true;
    pthread_mutex_unlock(&pool->shutdown_mutex);

    // Push sentinel tasks so workers wake, exit loop and return
    for (size_t i = 0; i < pool->thread_count; ++i) {
        task_queue_push(pool->queue, thread_sentinel, NULL);
    }
    // Also broadcast to ensure any waiting on cond variable wakes
    pthread_cond_broadcast(&pool->queue->cond);

    // Wait for all threads to finish
    for (size_t i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&pool->shutdown_mutex);
    task_queue_destroy(pool->queue);
    free(pool->threads);
    free(pool);
}

bool crest_thread_pool_submit(crest_thread_pool_t *pool, void (*function)(void *), void *arg) {
    if (!pool || !function) return false;

    pthread_mutex_lock(&pool->shutdown_mutex);
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->shutdown_mutex);
        return false;
    }
    pthread_mutex_unlock(&pool->shutdown_mutex);

    return task_queue_push(pool->queue, function, arg);
}

size_t crest_thread_pool_get_thread_count(const crest_thread_pool_t *pool) {
    return pool ? pool->thread_count : 0;
}

size_t crest_thread_pool_get_queue_size(const crest_thread_pool_t *pool) {
    if (!pool) return 0;

    pthread_mutex_lock(&pool->queue->mutex);
    size_t size = pool->queue->size;
    pthread_mutex_unlock(&pool->queue->mutex);

    return size;
}

/* ====================================================================
 * REQUEST PROCESSING TASK
 * ==================================================================== */

typedef struct {
    crest_app_t *app;
    crest_request_t *req;
    crest_response_t *res;
} request_task_t;

static void process_request_task(void *arg) {
    request_task_t *task = (request_task_t*)arg;
    if (!task || !task->app || !task->req || !task->res) {
        free(task);
        return;
    }

    // Process the request through the application
    crest_app_process_request(task->app, task->req, task->res);

    // Clean up
    crest_request_destroy(task->req);
    crest_response_destroy(task->res);
    free(task);
}

bool crest_thread_pool_submit_request(crest_thread_pool_t *pool, crest_app_t *app,
                                     crest_request_t *req, crest_response_t *res) {
    if (!pool || !app || !req || !res) return false;

    request_task_t *task = (request_task_t*)malloc(sizeof(request_task_t));
    if (!task) return false;

    task->app = app;
    task->req = req;
    task->res = res;

    return crest_thread_pool_submit(pool, process_request_task, task);
}