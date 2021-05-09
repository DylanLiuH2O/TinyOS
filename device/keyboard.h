#ifndef __DEVICE_KEYBOARD_H
#define __DEVICE_KEYBOARD_H

#include "device/ioqueue.h"

extern struct ioqueue kb_buf;

void keyboard_init(void);

#endif