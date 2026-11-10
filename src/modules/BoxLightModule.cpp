#include "BoxLightModule.h"
#include "configuration.h"
#include "MeshService.h"
#include <HardwareSerial.h>

#define BOXLIGHT_PORTNUM ((meshtastic_PortNum)260)
#define LINK_RX_PIN 4
#define LINK_TX_PIN 2
#define LINK_BAUD 115200
#define LINK_CHANNEL_INDEX 1   // TrustUp secondary channel

BoxLightModule *boxLightModule;
static HardwareSerial periphLink(2);

static void linkSend(const uint8_t *payload, uint8_t len)
{
    uint8_t xr = len;
    for (uint8_t i = 0; i < len; i++) xr ^= payload[i];
    periphLink.write(0xAA);
    periphLink.write(len);
    periphLink.write(payload, len);
    periphLink.write(xr);
}

BoxLightModule::BoxLightModule()
    : SinglePortModule("boxlight", BOXLIGHT_PORTNUM), concurrency::OSThread("BoxLight")
{
    periphLink.begin(LINK_BAUD, SERIAL_8N1, LINK_RX_PIN, LINK_TX_PIN);
    LOG_INFO("BoxLight: link UART2 rx=%d tx=%d @%d", LINK_RX_PIN, LINK_TX_PIN, LINK_BAUD);
}

ProcessMessage BoxLightModule::handleReceived(const meshtastic_MeshPacket &mp)
{
    auto &p = mp.decoded;
    LOG_INFO("BoxLight: %d bytes from 0x%x", (int)p.payload.size, (unsigned int)mp.from);
    if (p.payload.size < 8) {
        LOG_WARN("BoxLight: short payload, ignored");
        return ProcessMessage::CONTINUE;
    }
    LOG_INFO("BoxLight: ver=%d cmd=%d -> periphLink", (int)p.payload.bytes[0], (int)p.payload.bytes[1]);
    linkSend(p.payload.bytes, 8);
    return ProcessMessage::CONTINUE;
}

// Event frame from the peripheral -> broadcast on the mesh (same bytes, port 260, TrustUp).
static void sendEventToMesh(BoxLightModule *m, const uint8_t *payload, uint8_t len);

int32_t BoxLightModule::runOnce()
{
    static uint8_t st = 0, len = 0, idx = 0, xr = 0;
    static uint8_t buf[32];
    while (periphLink.available()) {
        uint8_t c = periphLink.read();
        switch (st) {
        case 0: if (c == 0xAA) st = 1; break;
        case 1: if (c == 0 || c > sizeof(buf)) { st = 0; break; } len = c; idx = 0; xr = c; st = 2; break;
        case 2: buf[idx++] = c; xr ^= c; if (idx == len) st = 3; break;
        case 3:
            if (c == xr) sendEventToMesh(this, buf, len);
            else LOG_WARN("BoxLight: link frame bad checksum");
            st = 0;
            break;
        }
    }
    return 50;
}

static void sendEventToMesh(BoxLightModule *m, const uint8_t *payload, uint8_t len)
{
    if (len < 8 || payload[0] != 1) { LOG_WARN("BoxLight: link frame ignored (len/ver)"); return; }
    meshtastic_MeshPacket *p = m->allocDataPacket();
    p->to = NODENUM_BROADCAST;
    p->channel = LINK_CHANNEL_INDEX;
    p->want_ack = false;
    p->decoded.payload.size = 8;
    memcpy(p->decoded.payload.bytes, payload, 8);
    LOG_INFO("BoxLight: event cmd=0x%02x source=%d -> mesh", payload[1], payload[2]);
    service->sendToMesh(p, RX_SRC_LOCAL, true);
}
