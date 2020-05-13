#pragma once
#define STR(x) #x
#define XSTR(x) STR(x)

#ifdef LOGICMODULE
#define MODULE "LOGICMODULE"
#include "Logikmodul.h"
#elif SENSORMODULE
#define MODULE "SENSORMODULE"
#include "../../knx-sensor/src/Sensormodul.h"
#elif PMMODULE
#define MODULE "PMMODULE"
#include "../../knx-pm/src/PMmodul.h"
#elif WIREGATEWAY
#define MODULE "WIREGATEWAY"
#include "../../knx-wire/src/WireGateway.h"
#elif TEST
#include "../../knx-test/src/Test.h"
#define MODULE "TEST"
#endif

// this processes a warning during compilation!
// this warning is intended
// the output tells you, for which module the logic is compiled
// to be exact: which include with ets definitions is taken to compile the logic
#pragma message "Building Logic for " MODULE
