
#include "stdio.h"

#include "py/runtime.h"
#include "py/gc.h"
#include "py/mpthread.h"
#include "py/mphal.h"
#include "mpthreadport.h"
#include "gd32h7xx.h"
#if MICROPY_PY_THREAD

#define MP_THREAD_MIN_STACK_SIZE                        (4 * 1024)
#define MP_THREAD_DEFAULT_STACK_SIZE                    (MP_THREAD_MIN_STACK_SIZE + MICROPY_STACK_CHECK_MARGIN)
static int task_priority;

typedef enum {
    MP_THREAD_RUN_STATE_NEW,
    MP_THREAD_RUN_STATE_RUNNING,
    MP_THREAD_RUN_STATE_FINISHED,
} mp_thread_run_state_t;

// this structure forms a linked list, one node per active thread
typedef struct _mp_thread_t {
    TaskHandle_t id;        // system id of thread
    mp_thread_run_state_t run_state; // current run state of the thread
    void *arg;              // thread Python args, a GC root pointer
    void *stack;            // pointer to the stack
    size_t stack_len;       // number of words in the stack
    StaticTask_t xTaskBuffer;
    struct _mp_thread_t *next;
} mp_thread_t;

// the mutex controls access to the linked list
static mp_thread_mutex_t thread_mutex;
static mp_thread_t thread_entry0;
static mp_thread_t *thread = NULL; // root pointer, handled by mp_thread_gc_others
static TaskHandle_t  delete_task_handle;
static TaskHandle_t delete_Handle;
static void pyb_delete_task(void);
void mp_thread_init(void *stack, uint32_t stack_len, int priority) {
    mp_thread_set_state(&mp_state_ctx.thread);
    // create the first entry in the linked list of all threads
    thread_entry0.id = xTaskGetCurrentTaskHandle();
    thread_entry0.run_state = MP_THREAD_RUN_STATE_RUNNING;
    thread_entry0.arg = NULL;
    thread_entry0.stack = stack;
    thread_entry0.stack_len = stack_len;
    thread_entry0.next = NULL;
    thread = &thread_entry0;
    mp_thread_mutex_init(&thread_mutex);
    task_priority = priority;
    pyb_delete_task();
    // memory barrier to ensure above data is committed
    // __sync_synchronize();    
    __DSB();
    __ISB();
}

void mp_thread_gc_others(void) {
    mp_thread_mutex_lock(&thread_mutex, 1);
    for (mp_thread_t *th = thread; th != NULL; th = th->next) {
        gc_collect_root((void **)&th, 1);
        gc_collect_root(&th->arg, 1); // probably not needed
        gc_collect_root(&th->stack, 1);
        if (th->id == xTaskGetCurrentTaskHandle()) {
            continue;
        }
        if (th->run_state != MP_THREAD_RUN_STATE_RUNNING) {
            continue;
        }
        gc_collect_root(th->stack, th->stack_len);
    }
    mp_thread_mutex_unlock(&thread_mutex);
}

mp_state_thread_t *mp_thread_get_state(void) {
    return pvTaskGetThreadLocalStoragePointer(NULL, 1);
}

void mp_thread_set_state(mp_state_thread_t *state) {
    vTaskSetThreadLocalStoragePointer(NULL, 1, state);
}

mp_uint_t mp_thread_get_id(void) {
    return (mp_uint_t)xTaskGetCurrentTaskHandle();
}

void mp_thread_start(void) {
    mp_thread_mutex_lock(&thread_mutex, 1);
    for (mp_thread_t *th = thread; th != NULL; th = th->next) {
        if (th->id == xTaskGetCurrentTaskHandle()) {
            th->run_state = MP_THREAD_RUN_STATE_RUNNING;
            break;
        }
    }
    mp_thread_mutex_unlock(&thread_mutex);
}

static void delete_task(void * pvParameter)
{
    uint32_t Communication_task_NotifyValue;
    delete_Handle = NULL;
    while(1)
    {
        Communication_task_NotifyValue=ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
        if(Communication_task_NotifyValue > 0)
        {
            if(delete_Handle != NULL)
            {
                vTaskDelete(delete_Handle);
                mp_thread_mutex_lock(&thread_mutex, 1);
                for (mp_thread_t **th = &thread; *th != NULL; th = &(*th)->next) {
                    if ((*th)->id == delete_Handle) {
                        *th = (*th)->next;
                    }
                }
                mp_thread_mutex_unlock(&thread_mutex);
                delete_Handle = NULL;
                MP_THREAD_GIL_EXIT();
            }
            else{
                printf("delete_task error 1\r\n");
                while (1);
            }
        }
        else{
            printf("delete_task error 2\r\n");
            while (1);
        }
    }
}
static void pyb_delete_task(void)
{
    BaseType_t ret;
	ret = xTaskCreate(delete_task, "delete_task_handle", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &delete_task_handle);
    if(delete_task_handle == NULL || ret != pdPASS)
    {
        printf("delete_task_handle error\r\n");
        while(1);
    }
}

static void *(*ext_thread_entry)(void *) = NULL;

static void freertos_entry(void *arg) {
    // Run the Python code.
    if (ext_thread_entry) {
        ext_thread_entry(arg);
    }
#if 1
    TaskHandle_t task_handle = xTaskGetCurrentTaskHandle();
    vTaskPrioritySet(task_handle,configMAX_PRIORITIES - 2);
    delete_Handle = task_handle;
    mp_thread_finish();
    if(pdPASS != xTaskNotifyGive(delete_task_handle)){
        printf("delete task error 1%p\r\n",task_handle);
        while (1);
    }
    taskYIELD();
    printf("delete task error 2%p\r\n",task_handle);
    
    while(1)
    {
        vTaskDelay(portMAX_DELAY);
    }
#else
    mp_thread_finish();

    MP_THREAD_GIL_EXIT();
    mp_thread_mutex_lock(&thread_mutex, 1);
    for (mp_thread_t **th = &thread; *th != NULL; th = &(*th)->next) {
        if ((*th)->id == xTaskGetCurrentTaskHandle()) {
            *th = (*th)->next;
        }
    }
    mp_thread_mutex_unlock(&thread_mutex);

    // Delete this FreeRTOS task (this call to vTaskDelete will not return).
    vTaskDelete(NULL);
#endif
}
#if 1
mp_uint_t mp_thread_create_ex(void *(*entry)(void *), void *arg, size_t *stack_size, int priority, char *name) {
    // store thread entry function into a global variable so we can access it
    ext_thread_entry = entry;

    if (*stack_size == 0) {
        *stack_size = MP_THREAD_DEFAULT_STACK_SIZE; // default stack size
    } else if (*stack_size < MP_THREAD_MIN_STACK_SIZE) {
        *stack_size = MP_THREAD_MIN_STACK_SIZE; // minimum stack size
    }

    // Allocate linked-list node (must be outside thread_mutex lock)
    mp_thread_t *th = m_new_obj(mp_thread_t);
    void *stack = m_new(StackType_t, *stack_size / sizeof(StackType_t));
    mp_thread_mutex_lock(&thread_mutex, 1);

    // create thread
    th->id = xTaskCreateStatic(freertos_entry, name, *stack_size / sizeof(StackType_t), arg, priority, stack, &th->xTaskBuffer);
    if (th->id == NULL) {
        mp_thread_mutex_unlock(&thread_mutex);
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("can't create thread"));
    }
    // add thread to linked list of all threads
    th->run_state = MP_THREAD_RUN_STATE_NEW;
    th->arg = arg;
    th->stack = stack;
    th->stack_len = *stack_size / sizeof(StackType_t);
    th->next = thread;
    thread = th;
    // printf("T:th:%p %p stack:%p\r\n",&th,th,stack);
    mp_thread_mutex_unlock(&thread_mutex);

    return (mp_uint_t)th->id;
}
#else
mp_uint_t mp_thread_create_ex(void *(*entry)(void *), void *arg, size_t *stack_size, int priority, char *name) {
    // store thread entry function into a global variable so we can access it
    ext_thread_entry = entry;

    if (*stack_size == 0) {
        *stack_size = MP_THREAD_DEFAULT_STACK_SIZE; // default stack size
    } else if (*stack_size < MP_THREAD_MIN_STACK_SIZE) {
        *stack_size = MP_THREAD_MIN_STACK_SIZE; // minimum stack size
    }

    // Allocate linked-list node (must be outside thread_mutex lock)
    mp_thread_t *th = m_new_obj(mp_thread_t);

    mp_thread_mutex_lock(&thread_mutex, 1);

    // create thread
    BaseType_t result = xTaskCreate(freertos_entry, name, *stack_size / sizeof(StackType_t), arg, priority, &th->id);
    if (result != pdPASS) {
        mp_thread_mutex_unlock(&thread_mutex);
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("can't create thread"));
    }
    TaskStatus_t taskDetails;
    vTaskGetInfo(th->id, &taskDetails, pdTRUE, eInvalid);
    // add thread to linked list of all threads
    th->run_state = MP_THREAD_RUN_STATE_NEW;
    th->arg = arg;
    th->stack = taskDetails.pxStackBase;
    th->stack_len = *stack_size / sizeof(uintptr_t);
    th->next = thread;
    thread = th;

    mp_thread_mutex_unlock(&thread_mutex);

    return (mp_uint_t)th->id;
}
#endif
mp_uint_t mp_thread_create(void *(*entry)(void *), void *arg, size_t *stack_size) {
    return mp_thread_create_ex(entry, arg, stack_size, task_priority, "mp_thread");
}

void mp_thread_finish(void) {
    mp_thread_mutex_lock(&thread_mutex, 1);
    for (mp_thread_t *th = thread; th != NULL; th = th->next) {
        if (th->id == xTaskGetCurrentTaskHandle()) {
            th->run_state = MP_THREAD_RUN_STATE_FINISHED;
            break;
        }
    }
    mp_thread_mutex_unlock(&thread_mutex);
}

void mp_thread_mutex_init(mp_thread_mutex_t *mutex) {
    // Need a binary semaphore so a lock can be acquired on one Python thread
    // and then released on another.
    mutex->handle = xSemaphoreCreateBinaryStatic(&mutex->buffer);
    xSemaphoreGive(mutex->handle);
}

int mp_thread_mutex_lock(mp_thread_mutex_t *mutex, int wait) {
    return pdTRUE == xSemaphoreTake(mutex->handle, wait ? portMAX_DELAY : 0);
}

void mp_thread_mutex_unlock(mp_thread_mutex_t *mutex) {
    xSemaphoreGive(mutex->handle);
    // Python threads run at equal priority, so preemptively yield here to
    // prevent pathological imbalances where a thread unlocks and then
    // immediately re-locks a mutex before a context switch can occur, leaving
    // another thread waiting for an unbounded period of time.
    taskYIELD();
}

void mp_thread_deinit(void) {
    // The current task should be thread_entry0 and should be the last in the linked list.
    assert(thread_entry0.id == xTaskGetCurrentTaskHandle());
    assert(thread_entry0.next == NULL);

    // Delete all tasks except the main one.
    mp_thread_mutex_lock(&thread_mutex, 1);
    for (mp_thread_t *th = thread; th != NULL; th = th->next) {
        if (th != &thread_entry0) {
            vTaskDelete(th->id);
        }
    }
    thread = &thread_entry0;
    mp_thread_mutex_unlock(&thread_mutex);

    // Give the idle task a chance to run, to clean up any deleted tasks.
    vTaskDelay(20);
}

#endif // MICROPY_PY_THREAD
