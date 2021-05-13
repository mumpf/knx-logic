#pragma once

#include <inttypes.h>

#define NUM_NATIVE_FUNCTIONS 7

class LogicFunction
{
  private:
    LogicFunction(/* args */);
    ~LogicFunction();

    static uint32_t (*nativeFunction[NUM_NATIVE_FUNCTIONS])(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t (*userFunction[30])(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);

    // implemented native functions, as an simple example
    static uint32_t nativeAdd(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t nativeSubtract(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t nativeMultiply(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t nativeDivide(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t nativeAverage(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t nativeMinimum(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t nativeMaximum(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);

    // user functions (empty, implemented by user)
    static uint32_t userFunction01(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction02(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction03(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction04(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction05(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction06(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction07(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction08(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction09(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction10(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction11(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction12(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction13(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction14(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction15(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction16(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction17(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction18(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction19(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction20(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction21(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction22(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction23(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction24(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction25(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction26(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction27(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction28(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction29(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);
    static uint32_t userFunction30(uint8_t DptE1, uint32_t E1, uint8_t DptE2, uint32_t E2, uint8_t *DptOut);

  public:
    static uint32_t callFunction(uint8_t iId, uint8_t iDptE1, uint32_t iE1, uint8_t iDptE2, uint32_t iE2, uint8_t *cDptOut);
};
