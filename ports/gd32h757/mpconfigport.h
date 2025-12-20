
#include <stdint.h>
#include "export_main.h"

#define MICROPY_CONFIG_ROM_LEVEL                (MICROPY_CONFIG_ROM_LEVEL_EVERYTHING)

#define MICROPY_HW_BOARD_NAME       "swr gd32H7"
#define MICROPY_HW_MCU_NAME         "GD32H757"
#define MICROPY_PY_SYS_PLATFORM "GD32H757"
// #define MICROPY_HW_FLASH_FS_LABEL   "Portenta H7"

// Compiler configuration
#define MICROPY_ENABLE_COMPILER                 (1)

// Python internal features
#define MICROPY_ERROR_REPORTING                 (MICROPY_ERROR_REPORTING_DETAILED)

#define	DISK_NAME		"0:"
// #define SYS_SENSORS_NUM                     19
// Fine control over Python builtins, classes, modules, etc.
#define MICROPY_PY_SYS                          (1)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_KBD_EXCEPTION       (1)
#define MICROPY_VFS                 (1)
#define MICROPY_READER_VFS          (1)
#define MICROPY_VFS_FAT             (1)
#define MICROPY_ALLOC_PATH_MAX              (128)
#define MICROPY_STACK_CHECK         (1)
#define MICROPY_STACK_CHECK_MARGIN  (1024)
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF (1)
// emitters
#define MICROPY_PERSISTENT_CODE_LOAD (1)
#ifndef MICROPY_EMIT_THUMB
#define MICROPY_EMIT_THUMB          (1)
#endif
#ifndef MICROPY_EMIT_INLINE_THUMB
#define MICROPY_EMIT_INLINE_THUMB   (1)
#endif
#define MICROPY_OPT_COMPUTED_GOTO           (1)
#define MICROPY_LONGINT_IMPL                (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL                  (MICROPY_FLOAT_IMPL_FLOAT)
#define MP_SSIZE_MAX                        (0x7fffffff)
#define MP_STATE_PORT MP_STATE_VM
#define MICROPY_STREAMS_POSIX_API           (1)
#define MICROPY_USE_INTERNAL_PRINTF         (0) 
#define MICROPY_SCHEDULER_DEPTH             (8)
#define MICROPY_SCHEDULER_STATIC_NODES      (1)
#define MICROPY_PY_TIME_GMTIME_LOCALTIME_MKTIME (1)
#define MICROPY_PY_TIME_TIME_TIME_NS        (1)
#define MICROPY_PY_TIME_INCLUDEFILE         "ports/gd32h757/modtime.c"
#define MICROPY_PY_THREAD                   (1)
#define MICROPY_PY_THREAD_GIL               (1)
#define MICROPY_PY_THREAD_GIL_VM_DIVISOR    (32)
typedef long mp_off_t;

extern uint32_t get_random_data();
#define MICROPY_PY_RANDOM_SEED_INIT_FUNC    (get_random_data())
#define MICROPY_PY_OS_INCLUDEFILE           "ports/gd32h757/modos.c"
#define MICROPY_PY_OS_DUPTERM               (1)
#define MICROPY_PY_OS_DUPTERM_NOTIFY        (1)
#define MICROPY_PY_OS_SYNC                  (1)
#define MICROPY_PY_OS_UNAME                 (1)
#define MICROPY_PY_OS_URANDOM               (1)


#if MICROPY_PY_THREAD
#define MICROPY_EVENT_POLL_HOOK \
    do { \
        extern void mp_handle_pending(bool); \
        mp_handle_pending(true); \
        MP_THREAD_GIL_EXIT(); \
        ulTaskNotifyTake(pdFALSE, 1); \
        MP_THREAD_GIL_ENTER(); \
    } while (0);
#endif
// Need to provide a declaration/definition of alloca()
#include <alloca.h>
