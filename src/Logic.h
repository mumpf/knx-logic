#pragma once
#include "LogicChannel.h"

class Logic
{
  private:
    struct sTime {
        uint8_t minute;
        uint8_t hour;
    };

    struct sDay {
        uint8_t day;
        uint8_t month;
    };

    static uint8_t sMagicWord[];
    static struct tm sDateTime;
    static uint8_t sTimeOk;

    LogicChannel *mChannel[LOG_ChannelsFirmware];
    uint8_t mNumChannels; // Number of channels defined in knxprod
    uint32_t mSaveInterruptTimestamp = 0;
    uint32_t mLastWriteToEEPROM = 0;
    bool mIsValidEEPROM = false;
    bool mIndicateTimerInput = false;
    EepromManager *mEEPROM;
    uint32_t mTimeDelay = 0;
    int8_t mMinuteTick = -1; // timer evaluation is called each time the minute changes
    int8_t mDayTick = -1; // sunrise/sunset calculation happens each time the day changes
    int16_t mEasterTick = -1; // easter calculation happens each time year changes
    sTime mSunrise;
    sTime mSunset;
    sDay mEaster = {0, 0};

    LogicChannel *getChannel(uint8_t iChannelId);
    uint8_t getChannelId(LogicChannel *iChannel);
    bool prepareChannels();

    void writeAllDptToEEPROM();
    void writeAllInputsToEEPROM();

    void onSavePinInterruptHandler();
    void beforeRestartHandler();
    void beforeTableUnloadHandler(TableObject & iTableObject, LoadState & iNewState);

    void processTime();
    void calculateSunriseSunset();
    void calculateEaster();

  public:
    Logic();
    ~Logic();
    
    // static
    static void onInputKoHandler(GroupObject &iKo);
    static void onBeforeRestartHandler();
    static void onBeforeTableUnloadHandler(TableObject & iTableObject, LoadState & iNewState);
    static void onSafePinInterruptHandler();
    static tm *getDateTime();
    // instance
    EepromManager *getEEPROM();
    void writeAllInputsToEEPROMFacade();
    void processAllInternalInputs(LogicChannel *iChannel, bool iValue);
    void processInputKo(GroupObject &iKo);
    void processInterrupt(bool iForce = false);
    bool processDiagnoseCommand(char *iBuffer);
    void debug();
    void setup(bool iSaveSupported);
    void loop();
};
