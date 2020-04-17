#include "Logic.h"
#include "Helper.h"
#include "Hardware.h"
#include "sun.h"

uint8_t Logic::sMagicWord[] = {0xAE, 0x49, 0xD2, 0x9F};
tm Logic::sDateTime = {0};
uint8_t Logic::sTimeOk = 0;

// callbacks have to be static members
void Logic::onInputKoHandler(GroupObject &iKo) {
    LogicChannel::sLogic->processInputKo(iKo);
}

void Logic::onBeforeRestartHandler()
{
    printDebug("logic before Restart called\n");
    LogicChannel::sLogic->writeAllInputsToEEPROMFacade();
}

void Logic::onBeforeTableUnloadHandler(TableObject & iTableObject, LoadState & iNewState)
{
    static uint32_t sLastCalled = 0;
    printDebug("Table changed called with state %d\n", iNewState);

    if (iNewState == 0)
    {
        printDebug("Table unload called\n");
        if (sLastCalled == 0 || delayCheck(sLastCalled, 10000))
        {
            LogicChannel::sLogic->writeAllInputsToEEPROMFacade();
            sLastCalled = millis();
        }
    }
}

void Logic::onSafePinInterruptHandler()
{
    LogicChannel::sLogic->mSaveInterruptTimestamp = millis();
}

tm* Logic::getDateTime() {
    return &sDateTime;
}

Logic::Logic()
{
    LogicChannel::sLogic = this;
    sDateTime.tm_year = 120;
    sDateTime.tm_mon = 0;
    sDateTime.tm_mday = 1;
    sDateTime.tm_wday = 3;
    mktime(&sDateTime);
    mTimeDelay = millis();
}

Logic::~Logic()
{
}

LogicChannel *Logic::getChannel(uint8_t iChannelId) {
    return mChannel[iChannelId];
}

uint8_t Logic::getChannelId(LogicChannel *iChannel) {
    uint8_t lResult = -1;
    for (uint8_t lIndex = 0; lIndex < mNumChannels; lIndex++)
    {
        if (mChannel[lIndex] == iChannel) {
            lResult = lIndex;
            break;
        }
    }
    return lResult;
}

bool Logic::prepareChannels() {
    bool lResult = false;
    for (uint8_t lIndex = 0; lIndex < mNumChannels; lIndex++)
    {
        lResult = lResult || mChannel[lIndex]->prepareChannel();
    }
    return lResult;
}

// we trigger all associated internal inputs with the new value
void Logic::processAllInternalInputs(LogicChannel *iChannel, bool iValue)
{
    // search for any internal input associated to this channel
    for (uint8_t lIndex = 0; lIndex < mNumChannels; lIndex++)
    {
        LogicChannel *lChannel = mChannel[lIndex];
        uint8_t lChannelId = getChannelId(iChannel);
        lChannel->processInternalInputs(lChannelId, iValue);
    }
}

// EEPROM handling
// We assume at max 128 channels, each channel 2 inputs, each input max 4 bytes (value) and 1 byte (DPT) = 128 * 2 * (4 + 1) = 1280 bytes to write
// So we use 40 Pages for data and one (first) page for aditional information (metadata).
// The DPT list is written startig with page 1 (address 32 = 0x20). It is written at device startup and is not timing critical.
// We have 256 Bytes in 16 x 16 byte blocks (8 pages), takes 16 * 5 ms = 80 ms, just at startup time and just if necessary.
// Writing data itself is timing critical during power failure.
// At first, magic word at address 12 = 0x0C is deleted (5 ms).
// The data itself is written in 16 byte blocks, each 5 ms means 1024 / 16 * 5 ms = 64 * 5 ms = 320 ms write time, starting at page 9, address 160 = 0xA0.
// Finally, we write magic word at Address 12 again as an ack, that all data was successfully written (5 ms)
// The resulting write time is at max 5 + 320 + 5 = 370 ms
// Bytes of the first page (page 0) might be used differently in future
// For inputs, which are not set as "store in memory", we write a dpt 0xFF
void Logic::writeAllDptToEEPROM()
{
    if (mLastWriteToEEPROM > 0 && delayCheck(mLastWriteToEEPROM, 10000))
    {
        println("writeAllDptToEEPROM called repeatedly within 10 seconds, skipped!");
        return;
    }
    mLastWriteToEEPROM = millis();

    // prepare initialization
    uint16_t lAddress = (SAVE_BUFFER_START_PAGE + 1) * 32; // begin of DPT memory
    // start writing all dpt. For inputs, which should not be saved, we write a dpt 0xFF
    for (uint8_t lIndex = 0; lIndex < mNumChannels; lIndex++)
    {
        mEEPROM->beginPage(lAddress);
        mChannel[lIndex]->writeSingleDptToEEPROM(IO_Input1);
        lAddress++;
        mChannel[lIndex]->writeSingleDptToEEPROM(IO_Input2);
        lAddress++;
        if (lAddress % 16 == 0)
            mEEPROM->endPage();
    }
    mEEPROM->endPage();
}

void Logic::writeAllInputsToEEPROM()
{
    if (mLastWriteToEEPROM > 0 && delayCheck(mLastWriteToEEPROM, 10000))
    {
        println("writeAllInputsToEEPROM called repeatedly within 10 seconds, skipped!");
        return;
    }
    mLastWriteToEEPROM = millis();

    // prepare initialization
    mEEPROM->beginWriteSession();

    //Begin write of KO values
    uint16_t lAddress = (SAVE_BUFFER_START_PAGE + 9) * 32; // begin of KO value memory
    // for (uint8_t i = 0; i < 10; i++)
    for (uint8_t lChannel = 0; lChannel < mNumChannels; lChannel++)
    {
        mEEPROM->beginPage(lAddress);
        GroupObject *lKo = LogicChannel::getKoForChannel(IO_Input1, lChannel);
        mEEPROM->write4Bytes(lKo->valueRef(), lKo->valueSize());
        lAddress += 4;
        lKo = LogicChannel::getKoForChannel(IO_Input2, lChannel);
        mEEPROM->write4Bytes(lKo->valueRef(), lKo->valueSize());
        lAddress += 4;
        if (lAddress % 16 == 0)
            mEEPROM->endPage();
    }
    mEEPROM->endPage();

    // as a last step we write magic number back
    // this is also the ACK, that writing was successfull
    mEEPROM->endWriteSession();
}

void Logic::writeAllInputsToEEPROMFacade() {
    uint32_t lTime = millis();
    writeAllInputsToEEPROM(); 
    lTime = millis() - lTime;
    print("WriteAllInputsToEEPROM took: ");
    println(lTime);
}

// on input level, all dpt>1 values are converted to bool by the according converter
void Logic::processInputKo(GroupObject &iKo)
{
    if (iKo.asap() == LOG_KoTime) {
        struct tm lTmp = iKo.value(getDPT(VAL_DPT_10));
        sDateTime.tm_sec = lTmp.tm_sec;
        sDateTime.tm_min = lTmp.tm_min;
        sDateTime.tm_hour = lTmp.tm_hour;
        mktime(&sDateTime);
        mTimeDelay = millis();
        sTimeOk |= 1;
    } else if (iKo.asap() == LOG_KoDate) {
        struct tm lTmp = iKo.value(getDPT(VAL_DPT_11));
        lTmp.tm_year -= 1900;
        lTmp.tm_mon -= 1;
        // we have to check, if some date dependant calculations have to be done
        // in case of date changes
        if (lTmp.tm_year != sDateTime.tm_year) {
            mEasterTick = -1; // triggers easter calculation
            mDayTick = -1; // triggers sunrise/sunset calculation
        } else if (lTmp.tm_mon != sDateTime.tm_mon) {
            mDayTick = -1; // triggers sunrise/sunset calculation
        } else if (lTmp.tm_mday != sDateTime.tm_mday) {
            mDayTick = -1; // triggers sunrise/sunset calculation
        }
        sDateTime.tm_mday = lTmp.tm_mday;
        sDateTime.tm_mon = lTmp.tm_mon;
        sDateTime.tm_year = lTmp.tm_year;
        mktime(&sDateTime);
        mTimeDelay = millis();
        sTimeOk |= 2;
    } else if (iKo.asap() >= LOG_KoOffset && iKo.asap() < LOG_KoOffset + mNumChannels * LOG_KoBlockSize) {
        uint16_t lKoNumber = iKo.asap() - LOG_KoOffset;
        uint8_t lChannelId = lKoNumber / LOG_KoBlockSize;
        uint8_t lIOIndex = lKoNumber % LOG_KoBlockSize + 1;
        LogicChannel *lChannel = mChannel[lChannelId];
        lChannel->processInput(lIOIndex);
    }
}

void Logic::processInterrupt(bool iForce)
{
    if (mSaveInterruptTimestamp > 0 || iForce)
    {
        if (!iForce) printDebug("Logic: SAVE-Interrupt processing started after %lu ms\n", millis() - mSaveInterruptTimestamp);
        mSaveInterruptTimestamp = millis();
        // If Interrupt happens during i2c read we try to finish last read first
        while (Wire.available())
            Wire.read();
        // now we write everything to EEPROM
        writeAllInputsToEEPROM();
        printDebug("Logic: SAVE-Interrupt processing duration %lu ms\n", millis() - mSaveInterruptTimestamp);
        mSaveInterruptTimestamp = 0;
    }
}

bool Logic::processDiagnoseCommand(char* cBuffer) {
    bool lResult = false;
    //diagnose is interactive and reacts on commands
    switch (cBuffer[0]) {
        case 'l': {
            // Command l<nn>: Logic inputs and output of last execution
            // find channel and dispatch
            uint8_t lIndex = (cBuffer[1] - '0') * 10 + cBuffer[2] - '0' - 1;
            lResult = mChannel[lIndex]->processDiagnoseCommand(cBuffer);
            break;
        }
        case 't': {
            // return internal time (might differ from external
            sprintf(cBuffer, "%02d:%02d:%02d %02d.%02d", sDateTime.tm_hour, sDateTime.tm_min, sDateTime.tm_sec, sDateTime.tm_mday, sDateTime.tm_mon + 1);
            lResult = true;
            break;
        }
        case 'r': {
            sprintf(cBuffer, "R%02d:%02d S%02d:%02d", mSunrise.hour, mSunrise.minute, mSunset.hour, mSunset.minute);
            lResult = true;
            break;
        }
        case 'o': {
            // calculate easter date
            sprintf(cBuffer, "O%02d.%02d", mEaster.day, mEaster.month);
            lResult = true;
            break;
        }
        default:
            lResult = false;
            break;
    }
    return lResult;
}

void Logic::onSavePinInterruptHandler() {
    mSaveInterruptTimestamp = millis();
}

void Logic::beforeRestartHandler()
{
    printDebug("logic before Restart called\n");
    writeAllInputsToEEPROMFacade();
}

void Logic::beforeTableUnloadHandler(TableObject & iTableObject, LoadState & iNewState)
{
    static uint32_t sLastCalled = 0;
    printDebug("Table changed called with state %d\n", iNewState);

    if (iNewState == 0)
    {
        printDebug("Table unload called\n");
        if (sLastCalled == 0 || delayCheck(sLastCalled, 10000))
        {
            writeAllInputsToEEPROMFacade();
            sLastCalled = millis();
        }
    }
}

void Logic::debug() {
    printDebug("Logik-LOG_ChannelsFirmware (in Firmware): %d\n", LOG_ChannelsFirmware);
    printDebug("Logik-gNumChannels (in knxprod):  %d\n", mNumChannels);
    printDebug("Aktuelle Zeit: %s", asctime(&sDateTime));
    // Test i2c failure
    // we start an i2c read i.e. for EEPROM
    // prepareReadEEPROM(4711, 20);
    // digitalWrite(LED_YELLOW_PIN, HIGH);
    // delay(10000);
    // digitalWrite(LED_YELLOW_PIN, LOW);
}

void Logic::setup(bool iSaveSupported) {
    // Wire.end();   // seems to end hangs on I2C bus
    // Wire.begin(); // we use I2C in logic, so we setup the bus. It is not critical to setup it more than once
    if (knx.configured())
    {
        // setup channels, not possible in constructor, because knx is not configured there
        // get number of channels from knxprod
        mNumChannels = knx.paramByte(LOG_NumChannels);
        if (LOG_ChannelsFirmware < mNumChannels)
        {
            printDebug("FATAL: Firmware compiled for %d channels, but knxprod needs %d channels!\n", LOG_ChannelsFirmware, mNumChannels);
            knx.platform().fatalError();
        }
        for (uint8_t lIndex = 0; lIndex < mNumChannels; lIndex++)
        {
            mChannel[lIndex] = new LogicChannel(lIndex);
        }
        // this should be changed if we ever use multiple instances of logic
        mEEPROM = new EepromManager(SAVE_BUFFER_START_PAGE, SAVE_BUFFER_NUM_PAGES, sMagicWord);
        // setup buzzer
#ifndef __linux__
#ifdef BUZZER_PIN
        pinMode(BUZZER_PIN, OUTPUT);
#endif
#endif
        // we set just a callback if it is not set from a potential caller
        if (GroupObject::classCallback() == 0) GroupObject::classCallback(Logic::onInputKoHandler);
        if (mEEPROM->isValid()) {
            printDebug("EEPROM contains valid KO inputs\n");
        } else {
            printDebug("EEPROM does NOT contain valid data\n");
        }
        // we store some input values in case of restart or ets programming
        if (knx.getBeforeRestartCallback() == 0) knx.addBeforeRestartCallback(onBeforeRestartHandler);
        if (TableObject::getBeforeTableUnloadCallback() == 0) TableObject::addBeforeTableUnloadCallback(onBeforeTableUnloadHandler);
        // set interrupt for poweroff handling
#ifdef SAVE_INTERRUPT_PIN
        if (iSaveSupported) {
            attachInterrupt(digitalPinToInterrupt(SAVE_INTERRUPT_PIN), onSafePinInterruptHandler, FALLING);
        }
#endif
        if (prepareChannels())
            writeAllDptToEEPROM();
    }
}

void Logic::processTime() {
    if (delayCheck(mTimeDelay, 1000)) {
        mTimeDelay = millis();
        sDateTime.tm_sec += 1;
        mktime(&sDateTime);
        if (sTimeOk == 3) {
            if (mMinuteTick != sDateTime.tm_min) {
                mIndicateTimerInput = true;
                // just call once a minute
                mMinuteTick = sDateTime.tm_min;
            }
            if (mDayTick != sDateTime.tm_mday) {
                calculateSunriseSunset();
                mDayTick = sDateTime.tm_mday;
            }
            if (mEasterTick != sDateTime.tm_year) {
                calculateEaster();
                mEasterTick = sDateTime.tm_year;
            }
        }
    }
}

void Logic::calculateSunriseSunset() {
    double rise, set;
    // sunrise/sunset calculation
    sun_rise_set(sDateTime.tm_year + 1900, sDateTime.tm_mon + 1, sDateTime.tm_mday,
                 8.639751, 49.310209,
                 &rise, &set);
    double lTmp;
    mSunrise.minute = round(modf(rise, &lTmp) * 60.0);
    mSunrise.hour = lTmp + 2;
    mSunset.minute = round(modf(set, &lTmp) * 60.0);
    mSunset.hour = lTmp + 2;
}

void Logic::calculateEaster() {
    getEaster(sDateTime.tm_year + 1900, &mEaster.day, &mEaster.month);
}

void Logic::loop()
{
    if (!knx.configured())
        return;

    processInterrupt();
    processTime();

    // we loop on all channels an execute pipeline
    for (uint8_t lIndex = 0; lIndex < mNumChannels; lIndex++)
    {
        LogicChannel *lChannel = mChannel[lIndex];
        if (mIndicateTimerInput)
            lChannel->startTimerInput();
        lChannel->loop();
        knx.loop();
    }
    mIndicateTimerInput = false;
}

EepromManager *Logic::getEEPROM() {
    return mEEPROM;
}
