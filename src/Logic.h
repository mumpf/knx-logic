#pragma once
#include "LogicChannel.h"
#include "Timer.h"

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

  private:
    static uint8_t sMagicWord[];
    static Timer &sTimer;

    LogicChannel *mChannel[LOG_ChannelsFirmware];
    uint8_t mNumChannels; // Number of channels defined in knxprod
    uint32_t mSaveInterruptTimestamp = 0;
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

    void processTime();
    void calculateSunriseSunset();
    void calculateEaster();
};
