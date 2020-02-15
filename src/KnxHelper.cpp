#include "KnxHelper.h"

static Dpt sDpt[] = {Dpt(1, 1), Dpt(2, 1), Dpt(5, 10), Dpt(5, 1), Dpt(6, 1), Dpt(7, 1), Dpt(8, 1), Dpt(9, 2), Dpt(16, 1), Dpt(17, 1), Dpt(232, 600), Dpt(10,1,1), Dpt(11,1)};

Dpt &getDPT(uint8_t iDptIndex)
{
    return sDpt[iDptIndex];
}
