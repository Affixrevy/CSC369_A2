#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>
//#include <setjmp.h>
#include "thread.h"
#include "interrupt.h"

#define READY_QUEUE_NO_ITEM (-10)

/* This is the wait queue structure */
struct wait_queue {
    /* ... Fill this in Assignment 3 ... */
};

// enum for thread state clarity
enum thread_states {READY = 0, RUNNING = 1, EXITED = 2, UNUSED = 3};


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
    enum thread_states thread_state;        // This can have three states: READY (0), RUNNING (1), EXITED (2)
    ucontext_t thread_context;              // Save the context of the thread
};

// Function pre-declarations

//static Tid change_threads(struct thread *next_thread);

/* Threads data structures
 *  - Linked list queue for ready
 *  - Linked list queue for exited
 *  - Global array for taken Tid
 */

struct thread all_threads[THREAD_MAX_THREADS];

Tid running, killed;

void
thread_init (void)
{
    /* Add necessary initialization for your threads library here. */
	/* Initialize the thread control block for the first thread */

    // Zero out thread memory and Tid List
    memset(all_threads, 0, sizeof all_threads);

    for (int i = 0; i < THREAD_MAX_THREADS; ++i) {
//        all_tid[i] = 0;
        all_threads[i].thread_id = i;
        all_threads[i].thread_state = UNUSED;
    }

    // Set up ready queue
//    memset(ready_queue, READY_QUEUE_NO_ITEM, sizeof ready_queue);
//    ready_queue_head = 0;
//    ready_queue_tail = 0;

    // Manually create first thread
//    all_tid[0] = 1;
    all_threads[0].thread_state = RUNNING;

    // Set global variables
    running = 0;
    killed = -1;
    
}

Tid
thread_id ()
{
    return running;
}

void clean_zombies() {
    if (killed != -1) {
        if (killed != 0) {
            free(all_threads[killed].thread_context.uc_stack.ss_sp);
            memset(&all_threads[killed], 0, sizeof (struct thread));
            all_threads[killed].thread_state = UNUSED;
            all_threads[killed].thread_id = killed;
        } else {
//            memset()
        }
    }

    killed = -1;
}

/* New thread starts by calling thread_stub. The arguments to thread_stub are
 * the thread_main() function, and one argument to the thread_main() function. 
 */
void
thread_stub (void (*thread_main)(void *), void *arg)
{
	clean_zombies();

    thread_main(arg); // call thread_main() function with arg
	thread_exit(0);
}

Tid
thread_create (void (*fn) (void *), void *parg)
{
    ucontext_t *new_thread_context;

    // Get a Tid for new thread
    int found_thread = 0;
    Tid found_id;
    for (int i = 0; i < THREAD_MAX_THREADS; ++i) {
        if(all_threads[i].thread_state == UNUSED) {
            found_id = i;
            found_thread = 1;
            break;
        }
    }

    if (!found_thread) return THREAD_NOMORE;


    // Allocate memory for the thread
    void *stack_ptr = malloc(THREAD_MIN_STACK);
    if (stack_ptr == NULL) {
        return THREAD_NOMEMORY;
    }

    // Set up thread context
//    new_thread_context = &new_thread->thread_context;

    int err = getcontext(&(all_threads[found_id].thread_context));
    assert(!err);

    new_thread_context = &all_threads[found_id].thread_context;

    new_thread_context->uc_mcontext.gregs[REG_RIP] = (unsigned long) thread_stub;       // Set program counter
    new_thread_context->uc_mcontext.gregs[REG_RDI] = (unsigned long) fn;                // Set up arguments
    new_thread_context->uc_mcontext.gregs[REG_RSI] = (unsigned long) parg;              // Set up arguments

    new_thread_context->uc_stack.ss_sp = stack_ptr;                                     // Set thread stack
    new_thread_context->uc_stack.ss_size = THREAD_MIN_STACK;                            // Set thread stack size

    void *stack_start = stack_ptr + THREAD_MIN_STACK - sizeof(long);
    new_thread_context->uc_mcontext.gregs[REG_RSP] = (unsigned long) stack_start;       // Start of the stack since
                                                                                        // stacks grow down

    all_threads[found_id].thread_state = READY;
    return found_id;
}

Tid
thread_yield (Tid want_tid)
{

    // TODO: SUGGESTED FIRST
    // TBD();
    int triggered = 0;
    if (want_tid == THREAD_ANY) {
        // Get the next available thread
        int found_ready = 0;
        Tid current_tid = running;
        Tid next_tid;

        for (int i = 0; i < THREAD_MAX_THREADS; ++i) {
            if(all_threads[(i + current_tid) % THREAD_MAX_THREADS].thread_state == READY) {
                next_tid = (i + current_tid) % THREAD_MAX_THREADS;
                found_ready = 1;
                break;
            }
        }

        if (!found_ready) {
            clean_zombies();
            return THREAD_NONE;
        }

        int err = getcontext(&(all_threads[current_tid].thread_context));
        if (!triggered) {
            triggered = 1;

            assert(!err);
            all_threads[current_tid].thread_state = READY;

            running = next_tid;
            all_threads[next_tid].thread_state = RUNNING;

            err = setcontext(&(all_threads[next_tid].thread_context));
        }

        assert(!err);
        return thread_id();

    } else if (want_tid == THREAD_SELF || want_tid == running) {
        // Does nothing as current thread continues
        clean_zombies();
        return thread_id();
    } else {                        // || !all_tid[want_tid]
        if (want_tid < THREAD_SELF
        || want_tid > THREAD_MAX_THREADS
        || all_threads[want_tid].thread_state != READY) {
            clean_zombies();

            return THREAD_INVALID;
        }


        // get context of currently running thread
        Tid old_id = running;
        int err = getcontext(&(all_threads[old_id].thread_context));
        assert(!err);
        if (!triggered) {
            triggered = 1;
            all_threads->thread_state = READY;

            // set context for new thread
            running = want_tid;
            all_threads[want_tid].thread_state = RUNNING;
            err = setcontext(&(all_threads[want_tid].thread_context));
            assert(!err);
        }
        return want_tid;
    }

    return THREAD_FAILED;
}

void
thread_exit (int exit_code)
{
    Tid exit_id = running;
    all_threads[exit_id].thread_state = EXITED;
    killed = exit_id;

    int found_ready = 0;
    int current_tid = running;
    Tid next_tid;

    for (int i = 0; i < THREAD_MAX_THREADS; ++i) {
        if(all_threads[(i + current_tid) % THREAD_MAX_THREADS].thread_state == READY) {
            next_tid = (i + current_tid) % THREAD_MAX_THREADS;
            found_ready = 1;
            break;
        }
    }

    if (!found_ready) {
        exit(exit_code);
    }

    running = next_tid;
    all_threads[next_tid].thread_state = RUNNING;
    int err = setcontext(&(all_threads[next_tid].thread_context));
    assert(!err);

}

Tid
thread_kill (Tid tid)
{
//    TBD();
    if (tid <= 0 || tid > THREAD_MAX_THREADS || all_threads[tid].thread_state != READY) return THREAD_INVALID;

    all_threads[tid].thread_state = EXITED;
    killed = tid;
    clean_zombies();
    return tid;

}

//Tid change_threads (struct thread *next_thread) {
//
//    setcontext(&next_thread->thread_context);
//
//    running = next_thread;
//
//    if (thread_id() == next_thread->thread_id) {
//        return thread_id();
//    }
//
//    return THREAD_FAILED;
//
//}

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
