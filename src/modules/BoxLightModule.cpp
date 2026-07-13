#include "BoxLightModule.h"
#include "configuration.h"

#define BOXLIGHT_PORTNUM ((meshtastic_PortNum)260)

BoxLightModule *boxLightModule;

BoxLightModule::BoxLightModule()
    : SinglePortModule("boxlight", BOXLIGHT_PORTNUM), concurrency::OSThread("BoxLight")
{
}

ProcessMessage BoxLightModule::handleReceived(const meshtastic_MeshPacket &mp)
{
    auto &p = mp.decoded;
    LOG_INFO("BoxLight: %d bytes from 0x%x", (int)p.payload.size, (unsigned int)mp.from);
    if (p.payload.size >= 2) {
        LOG_INFO("BoxLight: proto_ver=%d cmd=%d", (int)p.payload.bytes[0], (int)p.payload.bytes[1]);
    }
    // Phase 3: full payload parse + LED pattern engine dispatch
    return ProcessMessage::STOP;
}

int32_t BoxLightModule::runOnce()
{
    // Phase 3: ~33ms animation tick while a pattern is active.
    return 1000;
}