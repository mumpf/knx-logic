// #include <knx/bits.h>
// #include <knx_facade.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef NUM_Channels
#include "Logikmodul.h"
#endif

// Buzzer
#define BUZZER_PIN 9

// enum input defaults
#define VAL_InputDefault_Undefined 0
#define VAL_InputDefault_Read 1
#define VAL_InputDefault_False 2
#define VAL_InputDefault_True 3

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

// enum supported dpt
#define VAL_DPT_1 0
#define VAL_DPT_2 1
#define VAL_DPT_5 2
#define VAL_DPT_5001 3
#define VAL_DPT_6 4
#define VAL_DPT_7 5
#define VAL_DPT_8 6
#define VAL_DPT_9 7
#define VAL_DPT_16 8
#define VAL_DPT_17 9
#define VAL_DPT_232 10

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
sChannelInfo gChannelData[NUM_Channels];

// forward declaratins
void StartLogic(sChannelInfo *cData, uint8_t iChannel, uint8_t iIOIndex, bool iValue);

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

uint32_t calcParamIndex(uint16_t iParamIndex, int8_t iChannel)
{
    uint32_t lResult = iParamIndex + ((iChannel < 0) ? 0 : iChannel * NUM_paramBlockSize + NUM_paramOffset);
    return lResult;
}

uint8_t getByteParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
#ifdef LOGIKTEST
    return sParamData.data[iParamIndex];
#else
    return knx.paramByte(calcParamIndex(iParamIndex, iChannel));
#endif
}

int8_t getSByteParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
#ifdef LOGIKTEST
    return sParamData.data[iParamIndex];
#else
    uint8_t *lRef = knx.paramData(calcParamIndex(iParamIndex, iChannel));
    return lRef[0];
#endif
}

uint16_t getWordParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
#ifdef LOGIKTEST
    return sParamData.data[iParamIndex] + 256 * sParamData.data[iParamIndex + 1];
#else
    return knx.paramWord(calcParamIndex(iParamIndex, iChannel));
#endif
}

int16_t getSWordParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
#ifdef LOGIKTEST
    return sParamData.data[iParamIndex] + 256 * sParamData.data[iParamIndex + 1];
#else
    uint8_t *lRef = knx.paramData(calcParamIndex(iParamIndex, iChannel));
    return lRef[0] * 256 + lRef[1];
#endif
}

uint32_t getIntParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
#ifdef LOGIKTEST
    return sParamData.data[iParamIndex] + 256 * sParamData.data[iParamIndex + 1] + 256 * 256 * sParamData.data[iParamIndex + 2] + 256 * 256 * 256 * sParamData.data[iParamIndex + 3];
#else
    return knx.paramInt(calcParamIndex(iParamIndex, iChannel));
#endif
}

int32_t getSIntParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
#ifdef LOGIKTEST
    return sParamData.data[iParamIndex] + 256 * sParamData.data[iParamIndex + 1] + 256 * 256 * sParamData.data[iParamIndex + 2] + 256 * 256 * 256 * sParamData.data[iParamIndex + 3];
#else
    return knx.paramInt(calcParamIndex(iParamIndex, iChannel));
#endif
}

float getFloat(uint8_t *data)
{

    union Float {
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
#ifdef LOGIKTEST
    float lFloat = getFloat(&sParamData.data[lIndex]);
#else
    float lFloat = getFloat(knx.paramData(lIndex));
#endif
    return lFloat;
}

uint8_t *getStringParam(uint16_t iParamIndex, int8_t iChannel = -1)
{
    uint16_t lIndex = calcParamIndex(iParamIndex, iChannel);
#ifdef LOGIKTEST
    return &sParamData.data[lIndex];
#else
    return knx.paramData(lIndex);
#endif
}

// if IOIndex = 0, iChannel is the absolute KO number 8starting with 1)
// if IOIndex > 0, iChannel is channel number and IOIndex specifies Input1, Input2 or Output
uint16_t calcKoNumber(uint8_t iIOIndex, uint8_t iChannel)
{
    // int lIndex = (iIOIndex == 0) ? 3 : iIOIndex;
    // return KO_ChannelOffset - 1 + lIndex + iChannel * 3;
    uint16_t lIndex = (iIOIndex) ? KO_Offset - 1 + iIOIndex + iChannel * 3 : iChannel;
    return lIndex;
}

/* calculates correct KO for given I/O index and Channel
 * iIOIndex - 0=Output, 1=External input 1, 2=External input 2
 */
GroupObject *getKoForChannel(uint8_t iIOIndex, uint8_t iChannel)
{
    return &knx.getGroupObject(calcKoNumber(iIOIndex, iChannel));
}

// write value to bus
void knxWriteBool(uint8_t iIOIndex, uint8_t iChannel, bool iValue)
{
    DbgWrite("knxWrite KO %d bool value %d", calcKoNumber(iIOIndex, iChannel), iValue);
#ifndef LOGIKTEST
    getKoForChannel(iIOIndex, iChannel)->value(iValue);
#endif
}

void knxWriteInt(uint8_t iIOIndex, uint8_t iChannel, int32_t iValue)
{
    DbgWrite("knxWrite KO %d int value %d", calcKoNumber(iIOIndex, iChannel), iValue);
#ifndef LOGIKTEST
    getKoForChannel(iIOIndex, iChannel)->value((int32_t)iValue);
#endif
}

void knxWriteRawInt(uint8_t iIOIndex, uint8_t iChannel, int32_t iValue)
{
    DbgWrite("knxWrite KO %d int value %d", calcKoNumber(iIOIndex, iChannel), iValue);
#ifndef LOGIKTEST
    GroupObject *lKo = getKoForChannel(iIOIndex, iChannel);
    uint8_t *lValueRef = lKo->valueRef();
    *lValueRef = iValue;
    lKo->objectWritten();
#endif
}

void knxWriteFloat(uint8_t iIOIndex, uint8_t iChannel, float iValue)
{
    DbgWrite("knxWrite KO %d float value %f", calcKoNumber(iIOIndex, iChannel), iValue);
#ifndef LOGIKTEST
    getKoForChannel(iIOIndex, iChannel)->value(iValue);
#endif
}

void knxWriteString(uint8_t iIOIndex, uint8_t iChannel, char *iValue)
{
    DbgWrite("knxWrite KO %d string value %s", calcKoNumber(iIOIndex, iChannel), iValue);
#ifndef LOGIKTEST
    getKoForChannel(iIOIndex, iChannel)->value(iValue);
#endif
}

// send read request on bus
void knxRead(uint8_t iIOIndex, uint8_t iChannel)
{
    DbgWrite("knxReadRequest end from KO %d", calcKoNumber(iIOIndex, iChannel));
#ifndef LOGIKTEST
    getKoForChannel(iIOIndex, iChannel)->requestObjectRead();
#endif
}

// send reset device to bus
void knxResetDevice(uint16_t iParamIndex, uint8_t iChannel)
{
    uint16_t lAddress = getWordParam(iParamIndex, iChannel);
    uint8_t lHigh = lAddress / 256;
    DbgWrite("knxResetDevice with PA %d.%d.%d", lHigh / 16, lHigh % 16, lAddress % 256);
#ifndef LOGIKTEST
    knx.restart(lAddress);
#endif
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

#ifdef LOGIKTEST
int getInputValueTest(uint8_t iIOIndex, uint8_t iChannel)
{
    int lValue = 0;
    int lParamIndex = (iIOIndex == 1) ? PAR_CH_E1Dpt : PAR_CH_E2Dpt;
    int lIndex = calcKoNumber(iIOIndex, iChannel) - 1;
    // based on dpt, we read the correct c type.
    switch (getByteParam(lParamIndex, iChannel))
    {
    case VAL_DPT_1:
        lValue = sKoData.data[lIndex];
        break;
    case VAL_DPT_5:
        lValue = ((int)sKoData.data[lIndex] * 100 / 255);
        break;
    case VAL_DPT_2:
    case VAL_DPT_6:
    case VAL_DPT_17:
        lValue = sKoData.data[lIndex];
        break;
    case VAL_DPT_7:
    case VAL_DPT_8:
        lValue = sKoData.data[lIndex];
        break;
    case VAL_DPT_232:
        lValue = sKoData.data[lIndex];
        break;
    case VAL_DPT_9:
        lValue = ((double)sKoData.data[lIndex] * 100.0);
        break;
    default:
        break;
    }
    return lValue;
}
#endif

int32_t getInputValueKnx(uint8_t iIOIndex, uint8_t iChannel)
{

    int32_t lValue = 0;
    uint16_t lParamIndex = (iIOIndex == 1) ? PAR_CH_E1Dpt : PAR_CH_E2Dpt;
    GroupObject *lKo = getKoForChannel(iIOIndex, iChannel);
    // based on dpt, we read the correct c type.
    switch (getByteParam(lParamIndex, iChannel))
    {
    case VAL_DPT_2:
        lValue = lKo->valueRef()[0];
        break;
    case VAL_DPT_6:
        lValue = (int8_t)lKo->value();
        break;
    case VAL_DPT_8:
        lValue = (int16_t)lKo->value();
        break;

    // case VAL_DPT_7:
    //     lValue = lKo->valueRef()[0] + 256 * lKo->valueRef()[1];
    //     break;
    // case VAL_DPT_232:
    //     lValue =
    //         lKo->valueRef()[0] + 256 * lKo->valueRef()[1] + 65536 * lKo->valueRef()[2];
    //     break;
    case VAL_DPT_9:
        lValue = ((double)lKo->value() * 100.0);
        break;
    // case VAL_DPT_17:
    default:
        lValue = (int32_t)lKo->value();
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
#ifdef LOGIKTEST
    return getInputValueTest(iIOIndex, iChannel);
#else
    return getInputValueKnx(iIOIndex, iChannel);
#endif
}

void writeConstantValue(sChannelInfo *cData, uint16_t iParam, uint8_t iChannel)
{
    uint8_t lDpt = getByteParam(PAR_CH_ODpt, iChannel);
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
    uint16_t lParamDpt = (iIOIndex == 1) ? PAR_CH_E1Dpt : PAR_CH_E2Dpt;
    uint8_t lInputDpt = getByteParam(lParamDpt, iChannel);
    uint8_t lDpt = getByteParam(PAR_CH_ODpt, iChannel);
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
        uint8_t lInput1 = getByteParam(PAR_CH_I1, lChannel);
        if (lInput1 > 0)
        {
            uint32_t lFunction1 = getIntParam(PAR_CH_I1Function, lChannel);
            if (lFunction1 == (uint32_t)(iChannel + 1))
            {
                sChannelInfo *lData = &gChannelData[lChannel];
                StartLogic(lData, lChannel, BIT_INT_INPUT_1, iValue);
            }
        }
        uint8_t lInput2 = getByteParam(PAR_CH_I2, lChannel);
        if (lInput2 > 0)
        {
            uint32_t lFunction2 = getIntParam(PAR_CH_I2Function, lChannel);
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
        uint8_t lOn = getByteParam(PAR_CH_OOn, iChannel);
        switch (lOn)
        {
        case VAL_Out_Constant:
            writeConstantValue(cData, PAR_CH_OOnDpt1, iChannel);
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
            knxResetDevice(PAR_CH_OOnDpt1, iChannel);
            break;
        case VAL_Out_Buzzer:
#ifndef __linux__
            digitalWrite(BUZZER_PIN, HIGH);
#endif
            break;
        default:
            // there is no output parametrized
            break;
        }
    }
    else
    {
        uint8_t lOff = getByteParam(PAR_CH_OOff, iChannel);
        switch (lOff)
        {
        case VAL_Out_Constant:
            writeConstantValue(cData, PAR_CH_OOffDpt1, iChannel);
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
            knxResetDevice(PAR_CH_OOffDpt1, iChannel);
            break;
        case VAL_Out_Buzzer:
#ifndef __linux__
            digitalWrite(BUZZER_PIN, LOW);
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
    if (millis() - cData->onDelay > getIntParam(PAR_CH_ChannelDelay, iChannel) * 1000)
    {
        // we waited enough, remove pipeline marker
        cData->currentPipeline &= ~PIP_STARTUP;
        cData->onDelay = 0;
    }
}

// we send an ReadRequest if reading from input 1 should be repeated
void ProcessRepeatInput1(sChannelInfo *cData, uint8_t iChannel)
{
    uint16_t lRepeatTime = getIntParam(PAR_CH_E1Repeat, iChannel) * 1000;

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

    uint16_t lRepeatTime = getIntParam(PAR_CH_E2Repeat, iChannel) * 1000;

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

    uint16_t lParamBase = (iIOIndex == 1) ? PAR_CH_E1 : PAR_CH_E2;
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
            if (getIntParam(PAR_CH_ORepeatOn, iChannel) > 0)
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
            if (getIntParam(PAR_CH_ORepeatOff, iChannel) > 0)
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
        lRepeat = getIntParam(PAR_CH_ORepeatOn, iChannel) * 100;
        lValue = true;
    }
    if (cData->currentPipeline & PIP_OFF_REPEAT)
    {
        lRepeat = getIntParam(PAR_CH_ORepeatOff, iChannel) * 100;
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
    uint8_t lAllow = (getByteParam(PAR_CH_OOutputFilter, iChannel) & 96) >> 5;
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
    uint8_t lOnDelay = getByteParam(PAR_CH_ODelay, iChannel);
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
    uint32_t lOnDelay = getIntParam(PAR_CH_ODelayOn, iChannel) * 100;
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
    uint8_t lOffDelay = getByteParam(PAR_CH_ODelay, iChannel);
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
    uint32_t lOffDelay = getIntParam(PAR_CH_ODelayOff, iChannel) * 100;
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
    uint16_t lParamBase = (iIOIndex == 1) ? PAR_CH_E1 : (iIOIndex == 2) ? PAR_CH_E2 : (iIOIndex == 4) ? PAR_CH_I1 : PAR_CH_I2;
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
    uint32_t lBlinkTime = getIntParam(PAR_CH_OBlink, iChannel) * 100;
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
    uint32_t lBlinkTime = getIntParam(PAR_CH_OBlink, iChannel);
    if (lBlinkTime > 0)
    {
        cData->blinkDelay = millis();
        cData->currentPipeline |= PIP_BLINK;
        cData->currentIO |= BIT_OUTPUT;
    }
}

void ProcessStairlight(sChannelInfo *cData, uint8_t iChannel)
{

    uint8_t lStairTimeBase = getByteParam(PAR_CH_OTimeBase, iChannel);
    uint32_t lStairTime = getIntParam(PAR_CH_OTime, iChannel);
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

    if (getByteParam(PAR_CH_OStair, iChannel))
    {
        if (iOutput)
        {
            // if stairlight is not running yet, we switch first the output to on
            if ((cData->currentPipeline & PIP_STAIRLIGHT) == 0)
                StartOnDelay(cData, iChannel);
            // stairlight should also be switched on
            uint8_t lRetrigger = getByteParam(PAR_CH_ORetrigger, iChannel);
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
            uint8_t lOff = getByteParam(PAR_CH_OStairOff, iChannel);
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
    bool lNewOutput;
    bool lValidOutput = false;
    // first deactivate execution in pipeline
    cData->currentPipeline &= ~PIP_LOGIC_EXECUTE;
    // we have to delete all trigger if output pipeline is not started
    bool lOutputSent = false;
    if (getByteParam(PAR_CH_Calculate, iChannel) == 0 || lValidInputs == lActiveInputs)
    {
        // we process only if all inputs are valid or the user requested invalid evaluation
        uint8_t lLogic = getByteParam(PAR_CH_Logic, iChannel);
        uint8_t lOnes = 0;
        switch (lLogic)
        {

        case VAL_Logic_And:
            // AND handles invalid inputs as 1
            //			lCurrentInputs |= ~lValidInputs &
            //BIT_INPUT_MASK; //Add invalid bits to current input 			lNewOutput =
            //(lValidInputs == 15);				  //Check if all bits
            //are set -> logical AND of all input bits
            lNewOutput = (lCurrentInputs == lActiveInputs);
            lValidOutput = true;
            break;

        case VAL_Logic_Or:
            // OR handles invalid inputs as 0
            //			lCurrentInputs &= lValidInputs;  //Add invalid
            //bits to current input
            lNewOutput = (lCurrentInputs > 0); // Check if any bit is set -> logical OR of all input bits
            lValidOutput = true;
            break;

        case VAL_Logic_ExOr:
            // EXOR handles invalid inputs as non existig
            // count valid bits in input mask
            for (size_t lBit = 1; lBit < BIT_INPUT_MASK; lBit <<= 1)
            {
                lOnes += (lCurrentInputs & lBit) > 0;
            }
            lNewOutput = (lOnes % 2 == 1); // Check if we have an odd number of bits
                                           // -> logical EXOR of all input bits
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
                lNewOutput = lValue; // Check if any bit is set -> logical OR of all input bits
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
            uint8_t lTrigger = getByteParam(PAR_CH_Trigger, iChannel);
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

void prepareChannels()
{

    for (uint8_t lChannel = 0; lChannel < gNumChannels; lChannel++)
    {
        sChannelInfo *lData = &gChannelData[lChannel];
        // initialize most important runtime field
        lData->currentPipeline = 0;
        lData->validActiveIO = 0;
        lData->triggerIO = 0;
        lData->currentIO = 0;

        if (getByteParam(PAR_CH_Logic, lChannel) > 0)
        {
            // function is active, we process input presets
            // external input 1
            // first check, if input is active
            uint8_t lIsActive = getByteParam(PAR_CH_E1, lChannel) & BIT_INPUT_MASK;
            if (lIsActive == 0)
            {
                //input 1 might be also activated by a delta input converter
                lIsActive = (getByteParam(PAR_CH_E2Convert, lChannel) >> 4) & 1;
            }
            if (lIsActive > 0)
            {
                // input is active, we set according flag
                lData->validActiveIO |= BIT_EXT_INPUT_1 << 4;
                // now set input default value
                uint8_t lParInput = getByteParam(PAR_CH_E1Default, lChannel);
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
            // first check, if input is active
            lIsActive = getByteParam(PAR_CH_E2, lChannel);
            if (lIsActive == 0)
            {
                //input 2 might be also activated by a delta input converter
                lIsActive = (getByteParam(PAR_CH_E1Convert, lChannel) >> 4) & 1;
            }
            if (lIsActive > 0)
            {
                // input is active, we set according flag
                lData->validActiveIO |= BIT_EXT_INPUT_2 << 4;
                uint8_t lParInput = getByteParam(PAR_CH_E2Default, lChannel);
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
            lIsActive = getByteParam(PAR_CH_I1, lChannel);
            if (lIsActive > 0)
            {
                // input is active, we set according flag
                lData->validActiveIO |= BIT_INT_INPUT_1 << 4;
            }
            // internal input 2
            // first check, if input is active
            lIsActive = getByteParam(PAR_CH_I2, lChannel);
            if (lIsActive > 0)
            {
                // input is active, we set according flag
                lData->validActiveIO |= BIT_INT_INPUT_2 << 4;
            }
            // we set the startup delay
            StartStartup(lData, lChannel);
        }
    }
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

void processInput(uint8_t iIOIndex, uint8_t iChannel)
{
    if (iIOIndex == 0)
        return;
    sChannelInfo *lData = &gChannelData[iChannel];
    uint16_t lParamBase = (iIOIndex == 1) ? PAR_CH_E1 : PAR_CH_E2;
    // we have now an event for an input, first we check, if this input is active
    uint8_t lActive = getByteParam(lParamBase, iChannel) & BIT_INPUT_MASK;
    if (lActive > 0)
        // this input is we start convert for this input
        StartConvert(lData, iChannel, iIOIndex);
    // this input might also be used for delta conversion in the other input
    uint16_t lOtherParamBase = (iIOIndex == 2) ? PAR_CH_E1 : PAR_CH_E2;
    uint8_t lConverter = getByteParam(lOtherParamBase, iChannel) >> 4;
    if (lConverter & 1)
    {
        // delta convertersion, we start convert for the other input
        StartConvert(lData, iChannel, 3 - iIOIndex);
    }
}

// on input level, all dpt>1 values are converted to bool by the according converter
void processInputKo(GroupObject &iKo)
{
    uint16_t lKoNumber = iKo.asap() - KO_Offset;
    uint8_t lChannel = lKoNumber / 3;
    uint8_t lIOIndex = lKoNumber % 3 + 1;
    processInput(lIOIndex, lChannel);
}

#ifdef LOGIKTEST
void processInputTest(uint8_t iKoIndex)
{
    uint8_t lChannel = iKoIndex / 3;
    uint8_t lIOIndex = iKoIndex % 3;
    processInput(lIOIndex, lChannel);
}
#endif

/********************
 *
 * Setup processing
 *
 *******************/
void setDPT(GroupObject *iKo, uint8_t iChannel, uint8_t iParamDpt)
{
    uint8_t lDpt = getByteParam(iParamDpt, iChannel);
    switch (lDpt)
    {
    case VAL_DPT_1:
        iKo->dataPointType(Dpt(1, 1));
        break;
    case VAL_DPT_2:
        iKo->dataPointType(Dpt(2, 1));
        break;
    case VAL_DPT_5:
        iKo->dataPointType(Dpt(5, 10));
        break;
    case VAL_DPT_5001:
        iKo->dataPointType(Dpt(5, 1));
        break;
    case VAL_DPT_6:
        iKo->dataPointType(Dpt(6, 1));
        break;
    case VAL_DPT_7:
        iKo->dataPointType(Dpt(7, 1));
        break;
    case VAL_DPT_8:
        iKo->dataPointType(Dpt(8, 1));
        break;
    case VAL_DPT_9:
        iKo->dataPointType(Dpt(9, 2));
        break;
    case VAL_DPT_16:
        iKo->dataPointType(Dpt(16, 1));
        break;
    case VAL_DPT_17:
        iKo->dataPointType(Dpt(17, 1));
        break;
    case VAL_DPT_232:
        iKo->dataPointType(Dpt(232, 600));
        break;
    default:
        break;
    }
}

void logikDebug()
{
    DbgWrite("Logik-NUM_Channels: %d", NUM_Channels);
    DbgWrite("Logik-gNumChannels: %d", gNumChannels);
}

void logikSetup()
{
    gNumChannels = getIntParam(PAR_NumChannels);
    if (NUM_Channels < gNumChannels)
        knx.platform().fatalError();
    if (knx.configured())
    {
        // setup buzzer
#ifndef __linux__
        pinMode(BUZZER_PIN, OUTPUT);
#endif
        for (uint8_t lChannel = 0; lChannel < gNumChannels; lChannel++)
        {
            // we initialize DPT for output ko
            GroupObject *lKo = getKoForChannel(IO_Output, lChannel);
            setDPT(lKo, lChannel, PAR_CH_ODpt);
            // we initialize DPT and callback for input1 ko
            lKo = getKoForChannel(IO_Input1, lChannel);
            setDPT(lKo, lChannel, PAR_CH_E1Dpt);
            lKo->callback(processInputKo);
            // we initialize DPT and callback for input2 ko
            lKo = getKoForChannel(IO_Input2, lChannel);
            setDPT(lKo, lChannel, PAR_CH_E2Dpt);
            lKo->callback(processInputKo);
        }
        prepareChannels();
    }
}