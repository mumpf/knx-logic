#pragma once
#include <knx_facade.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <Wire.h>

#define STR( x ) #x
#define XSTR( x ) STR( x )

#ifdef LOGICMODULE
#define MODULE "LOGICMODULE"
#include "Logikmodul.h"
#elif SENSORMODULE
#define MODULE "SENSORMODULE"
#include "../../knx-sensor/src/Sensormodul.h"
#elif TEST
#include "../../knx-test/src/Test.h"
#define MODULE "TEST"
#endif
#include "KnxHelper.h"
#include "EepromManager.h"

#pragma message "Building Logic for " MODULE

#define SAVE_BUFFER_START_PAGE 0 // All stored KO data begin at this page and takes 37 pages,
#define SAVE_BUFFER_NUM_PAGES 40 // allow 3 pages boundary, so next store should start at page 40

#define LOG_ChannelsFirmware 50

// enum input defaults
#define VAL_InputDefault_Undefined 0
#define VAL_InputDefault_Read 1
#define VAL_InputDefault_False 2
#define VAL_InputDefault_True 3
#define VAL_InputDefault_EEPROM 4

// enum input converter
#define VAL_InputConvert_Interval 0
#define VAL_InputConvert_DeltaInterval 1
#define VAL_InputConvert_Hysterese 2
#define VAL_InputConvert_DeltaHysterese 3

// enum logical function
#define VAL_Logic_And 1
#define VAL_Logic_Or 2
#define VAL_Logic_ExOr 3
#define VAL_Logic_Gate 4

// enum delay extend
#define VAL_Delay_Nothing 0
#define VAL_Delay_Extend 1
#define VAL_Delay_Immediate 2

// enum delay reset
#define VAL_Reset_Nothing 0
#define VAL_Reset_Reset 1

// enum output values
#define VAL_Out_No 0
#define VAL_Out_Constant 1
#define VAL_Out_ValE1 2
#define VAL_Out_ValE2 3
#define VAL_Out_ReadRequest 4
#define VAL_Out_ResetDevice 5
#define VAL_Out_Buzzer 6

// enum output filter
#define VAL_AllowRepeat_All 0
#define VAL_AllowRepeat_On 1
#define VAL_AllowRepeat_Off 2
#define VAL_AllowRepeat_None 3

// flags for in- and output
#define BIT_EXT_INPUT_1 1
#define BIT_EXT_INPUT_2 2
#define BIT_INT_INPUT_1 4
#define BIT_INT_INPUT_2 8
#define BIT_INPUT_MASK 15
#define BIT_OUTPUT 16
#define BIT_LAST_OUTPUT 128

// enum fo IOIndex
#define IO_Absolute 0
#define IO_Input1 BIT_EXT_INPUT_1 // 1
#define IO_Input2 BIT_EXT_INPUT_2 // 2
#define IO_Output 3

// pipeline steps
#define PIP_STARTUP 1              // startup delay for each channel
#define PIP_REPEAT_INPUT1 2        // send read requests for input 1
#define PIP_REPEAT_INPUT2 4        // send read requests for input 2
#define PIP_CONVERT_INPUT1 8       // convert input value 1 to bool
#define PIP_CONVERT_INPUT2 16      // convert input value 2 to bool
#define PIP_LOGIC_EXECUTE 32       // do logical step
#define PIP_STAIRLIGHT 64          // do stairlight delay
#define PIP_BLINK 128              // do blinking during stairlight
#define PIP_ON_DELAY 256           // delay on signal
#define PIP_OFF_DELAY 512          // delay off signal
#define PIP_OUTPUT_FILTER_ON 1024  // Filter repeated signals
#define PIP_OUTPUT_FILTER_OFF 2048 // Filter repeated signals
#define PIP_ON_REPEAT 4096         // repeat on signal
#define PIP_OFF_REPEAT 8192        // repeat off signal

#ifdef __linux__
extern KnxFacade<LinuxPlatform, Bau57B0> knx;
#endif

const uint32_t cTimeFactors[] = {100, 1000, 60000, 3600000};

class Logic;

class LogicChannel
{
  private:
    // static
    // instance
    uint8_t mChannelId;
    uint32_t calcParamIndex(uint16_t iParamIndex);
    uint16_t calcKoNumber(uint8_t iIOIndex);
    float getFloat(uint8_t *data);
    uint8_t getByteParam(uint16_t iParamIndex);
    int8_t getSByteParam(uint16_t iParamIndex);
    uint16_t getWordParam(uint16_t iParamIndex);
    int16_t getSWordParam(uint16_t iParamIndex);
    uint32_t getIntParam(uint16_t iParamIndex);
    int32_t getSIntParam(uint16_t iParamIndex);
    float getFloatParam(uint16_t iParamIndex);
    uint8_t* getStringParam(uint16_t iParamIndex);
    GroupObject *getKo(uint8_t iIOIndex);
    Dpt &getKoDPT(uint8_t iIOIndex);
    void knxWriteBool(uint8_t iIOIndex, bool iValue);
    void knxWriteInt(uint8_t iIOIndex, int32_t iValue);
    void knxWriteRawInt(uint8_t iIOIndex, int32_t iValue);
    void knxWriteFloat(uint8_t iIOIndex, float iValue);
    void knxWriteString(uint8_t iIOIndex, char* iValue);
    void knxRead(uint8_t iIOIndex);
    void knxResetDevice(uint16_t iParamIndex);
    int32_t getParamForDelta(uint8_t iDpt, uint16_t iParamIndex);
    int32_t getParamByDpt(uint8_t iDpt, uint16_t iParamIndex);
    int32_t getInputValue(uint8_t iIOIndex);
    void writeConstantValue(uint16_t iParamIndex);
    void writeParameterValue(uint8_t iIOIndex);

    bool isInputActive(uint8_t iIOIndex);

    void startStartup();
    void processStartup();
    void processRepeatInput1();
    void processRepeatInput2();
    void startConvert(uint8_t iIOIndex);
    void processConvertInput(uint8_t iIOIndex);
    void startLogic(uint8_t iIOIndex, bool iValue);
    void processLogic();
    void startStairlight(bool iOutput);
    void processStairlight();
    void startBlink();
    void processBlink();
    void startOnDelay();
    void processOnDelay();
    void startOffDelay();
    void processOffDelay();
    void startOutputFilter(bool iOutput);
    void processOutputFilter();
    void startOnOffRepeat(bool iOutput);
    void processOnOffRepeat();

    void processOutput(bool iValue);

    bool readOneInputFromEEPROM(uint8_t iIOIndex);
    
  protected:

    // static

    // instance
    /* Runtime information per channel */
    uint8_t pTriggerIO;        // Bitfield: Which input (0-3) triggered processing, output (4) is triggering further processing
    uint8_t pValidActiveIO;    // Bitfield: validity flags for input (0-3) values and active inputs (4-7)
    uint8_t pCurrentIO;        // Bitfield: current input (0-3) and current output (4) and last (previous) output (7) values
    uint16_t pCurrentPipeline; // Bitfield: indicator for current pipeline step
    uint32_t pRepeatInput1Delay;
    uint32_t pRepeatInput2Delay;
    uint32_t pStairlightDelay;
    uint32_t pBlinkDelay;
    uint32_t pOnDelay;
    uint32_t pOffDelay;
    uint32_t pRepeatOnOffDelay;

  public:
    // Constructors
    LogicChannel(uint8_t iChannelNumber);
    ~LogicChannel();

    // static
    static Logic *sLogic;
    static uint16_t calcKoNumber(uint8_t iIOIndex, uint8_t iChannelId);
    static GroupObject *getKoForChannel(uint8_t iIOIndex, uint8_t iChannelId);

    // instance
    bool checkDpt(uint8_t iIOIndex, uint8_t iDpt);
    void processInput(uint8_t iIOIndex);
    void processInternalInputs(uint8_t iChannelId, bool iValue);
    void writeSingleDptToEEPROM(uint8_t iIOIndex);
    
    bool prepareChannel();
    void loop();
};

