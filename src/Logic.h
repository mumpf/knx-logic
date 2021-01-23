#pragma once
#include "LogicChannel.h"
#include "Timer.h"
#include "TimerRestore.h"

class Logic
{
  public:
    Logic();
    ~Logic();

    // static
    static void onInputKoHandler(GroupObject &iKo);
    static void onBeforeRestartHandler();
    static void onBeforeTableUnloadHandler(TableObject & iTableObject, LoadState & iNewState);
    static void onSafePinInterruptHandler();
    static char *initDiagnose(GroupObject &iKo);
    static char *getDiagnoseBuffer();

    // instance
    EepromManager *getEEPROM();
    void writeAllInputsToEEPROMFacade();
    void processAllInternalInputs(LogicChannel *iChannel, bool iValue);
    void processReadRequests();
    void processInputKo(GroupObject &iKo);
    void processInterrupt(bool iForce = false);
    bool processDiagnoseCommand();
    void outputDiagnose(GroupObject &iKo);
    void debug();
    void setup(bool iSaveSupported);
    void loop();

  private:
    static uint8_t sMagicWord[];
    static Timer &sTimer;
    static TimerRestore &sTimerRestore;
    static char sDiagnoseBuffer[16];

    LogicChannel *mChannel[LOG_ChannelsFirmware];
    uint8_t mNumChannels; // Number of channels defined in knxprod
    uint32_t mSaveInterruptTimestamp = 0;
    uint16_t mSaveInterruptCount = 0;

    uint32_t mLastWriteToEEPROM = 0;
    bool mIsValidEEPROM = false;
    EepromManager *mEEPROM;

    LogicChannel *getChannel(uint8_t iChannelId);
    uint8_t getChannelId(LogicChannel *iChannel);
    bool prepareChannels();

    void writeAllDptToEEPROM();
    void writeAllInputsToEEPROM();

    void onSavePinInterruptHandler();
    void beforeRestartHandler();
    void beforeTableUnloadHandler(TableObject &iTableObject, LoadState &iNewState);
    void processDiagnoseCommand(GroupObject &iKo);

    void processTime();
    void calculateSunriseSunset();
    void calculateEaster();

    void sendHoliday();
    void startTimerRestore();
};
