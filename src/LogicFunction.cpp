#include "LogicFunction.h"

// native functions, implemented as a simple example how to use user functions
uint32_t LogicFunction::nativeAdd(uint32_t E1, uint32_t E2, uint8_t *Dpt)
{
    return E1 + E2;
}

uint32_t LogicFunction::nativeSubtract(uint32_t E1, uint32_t E2, uint8_t *Dpt)
{
    return E1 - E2;
}

uint32_t LogicFunction::nativeMultiply(uint32_t E1, uint32_t E2, uint8_t *Dpt)
{
    return E1 * E2;
}

uint32_t LogicFunction::nativeDivide(uint32_t E1, uint32_t E2, uint8_t *Dpt)
{
    return E1 / E2;
}

uint32_t LogicFunction::nativeAverage(uint32_t E1, uint32_t E2, uint8_t *Dpt)
{
    return (E1 + E2) / 2;
}

uint32_t LogicFunction::nativeMinimum(uint32_t E1, uint32_t E2, uint8_t *Dpt)
{
    return (E1 < E2) ? E1 : E2;
}

uint32_t LogicFunction::nativeMaximum(uint32_t E1, uint32_t E2, uint8_t *Dpt)
{
    return (E1 > E2) ? E1 : E2;
}

// do not touch after this point

LogicFunction::LogicFunction(){};

LogicFunction::~LogicFunction(){};

uint32_t (*LogicFunction::nativeFunction[NUM_NATIVE_FUNCTIONS])(uint32_t, uint32_t, uint8_t *){
    nativeAdd,
    nativeSubtract,
    nativeMultiply,
    nativeDivide,
    nativeAverage,
    nativeMinimum,
    nativeMaximum};

uint32_t (*LogicFunction::userFunction[30])(uint32_t, uint32_t, uint8_t*){
    userFunction01,
    userFunction02,
    userFunction03,
    userFunction04,
    userFunction05,
    userFunction06,
    userFunction07,
    userFunction08,
    userFunction09,
    userFunction10,
    userFunction11,
    userFunction12,
    userFunction13,
    userFunction14,
    userFunction15,
    userFunction16,
    userFunction17,
    userFunction18,
    userFunction19,
    userFunction20,
    userFunction21,
    userFunction22,
    userFunction23,
    userFunction24,
    userFunction25,
    userFunction26,
    userFunction27,
    userFunction28,
    userFunction29,
    userFunction30};

// dispatcher
uint32_t LogicFunction::callFunction(uint8_t iId, uint32_t iE1, uint32_t iE2, uint8_t *cDpt) {
    if (iId > 0 && iId <= NUM_NATIVE_FUNCTIONS)
    {
        return nativeFunction[iId - 1](iE1, iE2, cDpt);
    }
    else if (iId > 200 && iId <= 230)
    {
        return userFunction[iId - 201](iE1, iE2, cDpt);
    }
    return 0.0;
}