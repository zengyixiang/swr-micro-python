#ifndef __MPHAIPORT_H__
#define __MPHAIPORT_H__

#include "py/ringbuf.h"

extern ringbuf_t stdin_ringbuf;

void mp_hal_set_interrupt_char(int c); 

#endif //__MPHAIPORT_H__
