// #include <knx/bits.h>
#include <knx_facade.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <Wire.h>

#ifndef LOG_Channels
#include "Logikmodul.h"
#endif
#include "Board.h"
#include "LogikmodulCommon.h"

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

uint32_t sTimeFactors[] = {100, 1000, 60000, 3600000};

struct sChannelInfo
{
    /* Runtime information per channel */
    uint8_t triggerIO;        // Bitfield: Which input (0-3) triggered processing, output
                              // (4) is triggering further processing
    uint8_t validActiveIO;    // Bitfield: validity flags for input (0-3) values and
                              // active inputs (4-7)
    uint8_t currentIO;        // Bitfield: current input (0-3) and current output (4) and last (previous) output (7) values
    uint16_t currentPipeline; // Bitfield: indicator for current pipeline step
    uint32_t repeatInput1Delay;
    uint32_t repeatInput2Delay;
    uint32_t stairlightDelay;
    uint32_t blinkDelay;
    uint32_t onDelay;
    uint32_t offDelay;
    uint32_t repeatOnOffDelay;
};

uint8_t gNumChannels;
sChannelInfo gChannelData[LOG_ChannelsFirmware];
uint8_t gBuzzerPin = 0;

// forward declaratins
void StartLogic(sChannelInfo *cData, uint8_t iChannel, uint8_t iIOIndex, bool iValue);
bool readOneInputFromEEPROM(uint8_t iIOIndex, uint8_t iChannel);
void writeAllDptToEEPROM();

void DbgWrite(const char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    //   perror (buffer);
    va_end(args);
    println(buffer);
}

voidFuncPtr gSaveInterruptCallback = 0;

voidFuncPtr getSaveInterruptCallback() {
    return gSaveInterruptCallback;
}

// encapsulate attachInterrupt to be able to retrieve interrupt callback
void logicAttachSaveInterrupt(uint32_t iPin, voidFuncPtr iCallback, uint32_t iMode) {
    gSaveInterruptCallback = iCallback;
    attachInterrupt(iPin, iCallback, iMode);
}

uint32_t calcParamIndex(uint16_t iParamIndex, int8_t iChannel)
{
    uint32_t lResult = iParamIndex + ((iChannel < 0) ? 0 : iChannel * LOG_ParamBlockSize + LOG_ParamBlockOffset);
    return lResult;
}

uint8_t getByteParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
    return knx.paramByte(calcParamIndex(iParamIndex, iChannel));
}

int8_t getSByteParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
    uint8_t *lRef = knx.paramData(calcParamIndex(iParamIndex, iChannel));
    return lRef[0];
}

uint16_t getWordParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
    return knx.paramWord(calcParamIndex(iParamIndex, iChannel));
}

int16_t getSWordParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
    uint8_t *lRef = knx.paramData(calcParamIndex(iParamIndex, iChannel));
    return lRef[0] * 256 + lRef[1];
}

uint32_t getIntParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
    return knx.paramInt(calcParamIndex(iParamIndex, iChannel));
}

int32_t getSIntParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
    return knx.paramInt(calcParamIndex(iParamIndex, iChannel));
}

float getFloat(uint8_t *data)
{

    union Float
    {
        float lFloat;
        uint8_t lBytes[sizeof(float)];
    };

    Float myFloat;

    myFloat.lBytes[0] = data[3];
    myFloat.lBytes[1] = data[2];
    myFloat.lBytes[2] = data[1];
    myFloat.lBytes[3] = data[0];
    return myFloat.lFloat;
}

float getFloatParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
    uint16_t lIndex = calcParamIndex(iParamIndex, iChannel);
    float lFloat = getFloat(knx.paramData(lIndex));
    return lFloat;
}

uint8_t *getStringParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
    uint16_t lIndex = calcParamIndex(iParamIndex, iChannel);
    return knx.paramData(lIndex);
}

// if IOIndex = 0, iChannel is the absolute KO number 8starting with 1)
// if IOIndex > 0, iChannel is channel number and IOIndex specifies Input1, Input2 or Output
uint16_t calcKoNumber(uint8_t iIOIndex, uint8_t iChannel)
{
    // int lIndex = (iIOIndex == 0) ? 3 : iIOIndex;
    // return KO_ChannelOffset - 1 + lIndex + iChannel * 3;
    uint16_t lIndex = (iIOIndex) ? LOG_KoOffset - 1 + iIOIndex + iChannel * LOG_KoBlockSize : iChannel;
    return lIndex;
}

/* calculates correct KO for given I/O index and Channel
 * iIOIndex - 0=Output, 1=External input 1, 2=External input 2
 */
GroupObject *getKoForChannel(uint8_t iIOIndex, uint8_t iChannel)
{
    return &knx.getGroupObject(calcKoNumber(iIOIndex, iChannel));
}

Dpt &getDPT(uint8_t iIOIndex, uint8_t iChannel)
{
    uint16_t lParamDpt;
    switch (iIOIndex)
    {
        case IO_Input1:
            lParamDpt = LOG_fE1Dpt;
            break;
        case IO_Input2:
            lParamDpt = LOG_fE2Dpt;
            break;
        case IO_Output:
            lParamDpt = LOG_fODpt;
            break;
        default:
            lParamDpt = iChannel;
            iChannel = 0;
            break;
    }
    uint8_t lDpt = getByteParam(lParamDpt, iChannel);
    return getDPT(lDpt);
}

// write value to bus
void knxWriteBool(uint8_t iIOIndex, uint8_t iChannel, bool iValue)
{
    DbgWrite("knxWrite KO %d bool value %d", calcKoNumber(iIOIndex, iChannel), iValue);
    getKoForChannel(iIOIndex, iChannel)->value(iValue, getDPT(iIOIndex, iChannel));
}

void knxWriteInt(uint8_t iIOIndex, uint8_t iChannel, int32_t iValue)
{
    DbgWrite("knxWrite KO %d int value %d", calcKoNumber(iIOIndex, iChannel), iValue);
    getKoForChannel(iIOIndex, iChannel)->value((int32_t)iValue, getDPT(iIOIndex, iChannel));
}

void knxWriteRawInt(uint8_t iIOIndex, uint8_t iChannel, int32_t iValue)
{
    DbgWrite("knxWrite KO %d int value %d", calcKoNumber(iIOIndex, iChannel), iValue);
    GroupObject *lKo = getKoForChannel(iIOIndex, iChannel);
    uint8_t *lValueRef = lKo->valueRef();
    *lValueRef = iValue;
    lKo->objectWritten();
}

void knxWriteFloat(uint8_t iIOIndex, uint8_t iChannel, float iValue)
{
    DbgWrite("knxWrite KO %d float value %f", calcKoNumber(iIOIndex, iChannel), iValue);
    getKoForChannel(iIOIndex, iChannel)->value(iValue, getDPT(iIOIndex, iChannel));
}

void knxWriteString(uint8_t iIOIndex, uint8_t iChannel, char *iValue)
{
    DbgWrite("knxWrite KO %d string value %s", calcKoNumber(iIOIndex, iChannel), iValue);
    getKoForChannel(iIOIndex, iChannel)->value(iValue, getDPT(iIOIndex, iChannel));
}

// send read request on bus
void knxRead(uint8_t iIOIndex, uint8_t iChannel)
{
    DbgWrite("knxReadRequest end from KO %d", calcKoNumber(iIOIndex, iChannel));
    getKoForChannel(iIOIndex, iChannel)->requestObjectRead();
}

// send reset device to bus
void knxResetDevice(uint16_t iParamIndex, uint8_t iChannel)
{
    uint16_t lAddress = getWordParam(iParamIndex, iChannel);
    uint8_t lHigh = lAddress >> 8;
    DbgWrite("knxResetDevice with PA %d.%d.%d", lHigh >> 4, lHigh & 0xF, lAddress & 0xFF);
    knx.restart(lAddress);
    // tmp: on restart we also call SAVE-Interrupt
    if ((lHigh >= 32) && getSaveInterruptCallback() != 0) 
        (getSaveInterruptCallback())();
}

/********************
 *
 * Tools and support functions
 *
 * ******************/

// we get an dpt dependant parameter value for difference
// input evaluation
// here special handling is necessary because of transport
// of dpt 9 as an int * 100
int32_t getParamForDelta(int8_t iDpt, uint8_t iParam, uint8_t iChannel)
{

    int32_t lValue;
    if (iDpt == VAL_DPT_9)
    {
        lValue = getFloatParam(iParam, iChannel) * 100.0;
    }
    else
    {
        lValue = (int32_t)getIntParam(iParam, iChannel);
    }
    return lValue;
}

// we get here numeric params by their DPT
// DPT1,2,5,6,7,8,17,232 => straight forward int vaues
// DPT2,17 => straight forward byte values
// DPT5001 => scale down to [0..100]
// DPT9 => transport as 1/100, means take int(float * 100)
int32_t getParamByDpt(int8_t iDpt, uint8_t iParam, uint8_t iChannel)
{
    int32_t lValue = 0;
    switch (iDpt)
    {
        case VAL_DPT_1:
            lValue = getByteParam(iParam, iChannel) != 0;
            break;
        case VAL_DPT_2:
        case VAL_DPT_5:
        case VAL_DPT_17:
        case VAL_DPT_5001:
            lValue = getByteParam(iParam, iChannel);
            break;
        case VAL_DPT_6:
            lValue = getSByteParam(iParam, iChannel);
            break;
        case VAL_DPT_7:
            lValue = getWordParam(iParam, iChannel);
            break;
        case VAL_DPT_8:
            lValue = getSWordParam(iParam, iChannel);
            break;
        case VAL_DPT_232:
            lValue = getIntParam(iParam, iChannel);
            break;
        case VAL_DPT_9:
            lValue = (getFloatParam(iParam, iChannel) * 100.0);
            break;
        default:
            lValue = getIntParam(iParam, iChannel);
            break;
    }
    return lValue;
}

/********************
 *
 * Runtime processing
 *
 * ******************/

int32_t getInputValueKnx(uint8_t iIOIndex, uint8_t iChannel)
{

    int32_t lValue = 0;
    uint16_t lParamIndex = (iIOIndex == 1) ? LOG_fE1Dpt : LOG_fE2Dpt;
    GroupObject *lKo = getKoForChannel(iIOIndex, iChannel);
    // based on dpt, we read the correct c type.
    uint8_t lDpt = getByteParam(lParamIndex, iChannel);
    switch (lDpt)
    {
        case VAL_DPT_2:
            lValue = lKo->valueRef()[0];
            break;
        case VAL_DPT_6:
            lValue = (int8_t)lKo->value(getDPT(VAL_DPT_6));
            break;
        case VAL_DPT_8:
            lValue = (int16_t)lKo->value(getDPT(VAL_DPT_8));
            break;

        // case VAL_DPT_7:
        //     lValue = lKo->valueRef()[0] + 256 * lKo->valueRef()[1];
        //     break;
        // case VAL_DPT_232:
        //     lValue =
        //         lKo->valueRef()[0] + 256 * lKo->valueRef()[1] + 65536 * lKo->valueRef()[2];
        //     break;
        case VAL_DPT_9:
            lValue = ((double)lKo->value(getDPT(VAL_DPT_9)) * 100.0);
            break;
        // case VAL_DPT_17:
        default:
            lValue = (int32_t)lKo->value(getDPT(lDpt));
            break;
    }
    return lValue;
}

// on input level, we have just numeric values, so all DPT are converted to int:
// DPT1,2,5,6,7,8,17,232 => straight forward
// DPT5001 => scale down to [0..100]
// DPT9 => transport as 1/100, means take int(float * 100)
int32_t getInputValue(uint8_t iIOIndex, uint8_t iChannel)
{
    return getInputValueKnx(iIOIndex, iChannel);
}

void writeConstantValue(sChannelInfo *cData, uint16_t iParam, uint8_t iChannel)
{
    uint8_t lDpt = getByteParam(LOG_fODpt, iChannel);
    switch (lDpt)
    {
        uint8_t lValueByte;
        case VAL_DPT_1:
            bool lValueBool;
            lValueBool = getByteParam(iParam, iChannel) != 0;
            knxWriteBool(IO_Output, iChannel, lValueBool);
            break;
        case VAL_DPT_2:
            lValueByte = getByteParam(iParam, iChannel);
            knxWriteRawInt(IO_Output, iChannel, lValueByte);
            break;
        case VAL_DPT_5:
        case VAL_DPT_5001: // correct value is calculated by dpt handling
            lValueByte = getByteParam(iParam, iChannel);
            knxWriteInt(IO_Output, iChannel, lValueByte);
            break;
        case VAL_DPT_17:
            lValueByte = getByteParam(iParam, iChannel) - 1;
            knxWriteInt(IO_Output, iChannel, lValueByte);
            break;
        case VAL_DPT_6:
            int8_t lValueInt;
            lValueInt = getSByteParam(iParam, iChannel);
            knxWriteRawInt(IO_Output, iChannel, lValueInt);
            break;
        case VAL_DPT_7:
            uint16_t lValueUWord;
            lValueUWord = getWordParam(iParam, iChannel);
            knxWriteInt(IO_Output, iChannel, lValueUWord);
            break;
        case VAL_DPT_8:
            int16_t lValueSWord;
            lValueSWord = getSWordParam(iParam, iChannel);
            knxWriteInt(IO_Output, iChannel, lValueSWord);
            break;
        case VAL_DPT_9:
            float lValueFloat;
            lValueFloat = getFloatParam(iParam, iChannel);
            knxWriteFloat(IO_Output, iChannel, lValueFloat);
            break;
        case VAL_DPT_16:
            uint8_t *lValueStr;
            lValueStr = getStringParam(iParam, iChannel);
            knxWriteString(IO_Output, iChannel, (char *)lValueStr);
            break;
        case VAL_DPT_232:
            int32_t lValueRGB;
            lValueRGB = getIntParam(iParam, iChannel);
            knxWriteInt(IO_Output, iChannel, lValueRGB);
            break;
        default:
            break;
    }
}

void writeParameterValue(sChannelInfo *cData, uint8_t iIOIndex, uint8_t iChannel)
{

    int32_t lValueOrig = getInputValue(iIOIndex, iChannel);
    uint16_t lParamDpt = (iIOIndex == 1) ? LOG_fE1Dpt : LOG_fE2Dpt;
    uint8_t lInputDpt = getByteParam(lParamDpt, iChannel);
    uint8_t lDpt = getByteParam(LOG_fODpt, iChannel);
    int32_t lValue = (lInputDpt == VAL_DPT_9) ? lValueOrig / 10 : lValueOrig;
    switch (lDpt)
    {
        uint8_t lValueByte;
        case VAL_DPT_1:
            bool lValueBool;
            lValueBool = lValue != 0;
            knxWriteBool(IO_Output, iChannel, lValueBool);
            break;
        case VAL_DPT_2:
            lValueByte = lValue;
            knxWriteRawInt(IO_Output, iChannel, lValueByte);
            break;
        case VAL_DPT_5:
        case VAL_DPT_5001:
        case VAL_DPT_6:
        case VAL_DPT_17:
            lValueByte = lValue;
            knxWriteInt(IO_Output, iChannel, lValueByte);
            break;
            // lValueByte = lValue;
            // // DPT5 means, that input value range is [0..100], output value range is
            // // [0..255]
            // lValueByte = (lValueByte / 100.0) * 255.0;
            // knxWrite(0, iChannel, lValueByte);
            // break;
        case VAL_DPT_7:
        case VAL_DPT_8:
            uint16_t lValueWord;
            lValueWord = lValue;
            knxWriteInt(IO_Output, iChannel, lValueWord);
            break;
        case VAL_DPT_9:
            float lValueFloat;
            if (lInputDpt == VAL_DPT_9)
            {
                lValueFloat = lValueOrig / 100.0;
            }
            else
            {
                lValueFloat = lValue;
            }
            knxWriteFloat(IO_Output, iChannel, lValueFloat);
            break;
        case VAL_DPT_16:
            char lValueStr[15];
            sprintf(lValueStr, "%ld", lValue);
            knxWriteString(IO_Output, iChannel, lValueStr);
            break;
        case VAL_DPT_232:
            knxWriteInt(IO_Output, iChannel, lValue);
            break;
        default:
            break;
    }
}

// we trigger all associated internal inputs with the new value
void ProcessInternalInputs(sChannelInfo *cData, uint8_t iChannel, bool iValue)
{

    // search for any internal input associated to this channel
    for (uint8_t lChannel = 0; lChannel < gNumChannels; lChannel++)
    {
        uint8_t lInput1 = getByteParam(LOG_fI1, lChannel);
        if (lInput1 > 0)
        {
            uint32_t lFunction1 = getIntParam(LOG_fI1Function, lChannel);
            if (lFunction1 == (uint32_t)(iChannel + 1))
            {
                sChannelInfo *lData = &gChannelData[lChannel];
                StartLogic(lData, lChannel, BIT_INT_INPUT_1, iValue);
            }
        }
        uint8_t lInput2 = getByteParam(LOG_fI2, lChannel);
        if (lInput2 > 0)
        {
            uint32_t lFunction2 = getIntParam(LOG_fI2Function, lChannel);
            if (lFunction2 == (uint32_t)(iChannel + 1))
            {
                sChannelInfo *lData = &gChannelData[lChannel];
                StartLogic(lData, lChannel, BIT_INT_INPUT_2, iValue);
            }
        }
    }
}

// process the output itself
void ProcessOutput(sChannelInfo *cData, uint8_t iChannel, bool iValue)
{
    ProcessInternalInputs(cData, iChannel, iValue);
    if (iValue)
    {
        uint8_t lOn = getByteParam(LOG_fOOn, iChannel);
        switch (lOn)
        {
            case VAL_Out_Constant:
                writeConstantValue(cData, LOG_fOOnDpt1, iChannel);
                break;
            case VAL_Out_ValE1:
                writeParameterValue(cData, IO_Input1, iChannel);
                break;
            case VAL_Out_ValE2:
                writeParameterValue(cData, IO_Input2, iChannel);
                break;
            case VAL_Out_ReadRequest:
                knxRead(IO_Output, iChannel);
                break;
            case VAL_Out_ResetDevice:
                knxResetDevice(LOG_fOOnDpt1, iChannel);
                break;
            case VAL_Out_Buzzer:
#ifndef __linux__
                if (gBuzzerPin) {
                    //digitalWrite(BUZZER_PIN, HIGH);
                    tone(gBuzzerPin, BUZZER_FREQ);
                }
#endif
                break;
            default:
                // there is no output parametrized
                break;
        }
    }
    else
    {
        uint8_t lOff = getByteParam(LOG_fOOff, iChannel);
        switch (lOff)
        {
            case VAL_Out_Constant:
                writeConstantValue(cData, LOG_fOOffDpt1, iChannel);
                break;
            case VAL_Out_ValE1:
                writeParameterValue(cData, IO_Input1, iChannel);
                break;
            case VAL_Out_ValE2:
                writeParameterValue(cData, IO_Input2, iChannel);
                break;
            case VAL_Out_ReadRequest:
                knxRead(IO_Output, iChannel);
                break;
            case VAL_Out_ResetDevice:
                knxResetDevice(LOG_fOOffDpt1, iChannel);
                break;
            case VAL_Out_Buzzer:
#ifndef __linux__
                if (gBuzzerPin) {
                    //digitalWrite(BUZZER_PIN, LOW);
                    noTone(gBuzzerPin);
                }
#endif
                break;
            default:
                // there is no output parametrized
                break;
        }
    }
    // any valid output removes all input trigger
    cData->triggerIO = 0;
}

void StartStartup(sChannelInfo *cData, uint8_t iChannel)
{
    cData->onDelay = millis();
    cData->currentPipeline |= PIP_STARTUP;
}

// channel startup delay
void ProcessStartup(sChannelInfo *cData, uint8_t iChannel)
{
    if (millis() - cData->onDelay > getIntParam(LOG_fChannelDelay, iChannel) * 1000)
    {
        // we waited enough, remove pipeline marker
        cData->currentPipeline &= ~PIP_STARTUP;
        cData->onDelay = 0;
    }
}

// we send an ReadRequest if reading from input 1 should be repeated
void ProcessRepeatInput1(sChannelInfo *cData, uint8_t iChannel)
{
    uint16_t lRepeatTime = getIntParam(LOG_fE1Repeat, iChannel) * 1000;

    if (millis() - cData->repeatInput1Delay > lRepeatTime)
    {
        knxRead(IO_Input1, iChannel);
        cData->repeatInput1Delay = millis();
        if (lRepeatTime == 0)
            cData->currentPipeline &= ~PIP_REPEAT_INPUT1;
    }
}

// we send an ReadRequest if reading from input 2 should be repeated
void ProcessRepeatInput2(sChannelInfo *cData, uint8_t iChannel)
{

    uint16_t lRepeatTime = getIntParam(LOG_fE2Repeat, iChannel) * 1000;

    if (millis() - cData->repeatInput2Delay > lRepeatTime)
    {
        knxRead(IO_Input2, iChannel);
        cData->repeatInput2Delay = millis();
        if (lRepeatTime == 0)
            cData->currentPipeline &= ~PIP_REPEAT_INPUT2;
    }
}

void StartConvert(sChannelInfo *cData, uint8_t iChannel, uint8_t iIOIndex)
{
    cData->currentPipeline |= (iIOIndex == 1) ? PIP_CONVERT_INPUT1 : PIP_CONVERT_INPUT2;
}

// we convert according ext. input value to bool
void ProcessConvertInput(sChannelInfo *cData, uint8_t iChannel, uint8_t iIOIndex)
{

    uint16_t lParamBase = (iIOIndex == 1) ? LOG_fE1 : LOG_fE2;
    uint8_t lConvert = getByteParam(lParamBase, iChannel) >> 4;
    bool lValueOut = 0;
    // get input value
    int32_t lValue1In = getInputValue(iIOIndex, iChannel);
    int32_t lValue2In = 0;
    if (lConvert & 1)
    {
        // in case of delta conversion get the other input value
        lValue2In = getInputValue(3 - iIOIndex, iChannel);
    }
    uint8_t lDpt = getByteParam(lParamBase + 1, iChannel);
    uint8_t lUpperBound = 0;
    bool lDoDefault = false;
    switch (lDpt)
    {
        case VAL_DPT_1:
            lValueOut = lValue1In;
            break;
        case VAL_DPT_17:
            // there might be 8 possible scenes to check
            lUpperBound = 9; // we start with 2
            lValue1In += 1;
        case VAL_DPT_2:
            // there might be 4 possible zwangsführung values to check
            if (lUpperBound == 0)
                lUpperBound = 5; // we start with 2
            // scenes or zwngsführung have no intervals, but multiple single values
            for (size_t lScene = 2; lScene <= lUpperBound && lValueOut == 0; lScene++)
            {
                uint8_t lValue = getByteParam(lParamBase + lScene, iChannel);
                lValueOut = ((uint8_t)lValue1In == lValue);
            }
            break;
        default:
            lDoDefault = true;
            break;
    }
    if (lDoDefault)
    {
        // for all remaining DPT we determine the input value by an converter module
        switch (lConvert)
        {
            case VAL_InputConvert_Interval:
                lValueOut = (lValue1In >= getParamByDpt(lDpt, lParamBase + 2, iChannel)) &&
                            (lValue1In <= getParamByDpt(lDpt, lParamBase + 6, iChannel));
                break;
            case VAL_InputConvert_DeltaInterval:
                lValueOut = (lValue1In - lValue2In >= getParamForDelta(lDpt, lParamBase + 2, iChannel)) &&
                            (lValue1In - lValue2In <= getParamForDelta(lDpt, lParamBase + 6, iChannel));
                break;
            case VAL_InputConvert_Hysterese:
                lValueOut = cData->currentIO & iIOIndex; // retrieve old result, will be send if current value is in hysterese inbervall
                if (lValue1In <= getParamByDpt(lDpt, lParamBase + 2, iChannel))
                    lValueOut = false;
                if (lValue1In >= getParamByDpt(lDpt, lParamBase + 6, iChannel))
                    lValueOut = true;
                break;
            case VAL_InputConvert_DeltaHysterese:
                lValueOut = cData->currentIO & iIOIndex; // retrieve old result, will be send if current value is in hysterese inbervall
                if (lValue1In - lValue2In <= getParamForDelta(lDpt, lParamBase + 2, iChannel))
                    lValueOut = false;
                if (lValue1In - lValue2In >= getParamForDelta(lDpt, lParamBase + 6, iChannel))
                    lValueOut = true;
                break;

            default:
                // do nothing, wrong converter id
                break;
        }
    }
    // start logic processing for this input
    StartLogic(cData, iChannel, iIOIndex, lValueOut);
}

// starts On-Off-Repeat
void StartOnOffRepeat(sChannelInfo *cData, uint8_t iChannel, bool iOutput)
{
    // with repeat, we first process the ouptut and then we repeat the signal
    // if repeat is already active, we wait until next cycle
    if (iOutput)
    {
        if ((cData->currentPipeline & PIP_ON_REPEAT) == 0)
        {
            cData->repeatOnOffDelay = millis();
            cData->currentPipeline &= ~PIP_OFF_REPEAT;
            ProcessOutput(cData, iChannel, iOutput);
            if (getIntParam(LOG_fORepeatOn, iChannel) > 0)
                cData->currentPipeline |= PIP_ON_REPEAT;
        }
    }
    else
    {
        if ((cData->currentPipeline & PIP_OFF_REPEAT) == 0)
        {
            cData->repeatOnOffDelay = millis();
            cData->currentPipeline &= ~PIP_ON_REPEAT;
            ProcessOutput(cData, iChannel, iOutput);
            if (getIntParam(LOG_fORepeatOff, iChannel) > 0)
                cData->currentPipeline |= PIP_OFF_REPEAT;
        }
    }
}

void ProcessOnOffRepeat(sChannelInfo *cData, uint8_t iChannel)
{

    uint32_t lRepeat = 0;
    bool lValue;

    if (cData->currentPipeline & PIP_ON_REPEAT)
    {
        lRepeat = getIntParam(LOG_fORepeatOn, iChannel) * 100;
        lValue = true;
    }
    if (cData->currentPipeline & PIP_OFF_REPEAT)
    {
        lRepeat = getIntParam(LOG_fORepeatOff, iChannel) * 100;
        lValue = false;
    }

    if (millis() - cData->repeatOnOffDelay > lRepeat)
    {
        // delay time is over, we repeat the output
        ProcessOutput(cData, iChannel, lValue);
        // and we restart repeat counter
        cData->repeatOnOffDelay = millis();
    }
}

// starts Output filter
void StartOutputFilter(sChannelInfo *cData, uint8_t iChannel, bool iOutput)
{
    uint8_t lAllow = (getByteParam(LOG_fOOutputFilter, iChannel) & 96) >> 5;
    bool lLastOutput = (cData->currentIO & BIT_LAST_OUTPUT) > 0;
    bool lContinue = false;
    switch (lAllow)
    {
        case VAL_AllowRepeat_All:
            lContinue = true;
            break;
        case VAL_AllowRepeat_On:
            lContinue = (iOutput || iOutput != lLastOutput);
            break;
        case VAL_AllowRepeat_Off:
            lContinue = (!iOutput || iOutput != lLastOutput);
            break;
        default: // VAL_AlloRepeat_None
            lContinue = (iOutput != lLastOutput);
            break;
    }
    if (lContinue)
    {
        cData->currentPipeline &= ~(PIP_OUTPUT_FILTER_OFF | PIP_OUTPUT_FILTER_ON);
        cData->currentPipeline |= iOutput ? PIP_OUTPUT_FILTER_ON : PIP_OUTPUT_FILTER_OFF;
        cData->currentIO &= ~BIT_LAST_OUTPUT;
        if (iOutput)
            cData->currentIO |= BIT_LAST_OUTPUT;
        // if (iOutput) {
        //     cData->currentPipeline |= PIP_OUTPUT_FILTER_ON;
        // } else {
        //     cData->currentPipeline |= PIP_OUTPUT_FILTER_OFF;
        // }
    }
}

void ProcessOutputFilter(sChannelInfo *cData, uint8_t iChannel)
{
    if (cData->currentPipeline & PIP_OUTPUT_FILTER_ON)
    {
        StartOnOffRepeat(cData, iChannel, true);
    }
    else if (cData->currentPipeline & PIP_OUTPUT_FILTER_OFF)
    {
        StartOnOffRepeat(cData, iChannel, false);
    }
    cData->currentPipeline &= ~(PIP_OUTPUT_FILTER_OFF | PIP_OUTPUT_FILTER_ON);
}

// starts Switch-On-Delay
void StartOnDelay(sChannelInfo *cData, uint8_t iChannel)
{
    // if on delay is already running, there are options:
    //    1. second on switches immediately on
    //    2. second on restarts delay time
    //    3. an off stops on delay
    uint8_t lOnDelay = getByteParam(LOG_fODelay, iChannel);
    uint8_t lOnDelayRepeat = (lOnDelay & 96) >> 5;
    if ((cData->currentPipeline & PIP_ON_DELAY) == 0)
    {
        cData->onDelay = millis();
        cData->currentPipeline |= PIP_ON_DELAY;
    }
    else
    {
        // we have a new on value, we look how to process in case of repetition
        switch (lOnDelayRepeat)
        {
            case VAL_Delay_Immediate:
                // end pipeline and switch immediately
                // cData->currentPipeline &= ~PIP_ON_DELAY;
                // StartOnOffRepeat(cData, iChannel, true);
                cData->onDelay = 0;
                break;
            case VAL_Delay_Extend:
                cData->onDelay = millis();
                break;
            default:
                break;
        }
    }
    uint8_t lOnDelayReset = (lOnDelay & 16) >> 4;
    // if requested, this on stops an off delay
    if ((lOnDelayReset > 0) && (cData->currentPipeline & PIP_OFF_DELAY) > 0)
    {
        cData->currentPipeline &= ~PIP_OFF_DELAY;
    }
}

void ProcessOnDelay(sChannelInfo *cData, uint8_t iChannel)
{
    uint32_t lOnDelay = getIntParam(LOG_fODelayOn, iChannel) * 100;
    if (millis() - cData->onDelay > lOnDelay)
    {
        // delay time is over, we turn off pipeline
        cData->currentPipeline &= ~PIP_ON_DELAY;
        // we start repeatOnProcessing
        StartOutputFilter(cData, iChannel, true);
    }
}

// starts Switch-Off-Delay
void StartOffDelay(sChannelInfo *cData, uint8_t iChannel)
{
    // if off delay is already running, there are options:
    //    1. second off switches immediately off
    //    2. second off restarts delay time
    //    3. an on stops off delay
    uint8_t lOffDelay = getByteParam(LOG_fODelay, iChannel);
    uint8_t lOffDelayRepeat = (lOffDelay & 12) >> 2;
    if ((cData->currentPipeline & PIP_OFF_DELAY) == 0)
    {
        cData->offDelay = millis();
        cData->currentPipeline |= PIP_OFF_DELAY;
    }
    else
    {
        // we have a new on value, we look how to process in case of repetition
        switch (lOffDelayRepeat)
        {
            case VAL_Delay_Immediate:
                // end pipeline and switch immediately
                // cData->currentPipeline &= ~PIP_OFF_DELAY;
                // StartOnOffRepeat(cData, iChannel, false);
                cData->offDelay = 0;
                break;
            case VAL_Delay_Extend:
                cData->offDelay = millis();
                break;
            default:
                break;
        }
    }
    uint8_t lOffDelayReset = (lOffDelay & 2) >> 1;
    // if requested, this on stops an off delay
    if ((lOffDelayReset > 0) && (cData->currentPipeline & PIP_ON_DELAY) > 0)
    {
        cData->currentPipeline &= ~PIP_ON_DELAY;
    }
}

void ProcessOffDelay(sChannelInfo *cData, uint8_t iChannel)
{
    uint32_t lOffDelay = getIntParam(LOG_fODelayOff, iChannel) * 100;
    if (millis() - cData->offDelay > lOffDelay)
    {
        // delay time is over, we turn off pipeline
        cData->currentPipeline &= ~PIP_OFF_DELAY;
        // we start repeatOffProcessing
        StartOutputFilter(cData, iChannel, false);
    }
}

void StartLogic(sChannelInfo *cData, uint8_t iChannel, uint8_t iIOIndex, bool iValue)
{
    // invert input
    bool lValue = iValue;
    uint16_t lParamBase = (iIOIndex == 1) ? LOG_fE1 : (iIOIndex == 2) ? LOG_fE2 : (iIOIndex == 4) ? LOG_fI1 : LOG_fI2;
    if ((getByteParam(lParamBase, iChannel) & BIT_INPUT_MASK) == 2)
        lValue = !iValue;
    // set according input bit
    cData->currentIO &= ~iIOIndex;
    cData->currentIO |= iIOIndex * lValue;
    // set the validity bit
    cData->validActiveIO |= iIOIndex;
    // set the trigger bit
    cData->triggerIO |= iIOIndex;
    // finally set the pipeline bit
    cData->currentPipeline |= PIP_LOGIC_EXECUTE;
}

void ProcessBlink(sChannelInfo *cData, uint8_t iChannel)
{
    uint32_t lBlinkTime = getIntParam(LOG_fOBlink, iChannel) * 100;
    if (millis() - cData->blinkDelay > lBlinkTime)
    {
        bool lOn = !(cData->currentIO & BIT_OUTPUT);
        if (lOn)
        {
            cData->currentIO |= BIT_OUTPUT;
            StartOnDelay(cData, iChannel);
        }
        else
        {
            cData->currentIO &= ~BIT_OUTPUT;
            StartOffDelay(cData, iChannel);
        }
        cData->blinkDelay = millis();
    }
}

void StartBlink(sChannelInfo *cData, uint8_t iChannel)
{
    uint32_t lBlinkTime = getIntParam(LOG_fOBlink, iChannel);
    if (lBlinkTime > 0)
    {
        cData->blinkDelay = millis();
        cData->currentPipeline |= PIP_BLINK;
        cData->currentIO |= BIT_OUTPUT;
    }
}

void ProcessStairlight(sChannelInfo *cData, uint8_t iChannel)
{

    uint8_t lStairTimeBase = getByteParam(LOG_fOTimeBase, iChannel);
    uint32_t lStairTime = getIntParam(LOG_fOTime, iChannel);
    uint32_t lTime = lStairTime * sTimeFactors[lStairTimeBase];

    if (millis() - cData->stairlightDelay > lTime)
    {
        // stairlight time is over, we switch off, also potential blinking
        cData->currentPipeline &= ~(PIP_STAIRLIGHT | PIP_BLINK);
        // we start switchOffProcessing
        StartOffDelay(cData, iChannel);
    }
}

// starts stairlight processing, is aware of parametrized
// behaviour, if stairlight is already running
void StartStairlight(sChannelInfo *cData, uint8_t iChannel, bool iOutput)
{

    if (getByteParam(LOG_fOStair, iChannel))
    {
        if (iOutput)
        {
            // if stairlight is not running yet, we switch first the output to on
            if ((cData->currentPipeline & PIP_STAIRLIGHT) == 0)
                StartOnDelay(cData, iChannel);
            // stairlight should also be switched on
            uint8_t lRetrigger = getByteParam(LOG_fORetrigger, iChannel);
            if ((cData->currentPipeline & PIP_STAIRLIGHT) == 0 || lRetrigger == 1)
            {
                // stairlight is not running or may be retriggered
                // we init the stairlight timer
                cData->stairlightDelay = millis();
                cData->currentPipeline |= PIP_STAIRLIGHT;
                StartBlink(cData, iChannel);
            }
        }
        else
        {
            // if stairlight is not running yet, we switch the output to off
            if ((cData->currentPipeline & PIP_STAIRLIGHT) == 0)
                StartOffDelay(cData, iChannel);
            // stairlight should be switched off
            uint8_t lOff = getByteParam(LOG_fOStairOff, iChannel);
            if (lOff == 1)
            {
                // stairlight might be switched off,
                // we set the timer to 0
                cData->stairlightDelay = 0;
            }
        }
    }
    else if (iOutput)
    {
        // an output without stairlight is forwarded to switch on processing
        StartOnDelay(cData, iChannel);
    }
    else
    {
        StartOffDelay(cData, iChannel);
    }
}

// Processing parametrized logic
void ProcessLogic(sChannelInfo *cData, uint8_t iChannel)
{

    /* Logic execution bit is set from any method which changes input values */
    uint8_t lValidInputs = cData->validActiveIO & BIT_INPUT_MASK;
    uint8_t lActiveInputs = (cData->validActiveIO >> 4) & BIT_INPUT_MASK;
    uint8_t lCurrentInputs = cData->currentIO & lValidInputs;
    bool lCurrentOuput = ((cData->currentIO & BIT_OUTPUT) == BIT_OUTPUT);
    bool lNewOutput = false;
    bool lValidOutput = false;
    // first deactivate execution in pipeline
    cData->currentPipeline &= ~PIP_LOGIC_EXECUTE;
    // we have to delete all trigger if output pipeline is not started
    bool lOutputSent = false;
    if (getByteParam(LOG_fCalculate, iChannel) == 0 || lValidInputs == lActiveInputs)
    {
        // we process only if all inputs are valid or the user requested invalid evaluation
        uint8_t lLogic = getByteParam(LOG_fLogic, iChannel);
        uint8_t lOnes = 0;
        switch (lLogic)
        {

            case VAL_Logic_And:
                // AND handles invalid inputs as 1
                //Check if all bits are set -> logical AND of all input bits
                lNewOutput = (lCurrentInputs == lActiveInputs);
                lValidOutput = true;
                break;

            case VAL_Logic_Or:
                // Check if any bit is set -> logical OR of all input bits
                lNewOutput = (lCurrentInputs > 0); 
                lValidOutput = true;
                break;

            case VAL_Logic_ExOr:
                // EXOR handles invalid inputs as non existig
                // count valid bits in input mask
                for (size_t lBit = 1; lBit < BIT_INPUT_MASK; lBit <<= 1)
                {
                    lOnes += (lCurrentInputs & lBit) > 0;
                }
                // Check if we have an odd number of bits -> logical EXOR of all input bits
                lNewOutput = (lOnes % 2 == 1); 
                lValidOutput = true;
                break;

            case VAL_Logic_Gate:
                // GATE works a little bit more complex
                // E1 OR I1 are the data inputs
                // E2 OR I2 are the gate inputs
                // Invalid data is handled as ???
                // Invalid gate is an open gate (1)
                uint8_t lGate;
                lGate = ((lCurrentInputs & (BIT_EXT_INPUT_2 | BIT_INT_INPUT_2)) > 0);
                uint8_t lValue;
                lValue = ((lCurrentInputs & (BIT_EXT_INPUT_1 | BIT_INT_INPUT_1)) > 0);
                if (lGate)
                    lNewOutput = lValue; 
                lValidOutput = lGate;
                break;

            default:
                break;
        }
        // now there is a new Output value and we know, if it is valid
        // lets check, if we send this value through the pipeline
        // and if not, we have to delete all trigger
        if (lValidOutput)
        {
            uint8_t lTrigger = getByteParam(LOG_fTrigger, iChannel);
            if ((lTrigger == 0 && lNewOutput != lCurrentOuput) ||
                (lTrigger & cData->triggerIO) > 0)
            {
                // set the output value (first delete BIT_OUTPUT and the set the value
                // of lNewOutput)
                cData->currentIO = (cData->currentIO & ~BIT_OUTPUT) | lNewOutput << 4;
                // set the output trigger bit
                cData->triggerIO |= BIT_OUTPUT;
                // now we start stairlight processing
                StartStairlight(cData, iChannel, lNewOutput);
                lOutputSent = true;
            }
        }
    }
    // we have to delete all trigger if output pipeline is not started
    if (!lOutputSent)
        cData->triggerIO = 0;
}

void processInput(uint8_t iIOIndex, uint8_t iChannel)
{
    if (iIOIndex == 0)
        return;
    sChannelInfo *lData = &gChannelData[iChannel];
    uint16_t lParamBase = (iIOIndex == 1) ? LOG_fE1 : LOG_fE2;
    // we have now an event for an input, first we check, if this input is active
    uint8_t lActive = getByteParam(lParamBase, iChannel) & BIT_INPUT_MASK;
    if (lActive > 0)
        // this input is we start convert for this input
        StartConvert(lData, iChannel, iIOIndex);
    // this input might also be used for delta conversion in the other input
    uint16_t lOtherParamBase = (iIOIndex == 2) ? LOG_fE1 : LOG_fE2;
    uint8_t lConverter = getByteParam(lOtherParamBase, iChannel) >> 4;
    if (lConverter & 1)
    {
        // delta convertersion, we start convert for the other input
        StartConvert(lData, iChannel, 3 - iIOIndex);
    }
}

bool isInputActive(uint8_t iIOIndex, uint8_t iChannel) {

    uint8_t lIsActive = getByteParam((iIOIndex == IO_Input1) ? LOG_fE1 : LOG_fE2, iChannel) & BIT_INPUT_MASK;
    if (lIsActive == 0)
    {
        //input might be also activated by a delta input converter, means from the other input
        lIsActive = (getByteParam((iIOIndex == IO_Input2) ? LOG_fE1Convert : LOG_fE2Convert, iChannel) >> 4) & 1;
    }
    return (lIsActive > 0);
}

// returns true, if all dpt should be written to EEPROM
bool prepareChannels()
{
    bool lResult = false;
    for (uint8_t lChannel = 0; lChannel < gNumChannels; lChannel++)
    {
        sChannelInfo *lData = &gChannelData[lChannel];
        // initialize most important runtime field
        lData->currentPipeline = 0;
        lData->validActiveIO = 0;
        lData->triggerIO = 0;
        lData->currentIO = 0;
 
        bool lInput1EEPROM = false;
        bool lInput2EEPROM = false;
        if (getByteParam(LOG_fLogic, lChannel) > 0)
        {
            // function is active, we process input presets
            // external input 1
            if (isInputActive(IO_Input1, lChannel))
            {
                // input is active, we set according flag
                lData->validActiveIO |= BIT_EXT_INPUT_1 << 4;
                // now set input default value
                uint8_t lParInput = getByteParam(LOG_fE1Default, lChannel);
                // shoud default be fetched from EEPROM
                if (lParInput & VAL_InputDefault_EEPROM) {
                    lInput1EEPROM = readOneInputFromEEPROM(IO_Input1, lChannel);
                    if (!lInput1EEPROM) {
                        lParInput &= ~VAL_InputDefault_EEPROM;
                        lResult = true;
                    }
                }
                switch (lParInput)
                {
                    case VAL_InputDefault_Read:
                        /* to read immediately we activate repeated read pipeline with 0 delay */
                        lData->repeatInput1Delay = 0;
                        lData->currentPipeline |= PIP_REPEAT_INPUT1;
                        break;

                    case VAL_InputDefault_False:
                        /* we clear bit for E1 and mark this value as valid */
                        StartLogic(lData, lChannel, BIT_EXT_INPUT_1, false);
                        break;

                    case VAL_InputDefault_True:
                        /* we set bit for E1 and mark this value as valid */
                        StartLogic(lData, lChannel, BIT_EXT_INPUT_1, true);
                        break;

                    default:
                        /* do nothing, value is invalid */
                        break;
                }
            }
            // external input 2
            if (isInputActive(IO_Input2, lChannel))
            {
                // input is active, we set according flag
                lData->validActiveIO |= BIT_EXT_INPUT_2 << 4;
                uint8_t lParInput = getByteParam(LOG_fE2Default, lChannel);
                // shoud default be fetched from EEPROM
                if (lParInput & VAL_InputDefault_EEPROM) {
                    lInput2EEPROM = readOneInputFromEEPROM(IO_Input2, lChannel);
                    if (!lInput2EEPROM) {
                        lParInput &= ~VAL_InputDefault_EEPROM;
                        lResult = true;
                    }
                }
                switch (lParInput)
                {
                    case VAL_InputDefault_Read:
                        /* to read immediately we activate repeated read pipeline with 0 delay */
                        lData->repeatInput2Delay = 0;
                        lData->currentPipeline |= PIP_REPEAT_INPUT2;
                        break;

                    case VAL_InputDefault_False:
                        /* we clear bit for E2 and mark this value as valid */
                        StartLogic(lData, lChannel, BIT_EXT_INPUT_2, false);
                        break;

                    case VAL_InputDefault_True:
                        /* we set bit for E2 and mark this value as valid */
                        StartLogic(lData, lChannel, BIT_EXT_INPUT_2, true);
                        break;

                    default:
                        /* do nothing, value is invalid */
                        break;
                }
            }
            // internal input 1
            // first check, if input is active
            uint8_t lIsActive = getByteParam(LOG_fI1, lChannel);
            if (lIsActive > 0)
            {
                // input is active, we set according flag
                lData->validActiveIO |= BIT_INT_INPUT_1 << 4;
            }
            // internal input 2
            // first check, if input is active
            lIsActive = getByteParam(LOG_fI2, lChannel);
            if (lIsActive > 0)
            {
                // input is active, we set according flag
                lData->validActiveIO |= BIT_INT_INPUT_2 << 4;
            }
            // we set the startup delay
            StartStartup(lData, lChannel);
            // we trigger input processing, if there are values from EEPROM
            if (lInput1EEPROM) processInput(IO_Input1, lChannel);
            if (lInput2EEPROM) processInput(IO_Input2, lChannel);
        }
    }
    return lResult;
}

void logikLoop()
{
    if (!knx.configured())
        return;

    // we loop on all channels an execute pipeline
    for (uint8_t lChannel = 0; lChannel < gNumChannels; lChannel++)
    {
        sChannelInfo *lData = &gChannelData[lChannel];
        if (lData->currentPipeline & PIP_STARTUP)
        {
            ProcessStartup(lData, lChannel);
        }
        else if (lData->currentPipeline > 0)
        {
            // repeat input pipeline
            if (lData->currentPipeline & PIP_REPEAT_INPUT1)
            {
                ProcessRepeatInput1(lData, lChannel);
            }
            if (lData->currentPipeline & PIP_REPEAT_INPUT2)
            {
                ProcessRepeatInput2(lData, lChannel);
            }
            // convert input pipeline
            if (lData->currentPipeline & PIP_CONVERT_INPUT1)
            {
                lData->currentPipeline &= ~PIP_CONVERT_INPUT1;
                ProcessConvertInput(lData, lChannel, IO_Input1);
            }
            if (lData->currentPipeline & PIP_CONVERT_INPUT2)
            {
                lData->currentPipeline &= ~PIP_CONVERT_INPUT2;
                ProcessConvertInput(lData, lChannel, IO_Input2);
            }
            // Logic execution pipeline
            if (lData->currentPipeline & PIP_LOGIC_EXECUTE)
            {
                ProcessLogic(lData, lChannel);
            }
            // stairlight pipeline
            if (lData->currentPipeline & PIP_STAIRLIGHT)
            {
                ProcessStairlight(lData, lChannel);
            }
            // blink pipeline
            if (lData->currentPipeline & PIP_BLINK)
            {
                ProcessBlink(lData, lChannel);
            }
            // On delay pipeline
            if (lData->currentPipeline & PIP_ON_DELAY)
            {
                ProcessOnDelay(lData, lChannel);
            }
            // Off delay pipeline
            if (lData->currentPipeline & PIP_OFF_DELAY)
            {
                ProcessOffDelay(lData, lChannel);
            }
            // Output Filter pipeline
            if (lData->currentPipeline & (PIP_OUTPUT_FILTER_ON | PIP_OUTPUT_FILTER_OFF))
            {
                ProcessOutputFilter(lData, lChannel);
            }
            // On/Off repeat pipeline
            if (lData->currentPipeline & (PIP_ON_REPEAT | PIP_OFF_REPEAT))
            {
                ProcessOnOffRepeat(lData, lChannel);
            }
        }
    }
}

// on input level, all dpt>1 values are converted to bool by the according converter
void processInputKo(GroupObject &iKo)
{
    if (iKo.asap() >= LOG_KoOffset && iKo.asap() < LOG_KoOffset + gNumChannels * LOG_KoBlockSize) {
        uint16_t lKoNumber = iKo.asap() - LOG_KoOffset;
        uint8_t lChannel = lKoNumber / LOG_KoBlockSize;
        uint8_t lIOIndex = lKoNumber % LOG_KoBlockSize + 1;
        processInput(lIOIndex, lChannel);
    }
}

// void printHexBlock(const char* iText, const uint8_t* iData, size_t iLen ) {
//     println(iText);
//     for (size_t i = 0; i < iLen; i+=32)
//     {
//         printHex("    ", iData + i, 32);
//     }
    
// }

// EEPROM handling
// We assume at max 128 channels, each channel 2 inputs, each input max 4 bytes (value) and 1 byte (DPT) = 128 * 2 * (4 + 1) = 1280 bytes to write
// At first, magic word at address 12 = 0x0C is deleted (5 ms).
// Then, DPT list is written startig with page 1 (address 32 = 0x20), in 8 x 16 byte blocks (4 pages), takes 8 * 5 ms = 40 ms
// The data itself is written in 16 byte blocks, each 5 ms means 1024 / 16 * 5 ms = 64 * 5 ms = 320 ms write time, starting at page 5, address 160 = 0xA0.
// Finally, we write magic word at Address 12 again as an ack, that all data was successfully written (5 ms)
// The resulting write time is 5 + 40 + 320 + 5 = 370 ms
// Bytes of the first page (page 0) might be used differently in future
// For inputs, which are not set as "store in memory", we write a dpt 0xFF
//
bool gIsValidEEPROM = false;
uint32_t gLastWriteEEPROM = 0;

void writeSingleDptToEEPROM(uint8_t iIOIndex, uint8_t iChannel) {
    uint8_t lDpt = 0xFF;
    if (isInputActive(iIOIndex, iChannel))
    {
        // now get input default value
        uint8_t lParInput = getByteParam(iIOIndex == 1 ? LOG_fE1Default : LOG_fE2Default, iChannel);
        if (lParInput & VAL_InputDefault_EEPROM) {
            // if the default is EEPROM, we get correct dpt
            lDpt = getByteParam(iIOIndex == 1 ? LOG_fE1Dpt : LOG_fE2Dpt, iChannel);
        }
    }
    Wire.write(lDpt);
}

void writeAllDptToEEPROM()
{
    if (gLastWriteEEPROM > 0 && gLastWriteEEPROM + 10000 > millis())
    {
        println("writeAllDptToEEPROM called repeatedly within 10 seconds, skipped!");
        return;
    }
    gLastWriteEEPROM = millis();

    // prepare initialization
    uint16_t lAddress = (SAVE_BUFFER_START_PAGE + 1) * 32; // begin of DPT memory
    // start writing all dpt. For inputs, which should not be saved, we write a dpt 0xFF
    for (uint8_t lChannel = 0; lChannel < gNumChannels; lChannel++)
    {
        beginPageEEPROM(lAddress);
        writeSingleDptToEEPROM(IO_Input1, lChannel);
        lAddress++;
        writeSingleDptToEEPROM(IO_Input2, lChannel);
        lAddress++;
        if (lAddress % 16 == 0)
            endPageEEPROM(false);
    }
    endPageEEPROM(false);
}

void writeAllInputsToEEPROM(bool iIsInterrupt) {
    if (gLastWriteEEPROM > 0 && gLastWriteEEPROM + 10000 > millis())
    {
        println("writeAllInputsToEEPROM called repeatedly within 10 seconds, skipped!");
        return;
    }
    gLastWriteEEPROM = millis();

    // prepare initialization
    beginWriteKoEEPROM(iIsInterrupt);

    //Begin write of KO values
    uint16_t lAddress = (SAVE_BUFFER_START_PAGE + 5) * 32; // begin of KO value memory
    // for (uint8_t i = 0; i < 10; i++)
        for (uint8_t lChannel = 0; lChannel < gNumChannels; lChannel++)
        {
            beginPageEEPROM(lAddress);
            GroupObject *lKo = getKoForChannel(IO_Input1, lChannel);
            write4BytesEEPROM(lKo->valueRef(), lKo->valueSize());
            lAddress += 4;
            lKo = getKoForChannel(IO_Input2, lChannel);
            write4BytesEEPROM(lKo->valueRef(), lKo->valueSize());
            lAddress += 4;
            if (lAddress % 16 == 0)
                endPageEEPROM(iIsInterrupt);
        }
    endPageEEPROM(iIsInterrupt);

    // as a last step we write magic number back
    // this is also the ACK, that writing was successfull
    endWriteKoEEPROM(iIsInterrupt);
}

bool readOneInputFromEEPROM(uint8_t iIOIndex, uint8_t iChannel) {
    // first check, if EEPROM contains valid values
    if (!gIsValidEEPROM)
        return false;
    // Now check, if the DPT for requested KO is valid
    // DPT might have changed due to new programming after last save
    uint16_t lAddress = (SAVE_BUFFER_START_PAGE + 1) * 32 + iChannel * 2 + iIOIndex - 1;
    prepareReadEEPROM(lAddress, 1);
    uint8_t lSavedDpt = Wire.read();
    uint8_t lNewDpt = getByteParam((iIOIndex == IO_Input1) ? LOG_fE1Dpt : LOG_fE2Dpt, iChannel);
    if (lNewDpt != lSavedDpt) return false;

    // if the dpt is ok, we get the ko value
    lAddress = (SAVE_BUFFER_START_PAGE + 5) * 32 + iChannel * 8 + (iIOIndex - 1) * 4;
    GroupObject *lKo = getKoForChannel(iIOIndex, iChannel);
    prepareReadEEPROM(lAddress, lKo->valueSize());
    int lIndex = 0;
    while (Wire.available() && lIndex < 4)
        lKo->valueRef()[lIndex++] = Wire.read();
    return true;
}

void writeAllInputsToEEPROMFacade(bool iIsInterrupt) {
    uint32_t lTime = millis();
    writeAllInputsToEEPROM(iIsInterrupt); 
    lTime = millis() - lTime;
    print("WriteAllInputsToEEPROM took: ");
    println(lTime);
}

// interrupt handler
void logicOnSafePinInterruptHandler() {
    DbgWrite("Logic: Handling SAVE-Interrupt...");
    // If Interrupt happens during i2c read we try to finish last read first
    while (Wire.available()) Wire.read();
    // now we write everything to EEPROM
    writeAllInputsToEEPROM(true);
}

void logikDebug()
{
    DbgWrite("Logik-LOG_ChannelsFirmware (in Firmware): %d", LOG_ChannelsFirmware);
    DbgWrite("Logik-gNumChannels (in knxprod):  %d", gNumChannels);
    // Test i2c failure
    // we start an i2c read i.e. for EEPROM
    // prepareReadEEPROM(4711, 20);
    // digitalWrite(LED_YELLOW_PIN, HIGH);
    // delay(10000);
    // digitalWrite(LED_YELLOW_PIN, LOW);
}

void logicBeforeRestartHandler() {
    DbgWrite("logic before Restart called");
    writeAllInputsToEEPROMFacade(false);
}

void logicBeforeTableUnloadHandler(TableObject& iTableObject, LoadState& iNewState) {
    static uint32_t sLastCalled = 0;
    DbgWrite("Table changed called with state %d", iNewState);
    
    if (iNewState == 0) {
        DbgWrite("Table unload called");
        if (sLastCalled == 0 || sLastCalled + 10000 < millis()) {
            writeAllInputsToEEPROMFacade(false);
            sLastCalled = millis();
        }
    }
}

void logikSetup(uint8_t iBuzzerPin, uint8_t iSavePin)
{
    Wire.end(); // seems to end hangs on I2C bus
    Wire.begin(); // we use I2C in logic, so we setut the bus. It is not critical to setup it more than once
    gNumChannels = getIntParam(LOG_NumChannels);
    if (LOG_ChannelsFirmware < gNumChannels) {
        DbgWrite("FATAL: Firmware compiled for %d channels, but knxprod needs %d channels!", LOG_ChannelsFirmware, gNumChannels);
        knx.platform().fatalError();
    }
    if (knx.configured())
    {
        // setup buzzer
#ifndef __linux__
        if (iBuzzerPin) {
            gBuzzerPin = iBuzzerPin;
            pinMode(gBuzzerPin, OUTPUT);
        }
#endif
        // we set just a callback if it is not set from a potential caller
        if (GroupObject::classCallback() == 0) GroupObject::classCallback(processInputKo);
        gIsValidEEPROM = checkDataValidEEPROM();
        if (gIsValidEEPROM) {
            DbgWrite("EEPROM contains valid KO inputs");
        } else {
            DbgWrite("EEPROM does NOT contain valid data");
        }
        // we store some input values in case of restart or ets programming
        if (knx.getBeforeRestartCallback() == 0) knx.addBeforeRestartCallback(logicBeforeRestartHandler);
        if (TableObject::getBeforeTableUnloadCallback() == 0) TableObject::addBeforeTableUnloadCallback(logicBeforeTableUnloadHandler);
        // set interrupt for poweroff handling
        if (iSavePin) {
            logicAttachSaveInterrupt(digitalPinToInterrupt(iSavePin), logicOnSafePinInterruptHandler, FALLING);
        }
        if (prepareChannels())
            writeAllDptToEEPROM();
        ;
    }
}

