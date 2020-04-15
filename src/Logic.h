#pragma once
#include "LogicChannel.h"

class Logic
{
  private:
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

    LogicChannel *getChannel(uint8_t iChannelId);
    uint8_t getChannelId(LogicChannel *iChannel);
    bool prepareChannels();

    void writeAllDptToEEPROM();
    void writeAllInputsToEEPROM();

    void onSavePinInterruptHandler();
    void beforeRestartHandler();
    void beforeTableUnloadHandler(TableObject & iTableObject, LoadState & iNewState);

    void processTime();

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
    void processDiagnoseCommand(char *iBuffer);
    void debug();
    void setup(bool iSaveSupported);
    void loop();
};
