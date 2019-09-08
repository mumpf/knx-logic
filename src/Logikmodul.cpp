
#include "LogikmodulCore.h"

struct sRuntimeInfo {
    uint16_t currentPipeline;
    unsigned long startupDelay;
    unsigned long heartbeatDelay;
};

sRuntimeInfo gRuntimeData;

// true solgange der Start des gesamten Moduls verzögert werden soll
bool startupDelay() {
    return (millis() - gRuntimeData.startupDelay < knx.paramInt(PAR_StartupDelay) * 1000);
}

void ProcessHeartbeat() {
    // the first heartbeat is send directly after startup delay of the device
    if (gRuntimeData.heartbeatDelay == 0 || millis() - gRuntimeData.heartbeatDelay > knx.paramInt(PAR_Heartbeat) * 1000) {
        // we waited enough, let's send a heartbeat signal
        knx.getGroupObject(KO_Heartbeat).value(true);
        gRuntimeData.heartbeatDelay = millis();
        // debug-helber for logic module
        // logikDebug();
    }
}

void appLoop() {
    if (!knx.configured())
        return;

    // handle KNX stuff
    if (startupDelay())
        return;

    // at this point startup-delay is done
    // we process heartbeat
    ProcessHeartbeat();

    logikLoop();
}

void appSetup() {

    logikSetup();

    if (knx.configured()) {
        gRuntimeData.startupDelay = millis();
        gRuntimeData.heartbeatDelay = 0;
        // init KO-DPTs
        knx.getGroupObject(KO_Heartbeat).dataPointType(Dpt(1, 2));
    }
}