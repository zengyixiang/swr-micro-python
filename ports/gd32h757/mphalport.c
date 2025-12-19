#include <string.h>

#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "extmod/misc.h"
#include "export_main.h"
#include <sys/time.h>
MP_WEAK uintptr_t mp_hal_stdio_poll(uintptr_t poll_flags) {

    return 0;
}
MP_WEAK int mp_hal_stdin_rx_chr(void) {
    for (;;) {
        return uart5_recv_char();
        // if (MP_STATE_PORT(pyb_stdio_uart) != NULL && uart_rx_any(MP_STATE_PORT(pyb_stdio_uart))) {
        //     return uart_rx_char(MP_STATE_PORT(pyb_stdio_uart));
        // }
        // int dupterm_c = mp_os_dupterm_rx_chr();
        // if (dupterm_c >= 0) {
        //     return dupterm_c;
        // }

        // MICROPY_EVENT_POLL_HOOK
    }
}

MP_WEAK mp_uint_t mp_hal_stdout_tx_strn(const char *str, size_t len) {
    mp_uint_t ret = len;
    bool did_write = false;
#if 1
    for(int i = 0;i < len; i++){
        uart5_write_char(str[i]);
    }
    did_write = true;
#else
    if (MP_STATE_PORT(pyb_stdio_uart) != NULL) {
        uart_tx_strn(MP_STATE_PORT(pyb_stdio_uart), str, len);
        did_write = true;
    }
#endif
    int dupterm_res = mp_os_dupterm_tx_strn(str, len);
    if (dupterm_res >= 0) {
        did_write = true;
        ret = MIN((mp_uint_t)dupterm_res, ret);
    }
    return did_write ? ret : 0;
}

uint64_t mp_hal_time_ns(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t ns = tv.tv_sec * 1000000000ULL;
    ns += (uint64_t)tv.tv_usec * 1000ULL;
    return ns;
}
