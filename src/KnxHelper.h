#pragma once

#include <knx.h>

// enum supported dpt
#define VAL_DPT_1 0
#define VAL_DPT_2 1
#define VAL_DPT_5 2
#define VAL_DPT_5001 3
#define VAL_DPT_6 4
#define VAL_DPT_7 5
#define VAL_DPT_8 6
#define VAL_DPT_9 7
#define VAL_DPT_16 8
#define VAL_DPT_17 9
#define VAL_DPT_232 10

Dpt &getDPT(uint8_t iDptIndex);
