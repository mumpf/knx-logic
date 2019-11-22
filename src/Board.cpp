#include <stdarg.h>
#include <Wire.h>
#include "Board.h"

// generic helper for formatted debug output
int printf(const char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    int lResult = vsnprintf(buffer, 256, format, args);
    //   perror (buffer);
    va_end(args);
    println(buffer);
    return lResult;
}

void savePower() {
    println("savePower: Switching off all energy hungry devices...");
    // turn off 5V rail (CO2-Sensor & Buzzer)
    uint8_t lBuffer[] = {U_INT_REG_WR_REQ_ACR0, ACR0_FLAG_XCLKEN | ACR0_FLAG_V20VCLIMIT };
    knx.platform().writeUart(lBuffer, 2);
    // Turn off on board leds
    digitalWrite(knx.ledPin(), HIGH - knx.ledPinActiveOn());
    digitalWrite(LED_YELLOW_PIN, LOW);
    // in future: Turn off i2c leds
    // in future: Turn off i2c 1Wire if possible
}

// During EEPROM Write we have to delay 5 ms
// This method ensures, that this delay works also running in interrupt
void delayEEPROMWrite(bool iIsInterrupt) {
    if (iIsInterrupt) 
        delayMicroseconds(5000); // in interrupt handler delay() does not work
    else 
        delay(5); // during debugging, delayMicroseconds does not work :-(
}

const uint8_t gMagicWord[] = {0xAE, 0x49, 0xD2, 0x9F};
const uint8_t gFiller[] = {0, 0, 0, 0};

bool beginWriteKoEEPROM(bool iIsInterrupt) {
    uint16_t lAddress = SAVE_BUFFER_START_PAGE * 32 + 12;
    bool lResult = false;
    // first we delete magic word, it is rewritten at the end. This is the ack for the successful write.
    Wire.beginTransmission(I2C_EEPROM_DEVICE_ADDRESSS);
    Wire.write((uint8_t)((lAddress) >> 8)); // MSB
    Wire.write((uint8_t)((lAddress)&0xFF)); // LSB
    Wire.write(gFiller, 4);
    lResult = Wire.endTransmission();
    if (lResult) delayEEPROMWrite(iIsInterrupt);
    return lResult;
}

void endWriteKoEEPROM(bool iIsInterrupt) {
    // as a last step we write magic number back
    // this is also the ACK, that writing was successfull
    uint16_t lAddress = SAVE_BUFFER_START_PAGE * 32 + 12;
    Wire.beginTransmission(I2C_EEPROM_DEVICE_ADDRESSS);
    Wire.write((uint8_t)((lAddress) >> 8)); // MSB
    Wire.write((uint8_t)((lAddress)&0xFF)); // LSB
    Wire.write(gMagicWord, 4);
    Wire.endTransmission();
    delayEEPROMWrite(iIsInterrupt);
}

bool checkDataValidEEPROM() {
    uint16_t lAddress = SAVE_BUFFER_START_PAGE * 32 + 12;
    prepareReadEEPROM(lAddress, 4);
    int lIndex = 0;
    bool lResult = true;
    while (lResult && lIndex < 4) 
        lResult = Wire.available() && (gMagicWord[lIndex++] == Wire.read());
    return lResult;
}

bool gIsTransmission = false;

void beginPageEEPROM(uint16_t iAddress) {
    if (!gIsTransmission)
    {
        Wire.beginTransmission(I2C_EEPROM_DEVICE_ADDRESSS);
        Wire.write((uint8_t)((iAddress) >> 8)); // MSB
        Wire.write((uint8_t)((iAddress)&0xFF)); // LSB
        gIsTransmission = true;
    }
}

void endPageEEPROM(bool iIsInterrupt) {
    if (gIsTransmission)
    {
        gIsTransmission = false;
        Wire.endTransmission();
        delayEEPROMWrite(iIsInterrupt);
    }
}

void write4BytesEEPROM(uint8_t* iData, uint8_t iLen) {
    Wire.write(iData, iLen);
    if (iLen < 4)
        Wire.write(gFiller, 4 - iLen);
}

void prepareReadEEPROM(uint16_t iAddress, uint8_t iLen) {
    Wire.beginTransmission(I2C_EEPROM_DEVICE_ADDRESSS);
    Wire.write((uint8_t)((iAddress) >> 8)); // MSB
    Wire.write((uint8_t)((iAddress)&0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(I2C_EEPROM_DEVICE_ADDRESSS, iLen);
}