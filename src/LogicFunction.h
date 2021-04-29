#pragma once

#include <inttypes.h>

#define NUM_NATIVE_FUNCTIONS 7

class LogicFunction
{
  private:
    LogicFunction(/* args */);
    ~LogicFunction();

    static uint32_t (*nativeFunction[NUM_NATIVE_FUNCTIONS])(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t (*userFunction[30])(uint32_t E1, uint32_t E2, uint8_t *Dpt);

    // implemented native functions, as an simple example
    static uint32_t nativeAdd(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t nativeSubtract(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t nativeMultiply(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t nativeDivide(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t nativeAverage(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t nativeMinimum(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t nativeMaximum(uint32_t E1, uint32_t E2, uint8_t *Dpt);

    // user functions (empty, implemented by user)
    static uint32_t userFunction01(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction02(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction03(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction04(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction05(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction06(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction07(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction08(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction09(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction10(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction11(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction12(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction13(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction14(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction15(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction16(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction17(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction18(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction19(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction20(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction21(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction22(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction23(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction24(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction25(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction26(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction27(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction28(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction29(uint32_t E1, uint32_t E2, uint8_t *Dpt);
    static uint32_t userFunction30(uint32_t E1, uint32_t E2, uint8_t *Dpt);

  public:
    static uint32_t callFunction(uint8_t iId, uint32_t iE1, uint32_t iE2, uint8_t *cDpt);
};
