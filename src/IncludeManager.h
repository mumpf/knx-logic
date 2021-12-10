#pragma once
#define STR(x) #x
#define XSTR(x) STR(x)

#ifdef LOGICMODULE
#pragma message "Building Logic for LOGICMODULE"
#include "Logikmodul.h"
#elif SENSORMODULE
#pragma message "Building Logic for SENSORMODULE"
#include "../../knx-sensor/src/Sensormodul.h"
#elif PMMODULE
#pragma message "Building Logic for PMMODULE"
#include "../../knx-pm/src/PMmodul.h"
#elif WIREGATEWAY
#pragma message "Building Logic for WIREGATEWAY"
#include "../../knx-wire/src/WireGateway.h"
#elif TEST
#include "../../knx-test/src/Test.h"
#pragma message "Building Logic for TEST"
#endif

