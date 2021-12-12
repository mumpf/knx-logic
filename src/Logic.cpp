#include "Logic.h"
#include "Helper.h"
#include "Hardware.h"
#include "Timer.h"
#include "TimerRestore.h"
#include "PCA9632.h"
#ifdef WATCHDOG
#include <Adafruit_SleepyDog.h>
uint32_t gWatchdogDelay;
uint8_t gWatchdogResetCause;
#endif


uint8_t Logic::sMagicWord[] = {0xAE, 0x49, 0xD2, 0x9F};
Timer &Logic::sTimer = Timer::instance(); // singleton
TimerRestore &Logic::sTimerRestore = TimerRestore::instance(); // singleton

char Logic::sDiagnoseBuffer[16] = {0};
sLoopCallbackParams Logic::sLoopCallbacks[5] = {nullptr};
uint8_t Logic::sNumLoopCallbacks = 0;

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

void Logic::addLoopCallback(loopCallback iLoopCallback, void *iThis) {
    sLoopCallbackParams lParams;
    lParams.callback = iLoopCallback;
    lParams.instance = iThis;
    Logic::sLoopCallbacks[sNumLoopCallbacks++] = lParams;
}

Logic::Logic()
{
    LogicChannel::sLogic = this;
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

void Logic::processReadRequests() {
    static bool sCalled = false;
    static uint32_t sDelay = 19000;

    // the following code should be called only once after initial startup delay 
    if (!sCalled) {
        if (knx.paramByte(LOG_VacationRead) & LOG_VacationReadMask)
        {
            knx.getGroupObject(LOG_KoVacation).requestObjectRead();
        }
        sCalled = true;
    }
    // date and time are red from bus every minute until a response is received
    if ((knx.paramByte(LOG_ReadTimeDate) & LOG_ReadTimeDateMask))
    {
        eTimeValid lValid = sTimer.isTimerValid();
        if (delayCheck(sDelay, 30000) && lValid != tmValid)
        {
            sDelay = millis();
            if (lValid != tmMinutesValid)
                knx.getGroupObject(LOG_KoTime).requestObjectRead();
            if (lValid != tmDateValid)
                knx.getGroupObject(LOG_KoDate).requestObjectRead();
        }
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
#ifdef I2C_EEPROM_DEVICE_ADDRESSS
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
#endif
}

void Logic::writeAllInputsToEEPROM()
{
#ifdef I2C_EEPROM_DEVICE_ADDRESSS

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
#endif
}

void Logic::writeAllInputsToEEPROMFacade() {
#ifdef I2C_EEPROM_DEVICE_ADDRESSS
    uint32_t lTime = millis();
    writeAllInputsToEEPROM(); 
    lTime = millis() - lTime;
    print("WriteAllInputsToEEPROM took: ");
    println(lTime);
#else
    print("No write to EEPROM, not available!");
#endif
}

// on input level, all dpt > 1 values are converted to bool by the according converter
void Logic::processInputKo(GroupObject &iKo)
{
    if (iKo.asap() == LOG_KoTime) {
        struct tm lTmp = iKo.value(getDPT(VAL_DPT_10));
        sTimer.setTimeFromBus(&lTmp);
    } else if (iKo.asap() == LOG_KoDate) {
        struct tm lTmp = iKo.value(getDPT(VAL_DPT_11));
        sTimer.setDateFromBus(&lTmp);
    } else if (iKo.asap() == LOG_Diagnose) {
        processDiagnoseCommand(iKo);
    }
#ifdef BUZZER_PIN
    else if (iKo.asap() == LOG_KoBuzzerLock) {
        // turn off buzzer in case of lock
        if (iKo.value(getDPT(VAL_DPT_1)))
            noTone(BUZZER_PIN);
    }
#endif
#ifdef I2C_RGBLED_DEVICE_ADDRESS
    else if (iKo.asap() == LOG_KoLedLock)
    {
        // turn off LED in case of lock
        if (iKo.value(getDPT(VAL_DPT_1)))
            PCA9632_SetColor(0, 0, 0);
    }
#endif
    else if (iKo.asap() >= LOG_KoOffset && iKo.asap() < LOG_KoOffset + mNumChannels * LOG_KoBlockSize)
    {
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
        // while (Wire.available())
        //     Wire.read();
        // now we write everything to EEPROM
        writeAllInputsToEEPROM();
        printDebug("Logic: SAVE-Interrupt processing duration %lu ms\n", millis() - mSaveInterruptTimestamp);
        mSaveInterruptTimestamp = 0;
    }
}

char *Logic::initDiagnose(GroupObject &iKo) {
    memcpy(sDiagnoseBuffer, iKo.valueRef(), 14);
    return sDiagnoseBuffer;
}

char *Logic::getDiagnoseBuffer() {
    return sDiagnoseBuffer;
}

bool Logic::processDiagnoseCommand() {
    bool lResult = false;
    //diagnose is interactive and reacts on commands
    switch (sDiagnoseBuffer[0]) {
        case 's': {
            // Command s: Number of save-Interupts (= false-save)
            snprintf(sDiagnoseBuffer, 15, "SAVE %5d", mSaveInterruptCount);
            lResult = true;
            break;
        }
        case 'l': {
            // Command l<nn>: Logic inputs and output of last execution
            // find channel and dispatch
            uint8_t lIndex = (sDiagnoseBuffer[1] - '0') * 10 + sDiagnoseBuffer[2] - '0' - 1;
            lResult = mChannel[lIndex]->processDiagnoseCommand(sDiagnoseBuffer);
            break;
        }
        case 't': {
            // return internal time (might differ from external
            uint8_t lHour = sTimer.getHour();
            uint8_t lMinute = sTimer.getMinute();
            uint8_t lSecond = sTimer.getSecond();
            uint8_t lDay = sTimer.getDay();
            uint8_t lMonth = sTimer.getMonth();
            // this if prevents stupid warnings
            if (lHour < 24 && lMinute < 60 && lSecond < 60 && lDay < 32 && lMonth < 13)
                snprintf(sDiagnoseBuffer, 15, "%02d:%02d:%02d %02d.%02d", lHour, lMinute, lSecond, lDay, lMonth);
            lResult = true;
            break;
        }
        case 'r': {
            // return sunrise and sunset
            sTime *lSunrise = sTimer.getSunInfo(SUN_SUNRISE);
            sTime *lSunset = sTimer.getSunInfo(SUN_SUNSET);
            // this if prevents stupid warnings
            if (lSunrise->hour < 24 && lSunrise->minute < 60 && lSunset->hour < 24 && lSunset->minute < 60)
                snprintf(sDiagnoseBuffer, 15, "R%02d:%02d S%02d:%02d", lSunrise->hour, lSunrise->minute, lSunset->hour, lSunset->minute);
            lResult = true;
            break;
        }
        case 'o': {
            // calculate easter date
            snprintf(sDiagnoseBuffer, 15, "O%02d.%02d", sTimer.getEaster()->day, sTimer.getEaster()->month);
            lResult = true;
            break;
        }
        case 'w': {
            // Watchdog information
#ifdef WATCHDOG
            if (((knx.paramByte(LOG_Watchdog) & LOG_WatchdogMask) >> LOG_WatchdogShift) == 0)
            {
                snprintf(sDiagnoseBuffer, 15, "WD not active");
            }
            else if (gWatchdogResetCause & WDT_RCAUSE_EXT)
            {
                snprintf(sDiagnoseBuffer, 15, "WD reset button");
            }
            else if (gWatchdogResetCause & WDT_RCAUSE_POR)
            {
                snprintf(sDiagnoseBuffer, 15, "WD bus reset");
            }
            else if (gWatchdogResetCause & WDT_RCAUSE_SYSTEM)
            {
                snprintf(sDiagnoseBuffer, 15, "WD ETS program");
            }
            else if (gWatchdogResetCause & WDT_RCAUSE_WDT)
            {
                snprintf(sDiagnoseBuffer, 15, "WD watchdog");
            }
            else
            {
                snprintf(sDiagnoseBuffer, 15, "WD unknown");
            }
#else
            snprintf(sDiagnoseBuffer, 15, "WD no compile");
#endif
            lResult = true;
            break;
        }
        default:
            lResult = false;
            break;
    }
    return lResult;
}

void Logic::processDiagnoseCommand(GroupObject &iKo) {
    // this method is called as soon as iKo is changed
    // an external change is expected
    // because this iKo also is changed within this method,
    // the method is called again. This might result in
    // an endless loop. This is prevented by the isCalled pattern.
    static bool sIsCalled = false;
    if (!sIsCalled)
    {
        sIsCalled = true;
        //diagnose is interactive and reacts on commands
        initDiagnose(iKo);
        if (processDiagnoseCommand())
            outputDiagnose(iKo);
        sIsCalled = false;
    }
};

void Logic::outputDiagnose(GroupObject &iKo) {
    sDiagnoseBuffer[15] = 0;
    iKo.value(sDiagnoseBuffer, getDPT(VAL_DPT_16));
    printDebug("Diagnose: %s\n", sDiagnoseBuffer);
}

void Logic::onSavePinInterruptHandler() {
    mSaveInterruptCount += 1;
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

    // printDebug("Aktuelle Zeit: %s", sTimer.getTimeAsc());
    sTimer.debug();
    // sTimer.debugHolidays();
    // Test i2c failure
    // we start an i2c read i.e. for EEPROM
    // prepareReadEEPROM(4711, 20);
    // delay(10000);
}

void Logic::setup(bool iSaveSupported) {
    // Wire.end();   // seems to end hangs on I2C bus
    // Wire.begin(); // we use I2C in logic, so we setup the bus. It is not critical to setup it more than once
#ifdef WATCHDOG
    if ((knx.paramByte(LOG_Watchdog) & LOG_WatchdogMask) >> LOG_WatchdogShift) {
        // used for Diagnose command
        gWatchdogResetCause = Watchdog.resetCause();
        // setup watchdog to prevent endless loops
        int lWatchTime = Watchdog.enable(16384, false);
        printDebug("Watchdog started with a watchtime of %i Seconds\n", lWatchTime / 1000);
    }
#endif
    if (knx.configured())
    {
        // setup channels, not possible in constructor, because knx is not configured there
        // get number of channels from knxprod
        mNumChannels = knx.paramByte(LOG_NumChannels);
        if (LOG_ChannelsFirmware < mNumChannels)
        {
            char lErrorText[80];
            sprintf(lErrorText, "FATAL: Firmware compiled for %d channels, but knxprod needs %d channels!\n", LOG_ChannelsFirmware, mNumChannels);
            fatalError(FATAL_LOG_WRONG_CHANNEL_COUNT, lErrorText);
        }
        for (uint8_t lIndex = 0; lIndex < mNumChannels; lIndex++)
        {
            mChannel[lIndex] = new LogicChannel(lIndex);
        }
        // this should be changed if we ever use multiple instances of logic
        mEEPROM = new EepromManager(SAVE_BUFFER_START_PAGE, SAVE_BUFFER_NUM_PAGES, sMagicWord);
        // setup buzzer
#ifdef BUZZER_PIN
        pinMode(BUZZER_PIN, OUTPUT);
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
        float lLat = LogicChannel::getFloat(knx.paramData(LOG_Latitude));
        float lLon = LogicChannel::getFloat(knx.paramData(LOG_Longitude));
        // sTimer.setup(8.639751, 49.310209, 1, true, 0xFFFFFFFF);
        uint8_t lTimezone = (knx.paramByte(LOG_Timezone) & LOG_TimezoneMask) >> LOG_TimezoneShift;
        bool lUseSummertime = (knx.paramByte(LOG_UseSummertime) & LOG_UseSummertimeMask);
        sTimer.setup(lLon, lLat, lTimezone, lUseSummertime, knx.paramInt(LOG_Neujahr));
        // for TimerRestore we prepare all Timer channels
        for (uint8_t lIndex = 0; lIndex < mNumChannels; lIndex++)
        {
            LogicChannel *lChannel = mChannel[lIndex];
            lChannel->startTimerRestoreState();
        }
    }
}

void Logic::loop()
{
#ifdef WATCHDOG
    if (delayCheck(gWatchdogDelay, 1000) && ((knx.paramByte(LOG_Watchdog) & LOG_WatchdogMask) >> LOG_WatchdogShift))
    {
        Watchdog.reset();
        gWatchdogDelay = millis();
    }
#endif
    if (!knx.configured())
        return;

    processInterrupt();
    sTimer.loop(); // clock and timer async methods
    loopSubmodules();

    // we loop on all channels and execute pipeline
    for (uint8_t lIndex = 0; lIndex < mNumChannels; lIndex++)
    {
        LogicChannel *lChannel = mChannel[lIndex];
        if (sTimer.minuteChanged())
            lChannel->startTimerInput();
        lChannel->loop();
        loopSubmodules();
    }
    if (sTimer.minuteChanged()) {
        sendHoliday();
        sTimer.clearMinuteChanged();
        loopSubmodules();
    }
    processTimerRestore();
}

EepromManager *Logic::getEEPROM() {
    return mEEPROM;
}

// start timer implementation
void Logic::processTimerRestore() {
    static uint32_t sTimerRestoreDelay = 1;
    if (sTimerRestoreDelay == 0)
        return;
    if (sTimer.isTimerValid() == tmValid && delayCheck(sTimerRestoreDelay, 500)) {
        sTimerRestoreDelay = millis();
        if (sTimerRestoreDelay == 0)
            sTimerRestoreDelay = 1; // prevent set to 0 in case of timer overflow
        if (sTimerRestore.getDayIteration() < 365) {
            if (sTimerRestore.getDayIteration() == 0)
            {
                // initialize RestoreTimer
                sTimerRestore.setup(sTimer);
            }
            else
            {
                sTimerRestore.decreaseDay();
            }
            loopSubmodules();
        } else {
            // stop timer restore processing in logic...
            sTimerRestoreDelay = 0;
            // ... and in each channel
            for (uint8_t lIndex = 0; lIndex < mNumChannels; lIndex++)
            {
                LogicChannel *lChannel = mChannel[lIndex];
                lChannel->stopTimerRestoreState();
            }
        }
    }
}

// send holiday information on bus
void Logic::sendHoliday() {
    if (sTimer.holidayChanged())
    {
        // write the newly calculated holiday information into KO (can be read externally)
        knx.getGroupObject(LOG_KoHoliday1).valueNoSend(sTimer.isHolidayToday(), getDPT(VAL_DPT_1));
        knx.getGroupObject(LOG_KoHoliday2).valueNoSend(sTimer.isHolidayTomorrow(), getDPT(VAL_DPT_1));
        sTimer.clearHolidayChanged();
        if (knx.paramByte(LOG_HolidaySend & LOG_HolidaySendMask)) {
            // and send it, if requested by application setting
            knx.getGroupObject(LOG_KoHoliday1).objectWritten();
            knx.getGroupObject(LOG_KoHoliday2).objectWritten();
        }
    }
}

void Logic::loopSubmodules() {
    static uint8_t sCount = 0;
    uint8_t lCount = sCount / 2;
    knx.loop();
    // we call submodules half as often as knx.loop();
    if (lCount * 2 == sCount && lCount < sNumLoopCallbacks) {
        sLoopCallbacks[lCount].callback(sLoopCallbacks[lCount].instance);
    }
    sCount = (lCount < sNumLoopCallbacks) ? sCount + 1 : 0;
    // for (uint8_t i = 0; i < sNumLoopCallbacks; i++)
    // {
    //     sLoopCallbacks[i].callback(sLoopCallbacks[i].instance);
    //     knx.loop();
    // }
}