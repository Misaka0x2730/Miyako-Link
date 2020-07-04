#ifndef __COMMON_H_
#define __COMMON_H_

#include "chip.h"

typedef enum {
	ERR_OK = 0,
	ERR_INV_ARG
} err_code;

#define COUNT_OF(arr)	((sizeof(arr))/(sizeof((arr)[0])))

#endif /* __COMMON_H_ */