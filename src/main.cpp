#include <knx.h>
#include "Hardware.h"

void appSetup(bool iSaveSupported);
void appLoop();

void setup()
{
    SerialUSB.begin(115200);
    pinMode(PROG_LED_PIN, OUTPUT);
    digitalWrite(PROG_LED_PIN, HIGH);
    delay(DEBUG_DELAY);
    digitalWrite(PROG_LED_PIN, LOW);
    SerialUSB.println("Startup called...");
    ArduinoPlatform::SerialDebug = &SerialUSB;

#ifdef INFO_LED_PIN
    pinMode(INFO_LED_PIN, OUTPUT);
    ledInfo(true);
#endif

    // Wire.begin();
    knx.readMemory();

    // pin or GPIO the programming led is connected to. Default is LED_BUILDIN
    knx.ledPin(PROG_LED_PIN);
    // is the led active on HIGH or low? Default is LOW
    knx.ledPinActiveOn(PROG_LED_PIN_ACTIVE_ON);
    // pin or GPIO programming button is connected to. Default is 0
    knx.buttonPin(PROG_BUTTON_PIN);
    // Is the interrup created in RISING or FALLING signal? Default is RISING
    knx.buttonPinInterruptOn(PROG_BUTTON_PIN_INTERRUPT_ON);

    // print values of parameters if device is already configured
    if (knx.configured())
        appSetup(true);

    // start the framework.
    knx.start();
    ledInfo(false);
}

void loop()
{
    // don't delay here to much. Otherwise you might lose packages or mess up the timing with ETS
    knx.loop();

    // only run the application code if the device was configured with ETS
    if (knx.configured())
        appLoop();
}