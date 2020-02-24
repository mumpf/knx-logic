#include "Helper.h"
#include "Hardware.h"
#include "Logic.h"

struct sRuntimeInfo
{
    uint16_t currentPipeline;
    unsigned long startupDelay;
    unsigned long heartbeatDelay;
};

sRuntimeInfo gRuntimeData;
Logic gLogic;

// true solgange der Start des gesamten Moduls verzögert werden soll
bool startupDelay()
{
    return !delayCheck(gRuntimeData.startupDelay, knx.paramInt(LOG_StartupDelay) * 1000);
}

void ProcessReadRequests() {
    // this method is called after startup delay and executes read requests, which should just happen once after startup
    static bool sCalled = false;

    if (!sCalled) {
        if (knx.paramByte(LOG_ReadTimeDate) & 0x80) {
            knx.getGroupObject(LOG_KoTime).requestObjectRead();
            knx.getGroupObject(LOG_KoDate).requestObjectRead();
        }
        sCalled = true;
    }
}

void ProcessHeartbeat()
{
    // the first heartbeat is send directly after startup delay of the device
    if (gRuntimeData.heartbeatDelay == 0 || delayCheck(gRuntimeData.heartbeatDelay, knx.paramInt(LOG_Heartbeat) * 1000))
    {
        // we waited enough, let's send a heartbeat signal
        knx.getGroupObject(LOG_KoHeartbeat).value(true, getDPT(VAL_DPT_1));
        gRuntimeData.heartbeatDelay = millis();
        // debug-helper for logic module, just a test
        gLogic.debug();
    }
}

void appLoop()
{
    if (!knx.configured())
        return;

    // handle KNX stuff
    if (startupDelay())
        return;

    // at this point startup-delay is done
    // we process heartbeat
    ProcessHeartbeat();
    ProcessReadRequests();
    gLogic.loop();
}

void appSetup(uint8_t iSavePin)
{

    // try to get rid of occasional I2C lock...
    // savePower();
    // delay(100);
    // restorePower();
    // check hardware availability
    boardCheck();

    if (knx.configured())
    {
        gRuntimeData.startupDelay = millis();
        gRuntimeData.heartbeatDelay = 0;
        gLogic.setup(iSavePin);
    }
}