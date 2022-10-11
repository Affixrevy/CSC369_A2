#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include "thread.h"
#include "interrupt.h"

#define READY_QUEUE_NO_ITEM (-10)

/* This is the wait queue structure */
struct wait_queue {
    /* ... Fill this in Assignment 3 ... */
};

// enum for thread state clarity
enum thread_states {READY = 0, RUNNING = 1, EXITED = 2};


/* This is the thread control block */
/* Implementation Details
 *  - thread has attributes:
 *      - Thread ID             Unique identifier for the thread number. Must be 0 <= thread_id <= THREAD_MAX_THREADS
 *      - Next Thread           The next thread in its queue
 *      - Thread State          Represented by enum thread_states: READY, RUNNING, and EXITED
 *      - Context               Registers etc for the thread
 */
struct thread {
    /* ... Fill this in ... */
    Tid thread_id;
    // struct thread *next_thread;     // The next thread in the queue to executed
    int thread_state;               // This can have three states: READY (0), RUNNING (1), EXITED (2)
    ucontext_t thread_context;      // Save the context of the thread
};

// Function pre-declarations

static Tid change_threads(struct thread *next_thread);

/* Threads data structures
 *  - Linked list queue for ready
 *  - Linked list queue for exited
 *  - Global array for taken Tid
 */

struct thread all_threads[THREAD_MAX_THREADS];

struct thread *running;
struct thread *killed;

Tid all_tid[THREAD_MAX_THREADS], ready_queue[THREAD_MAX_THREADS];

int ready_queue_head, ready_queue_tail;

static void ready_queue_enqueue(Tid new_id) {
    ready_queue[ready_queue_head] = new_id;
    ready_queue_head++;
    ready_queue_head %= THREAD_MAX_THREADS;
}

static Tid ready_queue_dequeue() {
    Tid result = ready_queue[ready_queue_tail];
    ready_queue[ready_queue_tail] = READY_QUEUE_NO_ITEM;
    ready_queue_tail++;
    ready_queue_tail %= THREAD_MAX_THREADS;
}

void
thread_init (void)
{
    /* Add necessary initialization for your threads library here. */
	/* Initialize the thread control block for the first thread */

    // Zero out thread memory and Tid List
    memset(all_threads, 0, sizeof all_threads);

    for (int i = 0; i < THREAD_MAX_THREADS; ++i) {
        all_tid[i] = 0;
        all_threads[i].thread_id = i;
    }

    // Set up ready queue
    memset(ready_queue, READY_QUEUE_NO_ITEM, sizeof ready_queue);
    ready_queue_head = 0;
    ready_queue_tail = 0;

    // Manually create first thread
    all_tid[0] = 1;
    all_threads[0].thread_state = RUNNING;

    // Set global variables
    running = &all_threads[0];
    killed = NULL;

}

Tid
thread_id ()
{
    // TBD();
    if (all_tid[running->thread_id]) {
        return running->thread_id;
    }

    return THREAD_INVALID;
}

/* New thread starts by calling thread_stub. The arguments to thread_stub are
 * the thread_main() function, and one argument to the thread_main() function. 
 */
void
thread_stub (void (*thread_main)(void *), void *arg)
{
	thread_main(arg); // call thread_main() function with arg
	thread_exit(0);
}

Tid
thread_create (void (*fn) (void *), void *parg)
{
    TBD();
    return THREAD_FAILED;
}

Tid
thread_yield (Tid want_tid)
{

    // TODO: SUGGESTED FIRST
    // TBD();

    if (want_tid == THREAD_ANY) {
        // Get the next available thread

        int check_index = ready_queue_tail;
        while (ready_queue[check_index] == READY_QUEUE_NO_ITEM) {
            check_index++;
            check_index %= THREAD_MAX_THREADS;
        }

        return change_threads(&all_threads[check_index]);

    } else if (want_tid == THREAD_SELF) {
        // Does nothing as current thread continues
        return thread_id();
    } else {
        if (!all_tid[want_tid]
        || all_threads[want_tid].thread_state == EXITED
        || want_tid < THREAD_SELF
        || want_tid > THREAD_MAX_THREADS) {
            // TODO: Kill some zombie processes

            return THREAD_INVALID;
        }

        int check_index = ready_queue_tail;
        while (ready_queue[check_index] != want_tid) {
            check_index++;
            check_index %= THREAD_MAX_THREADS;
        }

        ready_queue[check_index] = ready_queue[ready_queue_tail];
        ready_queue[ready_queue_tail] = READY_QUEUE_NO_ITEM;
        ready_queue_tail++;
        ready_queue_tail %= THREAD_MAX_THREADS;

        return change_threads(&all_threads[want_tid]);
    }

    return THREAD_FAILED;
}

void
thread_exit (int exit_code)
{
    TBD();
}

Tid
thread_kill (Tid tid)
{
    TBD();
    return THREAD_FAILED;
}

Tid change_thread (struct thread *next_thread) {

    struct thread *old_thread = running;

    getcontext(&old_thread->thread_context);

    ready_queue_enqueue(old_thread->thread_id);

    setcontext(next_thread->thread_context);

    running = next_thread;

    if (thread_id() == next_thread->thread_id) {
        return thread_id();
    }

    return THREAD_FAILED;

}

/**************************************************************************
 * Important: The rest of the code should be implemented in Assignment 3. *
 **************************************************************************/

/* make sure to fill the wait_queue structure defined above */
struct wait_queue *
wait_queue_create()
{
    struct wait_queue *wq;

    wq = malloc(sizeof(struct wait_queue));
    assert(wq);

    TBD();

    return wq;
}

void
wait_queue_destroy(struct wait_queue *wq)
{
    TBD();
    free(wq);
}

Tid
thread_sleep(struct wait_queue *queue)
{
    TBD();
    return THREAD_FAILED;
}

/* when the 'all' parameter is 1, wakeup all threads waiting in the queue.
 * returns whether a thread was woken up on not. */
int
thread_wakeup(struct wait_queue *queue, int all)
{
    TBD();
    return 0;
}

/* suspend current thread until Thread tid exits */
Tid
thread_wait(Tid tid, int *exit_code)
{
    TBD();
    return 0;
}

struct lock {
    /* ... Fill this in ... */
};

struct lock *
lock_create()
{
    struct lock *lock;

    lock = malloc(sizeof(struct lock));
    assert(lock);

    TBD();

    return lock;
}

void
lock_destroy(struct lock *lock)
{
    assert(lock != NULL);

    TBD();

    free(lock);
}

void
lock_acquire(struct lock *lock)
{
    assert(lock != NULL);

    TBD();
}

void
lock_release(struct lock *lock)
{
    assert(lock != NULL);

    TBD();
}

struct cv {
    /* ... Fill this in ... */
};

struct cv *
cv_create()
{
    struct cv *cv;

    cv = malloc(sizeof(struct cv));
    assert(cv);

    TBD();

    return cv;
}

void
cv_destroy(struct cv *cv)
{
    assert(cv != NULL);

    TBD();

    free(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
    assert(cv != NULL);
    assert(lock != NULL);

    TBD();
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
    assert(cv != NULL);
    assert(lock != NULL);

    TBD();
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
    assert(cv != NULL);
    assert(lock != NULL);

    TBD();
}
