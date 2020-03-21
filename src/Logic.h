#pragma once
#include "LogicChannel.h"

class Logic
{
  private:
    static uint8_t sMagicWord[];

    LogicChannel *mChannel[LOG_ChannelsFirmware];
    uint8_t mNumChannels; // Number of channels defined in knxprod
    uint32_t mSaveInterruptTimestamp = 0;
    uint32_t mLastWriteToEEPROM = 0;
    bool mIsValidEEPROM = false;
    EepromManager *mEEPROM;
    struct tm mDateTime = {0};
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

    // instance
    EepromManager *getEEPROM();
    void writeAllInputsToEEPROMFacade();
    void processAllInternalInputs(LogicChannel *iChannel, bool iValue);
    void processInputKo(GroupObject &iKo);
    void processInterrupt(bool iForce = false);
    void debug();
    void setup(bool iSaveSupported);
    void loop();
};

