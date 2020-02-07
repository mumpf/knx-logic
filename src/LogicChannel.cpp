#include "LogicChannel.h"
#include "Logic.h"
#include "Helper.h"
#include "Board.h"

Logic *LogicChannel::sLogic = nullptr;

/******************************
 * Constructors
 * ***************************/
LogicChannel::LogicChannel(uint8_t iChannelNumber)
{
    mChannelId = iChannelNumber;
    // initialize most important runtime field
    pCurrentPipeline = 0;
    pValidActiveIO = 0;
    pTriggerIO = 0;
    pCurrentIO = 0;
}

LogicChannel::~LogicChannel()
{
}

/******************************
 * Parameter helper
 * ***************************/
uint32_t LogicChannel::calcParamIndex(uint16_t iParamIndex)
{
    uint32_t lResult = iParamIndex + mChannelId * LOG_ParamBlockSize + LOG_ParamBlockOffset;
    return lResult;
}

uint8_t LogicChannel::getByteParam(uint16_t iParamIndex)
{
    return knx.paramByte(calcParamIndex(iParamIndex));
}

int8_t LogicChannel::getSByteParam(uint16_t iParamIndex)
{
    uint8_t *lRef = knx.paramData(calcParamIndex(iParamIndex));
    return lRef[0];
}

uint16_t LogicChannel::getWordParam(uint16_t iParamIndex)
{
    return knx.paramWord(calcParamIndex(iParamIndex));
}

int16_t LogicChannel::getSWordParam(uint16_t iParamIndex)
{
    uint8_t *lRef = knx.paramData(calcParamIndex(iParamIndex));
    return lRef[0] * 256 + lRef[1];
}

uint32_t LogicChannel::getIntParam(uint16_t iParamIndex)
{
    return knx.paramInt(calcParamIndex(iParamIndex));
}

int32_t LogicChannel::getSIntParam(uint16_t iParamIndex)
{
    return knx.paramInt(calcParamIndex(iParamIndex));
}

float LogicChannel::getFloat(uint8_t *data)
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

float LogicChannel::getFloatParam(uint16_t iParamIndex)
{
    uint16_t lIndex = calcParamIndex(iParamIndex);
    float lFloat = getFloat(knx.paramData(lIndex));
    return lFloat;
}

uint8_t *LogicChannel::getStringParam(uint16_t iParamIndex)
{
    uint16_t lIndex = calcParamIndex(iParamIndex);
    return knx.paramData(lIndex);
}

/*******************************
 * ComObject helper
 * ****************************/
// static
uint16_t LogicChannel::calcKoNumber(uint8_t iIOIndex, uint8_t iChannelId)
{
    // do not use iIOIndex = 0
    uint16_t lIndex = (iIOIndex) ? LOG_KoOffset - 1 + iIOIndex + iChannelId * LOG_KoBlockSize : iChannelId;
    return lIndex;
}

// static
GroupObject *LogicChannel::getKoForChannel(uint8_t iIOIndex, uint8_t iChannelId)
{
    return &knx.getGroupObject(calcKoNumber(iIOIndex, iChannelId));
}

uint16_t LogicChannel::calcKoNumber(uint8_t iIOIndex) {
    return LogicChannel::calcKoNumber(iIOIndex, mChannelId);
}

GroupObject *LogicChannel::getKo(uint8_t iIOIndex) {
    return LogicChannel::getKoForChannel(iIOIndex, mChannelId);
}

Dpt &LogicChannel::getKoDPT(uint8_t iIOIndex)
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
            lParamDpt = 0;
            break;
    }
    uint8_t lDpt = getByteParam(lParamDpt);
    return getDPT(lDpt);
}

// write value to bus
void LogicChannel::knxWriteBool(uint8_t iIOIndex, bool iValue)
{
    printDebug("knxWrite KO %d bool value %d\n", calcKoNumber(iIOIndex), iValue);
    getKo(iIOIndex)->value(iValue, getKoDPT(iIOIndex));
}

void LogicChannel::knxWriteInt(uint8_t iIOIndex, int32_t iValue)
{
    printDebug("knxWrite KO %d int value %li\n", calcKoNumber(iIOIndex), iValue);
    getKo(iIOIndex)->value((int32_t)iValue, getKoDPT(iIOIndex));
}

void LogicChannel::knxWriteRawInt(uint8_t iIOIndex, int32_t iValue)
{
    printDebug("knxWrite KO %d int value %li\n", calcKoNumber(iIOIndex), iValue);
    GroupObject *lKo = getKo(iIOIndex);
    uint8_t *lValueRef = lKo->valueRef();
    *lValueRef = iValue;
    lKo->objectWritten();
}

void LogicChannel::knxWriteFloat(uint8_t iIOIndex, float iValue)
{
    printDebug("knxWrite KO %d float value %f\n", calcKoNumber(iIOIndex), iValue);
    getKo(iIOIndex)->value(iValue, getKoDPT(iIOIndex));
}

void LogicChannel::knxWriteString(uint8_t iIOIndex, char *iValue)
{
    printDebug("knxWrite KO %d string value %s\n", calcKoNumber(iIOIndex), iValue);
    getKo(iIOIndex)->value(iValue, getKoDPT(iIOIndex));
}

// send read request on bus
void LogicChannel::knxRead(uint8_t iIOIndex)
{
    printDebug("knxReadRequest end from KO %d\n", calcKoNumber(iIOIndex));
    getKo(iIOIndex)->requestObjectRead();
}

// send reset device to bus
void LogicChannel::knxResetDevice(uint16_t iParamIndex)
{
    uint16_t lAddress = getWordParam(iParamIndex);
    uint8_t lHigh = lAddress >> 8;
    printDebug("knxResetDevice with PA %d.%d.%d\n", lHigh >> 4, lHigh & 0xF, lAddress & 0xFF);
    knx.restart(lAddress);
}

/********************************
 * Logic helper functions
 * *****************************/

// we get an dpt dependant parameter value for difference
// input evaluation
// here special handling is necessary because of transport
// of dpt 9 as an int * 100
int32_t LogicChannel::getParamForDelta(uint8_t iDpt, uint16_t iParamIndex)
{

    int32_t lValue;
    if (iDpt == VAL_DPT_9)
    {
        lValue = getFloatParam(iParamIndex) * 100.0;
    }
    else
    {
        lValue = (int32_t)getIntParam(iParamIndex);
    }
    return lValue;
}

// we get here numeric params by their DPT
// DPT1,2,5,6,7,8,17,232 => straight forward int vaues
// DPT2,17 => straight forward byte values
// DPT5001 => scale down to [0..100]
// DPT9 => transport as 1/100, means take int(float * 100)
int32_t LogicChannel::getParamByDpt(uint8_t iDpt, uint16_t iParamIndex)
{
    int32_t lValue = 0;
    switch (iDpt)
    {
        case VAL_DPT_1:
            lValue = getByteParam(iParamIndex) != 0;
            break;
        case VAL_DPT_2:
        case VAL_DPT_5:
        case VAL_DPT_17:
        case VAL_DPT_5001:
            lValue = getByteParam(iParamIndex);
            break;
        case VAL_DPT_6:
            lValue = getSByteParam(iParamIndex);
            break;
        case VAL_DPT_7:
            lValue = getWordParam(iParamIndex);
            break;
        case VAL_DPT_8:
            lValue = getSWordParam(iParamIndex);
            break;
        case VAL_DPT_232:
            lValue = getIntParam(iParamIndex);
            break;
        case VAL_DPT_9:
            lValue = (getFloatParam(iParamIndex) * 100.0);
            break;
        default:
            lValue = getIntParam(iParamIndex);
            break;
    }
    return lValue;
}

// on input level, we have just numeric values, so all DPT are converted to int:
// DPT1,2,5,6,7,8,17,232 => straight forward
// DPT5001 => scale down to [0..100]
// DPT9 => transport as 1/100, means take int(float * 100)
int32_t LogicChannel::getInputValue(uint8_t iIOIndex)
{

    int32_t lValue = 0;
    uint16_t lParamIndex = (iIOIndex == 1) ? LOG_fE1Dpt : LOG_fE2Dpt;
    GroupObject *lKo = getKo(iIOIndex);
    // based on dpt, we read the correct c type.
    uint8_t lDpt = getByteParam(lParamIndex);
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

void LogicChannel::writeConstantValue(uint16_t iParamIndex)
{
    uint8_t lDpt = getByteParam(LOG_fODpt);
    switch (lDpt)
    {
        uint8_t lValueByte;
        case VAL_DPT_1:
            bool lValueBool;
            lValueBool = getByteParam(iParamIndex) != 0;
            knxWriteBool(IO_Output, lValueBool);
            break;
        case VAL_DPT_2:
            lValueByte = getByteParam(iParamIndex);
            knxWriteRawInt(IO_Output, lValueByte);
            break;
        case VAL_DPT_5:
        case VAL_DPT_5001: // correct value is calculated by dpt handling
            lValueByte = getByteParam(iParamIndex);
            knxWriteInt(IO_Output, lValueByte);
            break;
        case VAL_DPT_17:
            lValueByte = getByteParam(iParamIndex) - 1;
            knxWriteInt(IO_Output, lValueByte);
            break;
        case VAL_DPT_6:
            int8_t lValueInt;
            lValueInt = getSByteParam(iParamIndex);
            knxWriteRawInt(IO_Output, lValueInt);
            break;
        case VAL_DPT_7:
            uint16_t lValueUWord;
            lValueUWord = getWordParam(iParamIndex);
            knxWriteInt(IO_Output, lValueUWord);
            break;
        case VAL_DPT_8:
            int16_t lValueSWord;
            lValueSWord = getSWordParam(iParamIndex);
            knxWriteInt(IO_Output, lValueSWord);
            break;
        case VAL_DPT_9:
            float lValueFloat;
            lValueFloat = getFloatParam(iParamIndex);
            knxWriteFloat(IO_Output, lValueFloat);
            break;
        case VAL_DPT_16:
            uint8_t *lValueStr;
            lValueStr = getStringParam(iParamIndex);
            knxWriteString(IO_Output, (char *)lValueStr);
            break;
        case VAL_DPT_232:
            int32_t lValueRGB;
            lValueRGB = getIntParam(iParamIndex);
            knxWriteInt(IO_Output, lValueRGB);
            break;
        default:
            break;
    }
}

void LogicChannel::writeParameterValue(uint8_t iIOIndex)
{
    int32_t lValueOrig = getInputValue(iIOIndex);
    uint16_t lParamDpt = (iIOIndex == 1) ? LOG_fE1Dpt : LOG_fE2Dpt;
    uint8_t lInputDpt = getByteParam(lParamDpt);
    uint8_t lDpt = getByteParam(LOG_fODpt);
    int32_t lValue = (lInputDpt == VAL_DPT_9) ? lValueOrig / 10 : lValueOrig;
    switch (lDpt)
    {
        uint8_t lValueByte;
        case VAL_DPT_1:
            bool lValueBool;
            lValueBool = lValue != 0;
            knxWriteBool(IO_Output, lValueBool);
            break;
        case VAL_DPT_2:
            lValueByte = lValue;
            knxWriteRawInt(IO_Output, lValueByte);
            break;
        case VAL_DPT_5:
        case VAL_DPT_5001:
        case VAL_DPT_6:
        case VAL_DPT_17:
            lValueByte = lValue;
            knxWriteInt(IO_Output, lValueByte);
            break;
            // lValueByte = lValue;
            // // DPT5 means, that input value range is [0..100], output value range is
            // // [0..255]
            // lValueByte = (lValueByte / 100.0) * 255.0;
            // knxWrite(0, lValueByte);
            // break;
        case VAL_DPT_7:
        case VAL_DPT_8:
            uint16_t lValueWord;
            lValueWord = lValue;
            knxWriteInt(IO_Output, lValueWord);
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
            knxWriteFloat(IO_Output, lValueFloat);
            break;
        case VAL_DPT_16:
            char lValueStr[15];
            sprintf(lValueStr, "%ld", lValue);
            knxWriteString(IO_Output, lValueStr);
            break;
        case VAL_DPT_232:
            knxWriteInt(IO_Output, lValue);
            break;
        default:
            break;
    }
}

/********************************
 * Logic functions
 *******************************/
bool LogicChannel::isInputActive(uint8_t iIOIndex) {
    uint8_t lIsActive = getByteParam((iIOIndex == IO_Input1) ? LOG_fE1 : LOG_fE2) & BIT_INPUT_MASK;
    if (lIsActive == 0)
    {
        //input might be also activated by a delta input converter, means from the other input
        lIsActive = (getByteParam((iIOIndex == IO_Input2) ? LOG_fE1Convert : LOG_fE2Convert) >> 4) & 1;
    }
    return (lIsActive > 0);
}


// channel startup delay
void LogicChannel::startStartup()
{
    pOnDelay = millis();
    pCurrentPipeline |= PIP_STARTUP;
}

void LogicChannel::processStartup()
{
    if (millis() - pOnDelay > getIntParam(LOG_fChannelDelay) * 1000)
    {
        // we waited enough, remove pipeline marker
        pCurrentPipeline &= ~PIP_STARTUP;
        pOnDelay = 0;
    }
}

void LogicChannel::processInput(uint8_t iIOIndex) {
    if (iIOIndex == 0)
        return;
    uint16_t lParamBase = (iIOIndex == 1) ? LOG_fE1 : LOG_fE2;
    // we have now an event for an input, first we check, if this input is active
    uint8_t lActive = getByteParam(lParamBase) & BIT_INPUT_MASK;
    if (lActive > 0)
        // this input is we start convert for this input
        startConvert(iIOIndex);
    // this input might also be used for delta conversion in the other input
    uint16_t lOtherParamBase = (iIOIndex == 2) ? LOG_fE1 : LOG_fE2;
    uint8_t lConverter = getByteParam(lOtherParamBase) >> 4;
    if (lConverter & 1)
    {
        // delta convertersion, we start convert for the other input
        startConvert(3 - iIOIndex);
    }
}

// we send an ReadRequest if reading from input 1 should be repeated
void LogicChannel::processRepeatInput1() {
    uint32_t lRepeatTime = getIntParam(LOG_fE1Repeat) * 1000;

    if (delayCheck(pRepeatInput1Delay, lRepeatTime))
    {
        knxRead(IO_Input1);
        pRepeatInput1Delay = millis();
        if (lRepeatTime == 0)
            pCurrentPipeline &= ~PIP_REPEAT_INPUT1;
    }
}

// we send an ReadRequest if reading from input 1 should be repeated
void LogicChannel::processRepeatInput2() {
    uint32_t lRepeatTime = getIntParam(LOG_fE2Repeat) * 1000;

    if (delayCheck(pRepeatInput2Delay, lRepeatTime))
    {
        knxRead(IO_Input2);
        pRepeatInput2Delay = millis();
        if (lRepeatTime == 0)
            pCurrentPipeline &= ~PIP_REPEAT_INPUT2;
    }
}

void LogicChannel::startConvert(uint8_t iIOIndex) {
        pCurrentPipeline |= (iIOIndex == 1) ? PIP_CONVERT_INPUT1 : PIP_CONVERT_INPUT2;
}

void LogicChannel::processConvertInput(uint8_t iIOIndex) {
    uint16_t lParamBase = (iIOIndex == 1) ? LOG_fE1 : LOG_fE2;
    uint8_t lConvert = getByteParam(lParamBase) >> 4;
    bool lValueOut = 0;
    // get input value
    int32_t lValue1In = getInputValue(iIOIndex);
    int32_t lValue2In = 0;
    if (lConvert & 1)
    {
        // in case of delta conversion get the other input value
        lValue2In = getInputValue(3 - iIOIndex);
    }
    uint8_t lDpt = getByteParam(lParamBase + 1);
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
                uint8_t lValue = getByteParam(lParamBase + lScene);
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
                lValueOut = (lValue1In >= getParamByDpt(lDpt, lParamBase + 2)) &&
                            (lValue1In <= getParamByDpt(lDpt, lParamBase + 6));
                break;
            case VAL_InputConvert_DeltaInterval:
                lValueOut = (lValue1In - lValue2In >= getParamForDelta(lDpt, lParamBase + 2)) &&
                            (lValue1In - lValue2In <= getParamForDelta(lDpt, lParamBase + 6));
                break;
            case VAL_InputConvert_Hysterese:
                lValueOut = pCurrentIO & iIOIndex; // retrieve old result, will be send if current value is in hysterese inbervall
                if (lValue1In <= getParamByDpt(lDpt, lParamBase + 2))
                    lValueOut = false;
                if (lValue1In >= getParamByDpt(lDpt, lParamBase + 6))
                    lValueOut = true;
                break;
            case VAL_InputConvert_DeltaHysterese:
                lValueOut = pCurrentIO & iIOIndex; // retrieve old result, will be send if current value is in hysterese inbervall
                if (lValue1In - lValue2In <= getParamForDelta(lDpt, lParamBase + 2))
                    lValueOut = false;
                if (lValue1In - lValue2In >= getParamForDelta(lDpt, lParamBase + 6))
                    lValueOut = true;
                break;

            default:
                // do nothing, wrong converter id
                break;
        }
    }
    // start logic processing for this input
    startLogic(iIOIndex, lValueOut);
}

void LogicChannel::startLogic(uint8_t iIOIndex, bool iValue) {
    // invert input
    bool lValue = iValue;
    uint16_t lParamBase = (iIOIndex == 1) ? LOG_fE1 : (iIOIndex == 2) ? LOG_fE2 : (iIOIndex == 4) ? LOG_fI1 : LOG_fI2;
    if ((getByteParam(lParamBase) & BIT_INPUT_MASK) == 2)
        lValue = !iValue;
    // set according input bit
    pCurrentIO &= ~iIOIndex;
    pCurrentIO |= iIOIndex * lValue;
    // set the validity bit
    pValidActiveIO |= iIOIndex;
    // set the trigger bit
    pTriggerIO |= iIOIndex;
    // finally set the pipeline bit
    pCurrentPipeline |= PIP_LOGIC_EXECUTE;
}

// Processing parametrized logic
void LogicChannel::processLogic() {
    /* Logic execution bit is set from any method which changes input values */
    uint8_t lValidInputs = pValidActiveIO & BIT_INPUT_MASK;
    uint8_t lActiveInputs = (pValidActiveIO >> 4) & BIT_INPUT_MASK;
    uint8_t lCurrentInputs = pCurrentIO & lValidInputs;
    bool lCurrentOuput = ((pCurrentIO & BIT_OUTPUT) == BIT_OUTPUT);
    bool lNewOutput = false;
    bool lValidOutput = false;
    // first deactivate execution in pipeline
    pCurrentPipeline &= ~PIP_LOGIC_EXECUTE;
    // we have to delete all trigger if output pipeline is not started
    bool lOutputSent = false;
    if (getByteParam(LOG_fCalculate) == 0 || lValidInputs == lActiveInputs)
    {
        // we process only if all inputs are valid or the user requested invalid evaluation
        uint8_t lLogic = getByteParam(LOG_fLogic);
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
            uint8_t lTrigger = getByteParam(LOG_fTrigger);
            if ((lTrigger == 0 && lNewOutput != lCurrentOuput) ||
                (lTrigger & pTriggerIO) > 0)
            {
                // set the output value (first delete BIT_OUTPUT and the set the value
                // of lNewOutput)
                pCurrentIO = (pCurrentIO & ~BIT_OUTPUT) | lNewOutput << 4;
                // set the output trigger bit
                pTriggerIO |= BIT_OUTPUT;
                // now we start stairlight processing
                startStairlight(lNewOutput);
                lOutputSent = true;
            }
        }
    }
    // we have to delete all trigger if output pipeline is not started
    if (!lOutputSent)
        pTriggerIO = 0;
}

void LogicChannel::startStairlight(bool iOutput) {
    if (getByteParam(LOG_fOStair))
    {
        if (iOutput)
        {
            // if stairlight is not running yet, we switch first the output to on
            if ((pCurrentPipeline & PIP_STAIRLIGHT) == 0)
                startOnDelay();
            // stairlight should also be switched on
            uint8_t lRetrigger = getByteParam(LOG_fORetrigger);
            if ((pCurrentPipeline & PIP_STAIRLIGHT) == 0 || lRetrigger == 1)
            {
                // stairlight is not running or may be retriggered
                // we init the stairlight timer
                pStairlightDelay = millis();
                pCurrentPipeline |= PIP_STAIRLIGHT;
                startBlink();
            }
        }
        else
        {
            // if stairlight is not running yet, we switch the output to off
            if ((pCurrentPipeline & PIP_STAIRLIGHT) == 0)
                startOffDelay();
            // stairlight should be switched off
            uint8_t lOff = getByteParam(LOG_fOStairOff);
            if (lOff == 1)
            {
                // stairlight might be switched off,
                // we set the timer to 0
                pStairlightDelay = 0;
            }
        }
    }
    else if (iOutput)
    {
        // an output without stairlight is forwarded to switch on processing
        startOnDelay();
    }
    else
    {
        startOffDelay();
    }
}

void LogicChannel::processStairlight() {
    uint8_t lStairTimeBase = getByteParam(LOG_fOTimeBase);
    uint32_t lStairTime = getIntParam(LOG_fOTime);
    uint32_t lTime = lStairTime * cTimeFactors[lStairTimeBase];

    if (delayCheck(pStairlightDelay, lTime))
    {
        // stairlight time is over, we switch off, also potential blinking
        pCurrentPipeline &= ~(PIP_STAIRLIGHT | PIP_BLINK);
        // we start switchOffProcessing
        startOffDelay();
    }
}

void LogicChannel::startBlink() {
    uint32_t lBlinkTime = getIntParam(LOG_fOBlink);
    if (lBlinkTime > 0)
    {
        pBlinkDelay = millis();
        pCurrentPipeline |= PIP_BLINK;
        pCurrentIO |= BIT_OUTPUT;
    }
}

void LogicChannel::processBlink() {
    uint32_t lBlinkTime = getIntParam(LOG_fOBlink) * 100;
    if (delayCheck(pBlinkDelay, lBlinkTime))
    {
        bool lOn = !(pCurrentIO & BIT_OUTPUT);
        if (lOn)
        {
            pCurrentIO |= BIT_OUTPUT;
            startOnDelay();
        }
        else
        {
            pCurrentIO &= ~BIT_OUTPUT;
            startOffDelay();
        }
        pBlinkDelay = millis();
    }
}

// delays the on signal by defined druation
void LogicChannel::startOnDelay() {
    // if on delay is already running, there are options:
    //    1. second on switches immediately on
    //    2. second on restarts delay time
    //    3. an off stops on delay
    uint8_t lOnDelay = getByteParam(LOG_fODelay);
    uint8_t lOnDelayRepeat = (lOnDelay & 96) >> 5;
    if ((pCurrentPipeline & PIP_ON_DELAY) == 0)
    {
        pOnDelay = millis();
        pCurrentPipeline |= PIP_ON_DELAY;
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
                pOnDelay = 0;
                break;
            case VAL_Delay_Extend:
                pOnDelay = millis();
                break;
            default:
                break;
        }
    }
    uint8_t lOnDelayReset = (lOnDelay & 16) >> 4;
    // if requested, this on stops an off delay
    if ((lOnDelayReset > 0) && (pCurrentPipeline & PIP_OFF_DELAY) > 0)
    {
        pCurrentPipeline &= ~PIP_OFF_DELAY;
    }
}

void LogicChannel::processOnDelay() {
    uint32_t lOnDelay = getIntParam(LOG_fODelayOn) * 100;
    if (delayCheck(pOnDelay, lOnDelay))
    {
        // delay time is over, we turn off pipeline
        pCurrentPipeline &= ~PIP_ON_DELAY;
        // we start repeatOnProcessing
        startOutputFilter(true);
    }
}

// delays the off signal by defined druation
void LogicChannel::startOffDelay() {
    // if off delay is already running, there are options:
    //    1. second off switches immediately off
    //    2. second off restarts delay time
    //    3. an on stops off delay
    uint8_t lOffDelay = getByteParam(LOG_fODelay);
    uint8_t lOffDelayRepeat = (lOffDelay & 12) >> 2;
    if ((pCurrentPipeline & PIP_OFF_DELAY) == 0)
    {
        pOffDelay = millis();
        pCurrentPipeline |= PIP_OFF_DELAY;
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
                pOffDelay = 0;
                break;
            case VAL_Delay_Extend:
                pOffDelay = millis();
                break;
            default:
                break;
        }
    }
    uint8_t lOffDelayReset = (lOffDelay & 2) >> 1;
    // if requested, this on stops an off delay
    if ((lOffDelayReset > 0) && (pCurrentPipeline & PIP_ON_DELAY) > 0)
    {
        pCurrentPipeline &= ~PIP_ON_DELAY;
    }
}

void LogicChannel::processOffDelay() {
    uint32_t lOffDelay = getIntParam(LOG_fODelayOff) * 100;
    if (delayCheck(pOffDelay, lOffDelay))
    {
        // delay time is over, we turn off pipeline
        pCurrentPipeline &= ~PIP_OFF_DELAY;
        // we start repeatOffProcessing
        startOutputFilter(false);
    }
}

// Output filter prevents repetition of 0 or 1 values
void LogicChannel::startOutputFilter(bool iOutput) {
    uint8_t lAllow = (getByteParam(LOG_fOOutputFilter) & 96) >> 5;
    bool lLastOutput = (pCurrentIO & BIT_LAST_OUTPUT) > 0;
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
        pCurrentPipeline &= ~(PIP_OUTPUT_FILTER_OFF | PIP_OUTPUT_FILTER_ON);
        pCurrentPipeline |= iOutput ? PIP_OUTPUT_FILTER_ON : PIP_OUTPUT_FILTER_OFF;
        pCurrentIO &= ~BIT_LAST_OUTPUT;
        if (iOutput)
            pCurrentIO |= BIT_LAST_OUTPUT;
    }
}
void LogicChannel::processOutputFilter() {
    if (pCurrentPipeline & PIP_OUTPUT_FILTER_ON)
    {
        startOnOffRepeat(true);
    }
    else if (pCurrentPipeline & PIP_OUTPUT_FILTER_OFF)
    {
        startOnOffRepeat(false);
    }
    pCurrentPipeline &= ~(PIP_OUTPUT_FILTER_OFF | PIP_OUTPUT_FILTER_ON);
}

// starts On-Off-Repeat
void LogicChannel::startOnOffRepeat(bool iOutput) {
    // with repeat, we first process the ouptut and then we repeat the signal
    // if repeat is already active, we wait until next cycle
    if (iOutput)
    {
        if ((pCurrentPipeline & PIP_ON_REPEAT) == 0)
        {
            pRepeatOnOffDelay = millis();
            pCurrentPipeline &= ~PIP_OFF_REPEAT;
            processOutput(iOutput);
            if (getIntParam(LOG_fORepeatOn) > 0)
                pCurrentPipeline |= PIP_ON_REPEAT;
        }
    }
    else
    {
        if ((pCurrentPipeline & PIP_OFF_REPEAT) == 0)
        {
            pRepeatOnOffDelay = millis();
            pCurrentPipeline &= ~PIP_ON_REPEAT;
            processOutput(iOutput);
            if (getIntParam(LOG_fORepeatOff) > 0)
                pCurrentPipeline |= PIP_OFF_REPEAT;
        }
    }
}

void LogicChannel::processOnOffRepeat() {
    uint32_t lRepeat = 0;
    bool lValue;

    if (pCurrentPipeline & PIP_ON_REPEAT)
    {
        lRepeat = getIntParam(LOG_fORepeatOn) * 100;
        lValue = true;
    }
    if (pCurrentPipeline & PIP_OFF_REPEAT)
    {
        lRepeat = getIntParam(LOG_fORepeatOff) * 100;
        lValue = false;
    }

    if (delayCheck(pRepeatOnOffDelay, lRepeat))
    {
        // delay time is over, we repeat the output
        processOutput(lValue);
        // and we restart repeat counter
        pRepeatOnOffDelay = millis();
    }
}

// we trigger all associated internal inputs with the new value
void LogicChannel::processInternalInputs(uint8_t iChannelId, bool iValue)
{
    uint8_t lInput1 = getByteParam(LOG_fI1);
    if (lInput1 > 0)
    {
        uint32_t lFunction1 = getIntParam(LOG_fI1Function);
        if (lFunction1 == (uint32_t)(iChannelId + 1))
        {
            startLogic(BIT_INT_INPUT_1, iValue);
        }
    }
    uint8_t lInput2 = getByteParam(LOG_fI2);
    if (lInput2 > 0)
    {
        uint32_t lFunction2 = getIntParam(LOG_fI2Function);
        if (lFunction2 == (uint32_t)(iChannelId + 1))
        {
            startLogic(BIT_INT_INPUT_2, iValue);
        }
    }
}

// process the output itself
void LogicChannel::processOutput(bool iValue) {
    LogicChannel::sLogic->processAllInternalInputs(this, iValue);
    if (iValue)
    {
        uint8_t lOn = getByteParam(LOG_fOOn);
        switch (lOn)
        {
            case VAL_Out_Constant:
                writeConstantValue(LOG_fOOnDpt1);
                break;
            case VAL_Out_ValE1:
                writeParameterValue(IO_Input1);
                break;
            case VAL_Out_ValE2:
                writeParameterValue(IO_Input2);
                break;
            case VAL_Out_ReadRequest:
                knxRead(IO_Output);
                break;
            case VAL_Out_ResetDevice:
                knxResetDevice(LOG_fOOnDpt1);
                break;
            case VAL_Out_Buzzer:
#ifndef __linux__
                //digitalWrite(BUZZER_PIN, HIGH);
                tone(BUZZER_PIN, BUZZER_FREQ);
#endif
                break;
            default:
                // there is no output parametrized
                break;
        }
    }
    else
    {
        uint8_t lOff = getByteParam(LOG_fOOff);
        switch (lOff)
        {
            case VAL_Out_Constant:
                writeConstantValue(LOG_fOOffDpt1);
                break;
            case VAL_Out_ValE1:
                writeParameterValue(IO_Input1);
                break;
            case VAL_Out_ValE2:
                writeParameterValue(IO_Input2);
                break;
            case VAL_Out_ReadRequest:
                knxRead(IO_Output);
                break;
            case VAL_Out_ResetDevice:
                knxResetDevice(LOG_fOOffDpt1);
                break;
            case VAL_Out_Buzzer:
#ifndef __linux__
                //digitalWrite(BUZZER_PIN, LOW);
                noTone(BUZZER_PIN);
#endif
                break;
            default:
                // there is no output parametrized
                break;
        }
    }
    // any valid output removes all input trigger
    pTriggerIO = 0;
}

bool LogicChannel::checkDpt(uint8_t iIOIndex, uint8_t iDpt) {
    uint16_t lParam;
    switch (iIOIndex)
    {
    case IO_Input1:
        lParam = LOG_fE1Dpt;
        break;
    case IO_Input2:
        lParam = LOG_fE2Dpt;
        break;
    case IO_Output:
        lParam = LOG_fODpt;
        break;
    default:
        return false;
        break;
    }
    uint8_t lDpt = getByteParam(lParam);
    return lDpt == iDpt;
}

bool LogicChannel::readOneInputFromEEPROM(uint8_t iIOIndex)
{
    EepromManager *lEEPROM = sLogic->getEEPROM();
    // first check, if EEPROM contains valid values
    if (!lEEPROM->isValid())
        return false;
    // Now check, if the DPT for requested KO is valid
    // DPT might have changed due to new programming after last save
    uint16_t lAddress = (SAVE_BUFFER_START_PAGE + 1) * 32 + mChannelId * 2 + iIOIndex - 1;
    lEEPROM->prepareRead(lAddress, 1);
    uint8_t lSavedDpt = Wire.read();
    if (!checkDpt(iIOIndex, lSavedDpt)) return false;

    // if the dpt is ok, we get the ko value
    lAddress = (SAVE_BUFFER_START_PAGE + 5) * 32 + mChannelId * 8 + (iIOIndex - 1) * 4;
    GroupObject *lKo = getKo(iIOIndex);
    lEEPROM->prepareRead(lAddress, lKo->valueSize());
    int lIndex = 0;
    while (Wire.available() && lIndex < 4)
        lKo->valueRef()[lIndex++] = Wire.read();
    return true;
}

void LogicChannel::writeSingleDptToEEPROM(uint8_t iIOIndex) {
    uint8_t lDpt = 0xFF;
    if (isInputActive(iIOIndex))
    {
        // now get input default value
        uint8_t lParInput = getByteParam(iIOIndex == 1 ? LOG_fE1Default : LOG_fE2Default);
        if (lParInput & VAL_InputDefault_EEPROM) {
            // if the default is EEPROM, we get correct dpt
            lDpt = getByteParam(iIOIndex == 1 ? LOG_fE1Dpt : LOG_fE2Dpt);
        }
    }
    Wire.write(lDpt);
}

bool LogicChannel::prepareChannel() {
    bool lResult = false;
    bool lInput1EEPROM = false;
    bool lInput2EEPROM = false;
    if (getByteParam(LOG_fLogic) > 0)
    {
        // function is active, we process input presets
        // external input 1
        if (isInputActive(IO_Input1))
        {
            // input is active, we set according flag
            pValidActiveIO |= BIT_EXT_INPUT_1 << 4;
            // now set input default value
            uint8_t lParInput = getByteParam(LOG_fE1Default);
            // shoud default be fetched from EEPROM
            if (lParInput & VAL_InputDefault_EEPROM) {
                lInput1EEPROM = readOneInputFromEEPROM(IO_Input1);
                if (!lInput1EEPROM) {
                    lParInput &= ~VAL_InputDefault_EEPROM;
                    lResult = true;
                }
            }
            switch (lParInput)
            {
                case VAL_InputDefault_Read:
                    /* to read immediately we activate repeated read pipeline with 0 delay */
                    pRepeatInput1Delay = 0;
                    pCurrentPipeline |= PIP_REPEAT_INPUT1;
                    break;

                case VAL_InputDefault_False:
                    /* we clear bit for E1 and mark this value as valid */
                    startLogic(BIT_EXT_INPUT_1, false);
                    break;

                case VAL_InputDefault_True:
                    /* we set bit for E1 and mark this value as valid */
                    startLogic(BIT_EXT_INPUT_1, true);
                    break;

                default:
                    /* do nothing, value is invalid */
                    break;
            }
        }
        // external input 2
        if (isInputActive(IO_Input2))
        {
            // input is active, we set according flag
            pValidActiveIO |= BIT_EXT_INPUT_2 << 4;
            uint8_t lParInput = getByteParam(LOG_fE2Default);
            // shoud default be fetched from EEPROM
            if (lParInput & VAL_InputDefault_EEPROM) {
                lInput2EEPROM = readOneInputFromEEPROM(IO_Input2);
                if (!lInput2EEPROM) {
                    lParInput &= ~VAL_InputDefault_EEPROM;
                    lResult = true;
                }
            }
            switch (lParInput)
            {
                case VAL_InputDefault_Read:
                    /* to read immediately we activate repeated read pipeline with 0 delay */
                    pRepeatInput2Delay = 0;
                    pCurrentPipeline |= PIP_REPEAT_INPUT2;
                    break;

                case VAL_InputDefault_False:
                    /* we clear bit for E2 and mark this value as valid */
                    startLogic(BIT_EXT_INPUT_2, false);
                    break;

                case VAL_InputDefault_True:
                    /* we set bit for E2 and mark this value as valid */
                    startLogic(BIT_EXT_INPUT_2, true);
                    break;

                default:
                    /* do nothing, value is invalid */
                    break;
            }
        }
        // internal input 1
        // first check, if input is active
        uint8_t lIsActive = getByteParam(LOG_fI1);
        if (lIsActive > 0)
        {
            // input is active, we set according flag
            pValidActiveIO |= BIT_INT_INPUT_1 << 4;
        }
        // internal input 2
        // first check, if input is active
        lIsActive = getByteParam(LOG_fI2);
        if (lIsActive > 0)
        {
            // input is active, we set according flag
            pValidActiveIO |= BIT_INT_INPUT_2 << 4;
        }
        // we set the startup delay
        startStartup();
        // we trigger input processing, if there are values from EEPROM
        if (lInput1EEPROM) processInput(IO_Input1);
        if (lInput2EEPROM) processInput(IO_Input2);
    }
    return lResult;
}

void LogicChannel::loop()
{
    if (!knx.configured())
        return;

    if (pCurrentPipeline & PIP_STARTUP)
    {
        processStartup();
    }
    else if (pCurrentPipeline > 0)
    {
        // repeat input pipeline
        if (pCurrentPipeline & PIP_REPEAT_INPUT1)
            processRepeatInput1();
        if (pCurrentPipeline & PIP_REPEAT_INPUT2)
            processRepeatInput2();
        // convert input pipeline
        if (pCurrentPipeline & PIP_CONVERT_INPUT1)
        {
            pCurrentPipeline &= ~PIP_CONVERT_INPUT1;
            processConvertInput(IO_Input1);
        }
        if (pCurrentPipeline & PIP_CONVERT_INPUT2)
        {
            pCurrentPipeline &= ~PIP_CONVERT_INPUT2;
            processConvertInput(IO_Input2);
        }
        // Logic execution pipeline
        if (pCurrentPipeline & PIP_LOGIC_EXECUTE)
            processLogic();
        // stairlight pipeline
        if (pCurrentPipeline & PIP_STAIRLIGHT)
            processStairlight();
        // blink pipeline
        if (pCurrentPipeline & PIP_BLINK)
            processBlink();
        // On delay pipeline
        if (pCurrentPipeline & PIP_ON_DELAY)
            processOnDelay();
        // Off delay pipeline
        if (pCurrentPipeline & PIP_OFF_DELAY)
            processOffDelay();
        // Output Filter pipeline
        if (pCurrentPipeline & (PIP_OUTPUT_FILTER_ON | PIP_OUTPUT_FILTER_OFF))
            processOutputFilter();
        // On/Off repeat pipeline
        if (pCurrentPipeline & (PIP_ON_REPEAT | PIP_OFF_REPEAT))
            processOnOffRepeat();
    }
}
