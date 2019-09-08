#include <knx.h>

#ifdef ARDUINO_ARCH_ESP8266
#include <WiFiManager.h>
#endif

// ################################################
// ### IO Configuration
// ################################################
#define PROG_LED_PIN 13
#define PROG_BUTTON_PIN 11
#define LED_YELLOW 38

void appSetup();
void appLoop();

void setup() {
    SerialDBG.begin(115200);
    SerialDBG.println("Startup called...");

#ifdef ARDUINO_ARCH_ESP8266
    WiFiManager wifiManager;
    wifiManager.autoConnect("knx-logik");
#endif

    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();

    // // pin or GPIO the programming led is connected to. Default is LED_BUILDIN
    // knx.ledPin(26);
    // // is the led active on HIGH or low? Default is LOW
    // // knx.ledPinActiveOn(HIGH);
    // // pin or GPIO programming button is connected to. Default is 0
    // knx.buttonPin(10);
    // // Is the interrup created in RISING or FALLING signal? Default is RISING
    // // knx.buttonPinInterruptOn(FALLING);

    // pin or GPIO the programming led is connected to. Default is LED_BUILDIN
    knx.ledPin(PROG_LED_PIN);
    // is the led active on HIGH or low? Default is LOW
    knx.ledPinActiveOn(HIGH);
    // pin or GPIO programming button is connected to. Default is 0
    knx.buttonPin(PROG_BUTTON_PIN);
    // Is the interrup created in RISING or FALLING signal? Default is RISING
    knx.buttonPinInterruptOn(FALLING);

    // print values of parameters if device is already configured
    if (knx.configured()) {
        appSetup();
    }

    // start the framework.
    knx.start();
}

void loop() {
    // don't delay here to much. Otherwise you might lose packages or mess up the timing with ETS
    knx.loop();

    // only run the application code if the device was configured with ETS
    if (knx.configured())
        appLoop();
}