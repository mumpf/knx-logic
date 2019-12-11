#pragma once
#include <knx.h>

// Board specific implementations

// additional LED (yellow) as debug indicator
#define LED_YELLOW_PIN             38
// Buzzer
#define BUZZER_PIN                 9
#define BUZZER_FREQ                2400

// EEPROM Support
#define I2C_EEPROM_DEVICE_ADDRESSS 0x50 // Address of 24LC256 eeprom chip
#define SAVE_BUFFER_START_PAGE     0        // All stored KO data begin at this page and takes 37 pages, 
                                        // allow 3 pages boundary, so next store should start at page 40

// NCN5130 internal registers
#define U_INT_REG_WR_REQ_WD        0x28
#define U_INT_REG_WR_REQ_ACR0      0x29
#define U_INT_REG_WR_REQ_ACR1      0x2A
#define U_INT_REG_WR_REQ_ASR0      0x2B

// NCN5130: Analog Control Register 0 - Bit values
#define ACR0_FLAG_V20VEN           0x40
#define ACR0_FLAG_DC2EN            0x20
#define ACR0_FLAG_XCLKEN           0x10
#define ACR0_FLAG_TRIGEN           0x08
#define ACR0_FLAG_V20VCLIMIT       0x04

// generic helper for formatted debug output
int printf(const char *format, ...);

// Turn off 5V rail from NCN5130 to save power for EEPROM write during knx save operation
void savePower();
// Turn on 5V rail from NCN5130 in case SAVE-Interrupt was false positive
void restorePower();

// During EEPROM Write we have to delay 5 ms
// This method ensures, that this delay works also running in interrupt
void delayEEPROMWrite(bool iIsInterrupt);
// writing KOs is a transaction with a specific begin and end state
bool beginWriteKoEEPROM(bool iIsInterrupt);
void endWriteKoEEPROM(bool iIsInterrupt);
void beginPageEEPROM(uint16_t iAddress);
void endPageEEPROM(bool iIsInterrupt);
void write4BytesEEPROM(uint8_t *iData, uint8_t iLen);
void prepareReadEEPROM(uint16_t iAddress, uint8_t iLen);
bool checkDataValidEEPROM();
void fatalError(uint8_t iErrorCode, const char *iErrorText = 0);
bool delayCheck(uint32_t iOldTimer, uint32_t iDuration);
void writeMagicWordToEEPROM(uint16_t iAddress, bool iIsInterrupt);
bool checkMagicWordInEEPROM(uint16_t iAddress);
