#ifndef __MUX_TASK_H
#define __MUX_TASK_H

#include "periph_conf.h"

#define MUX_TASK_EVENT_UPDATE        (0x01)

void mux_task_init(void);
void mux_pin_set(platform_pin_t pin, bool value);
bool mux_pin_get(platform_pin_t pin);

#endif // __MUX_TASK_H