#include "BoxLightModule.h"
#include "configuration.h"
#include <HardwareSerial.h>

#define BOXLIGHT_PORTNUM ((meshtastic_PortNum)260)

// Link to the peripheral ESP. UART2 on free pads; Serial1 belongs to the GPS driver here.
#define LINK_RX_PIN 4      // D3 <- periph TX (J1-9)
#define LINK_TX_PIN 2      // D1 -> periph RX (J1-7)
#define LINK_BAUD 115200

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
    LOG_INFO("BoxLight: periphLink UART2 rx=%d tx=%d @%d", LINK_RX_PIN, LINK_TX_PIN, LINK_BAUD);
}

ProcessMessage BoxLightModule::handleReceived(const meshtastic_MeshPacket &mp)
{
    auto &p = mp.decoded;
    LOG_INFO("BoxLight: %d bytes from 0x%x", (int)p.payload.size, (unsigned int)mp.from);
    if (p.payload.size < 8) {
        LOG_WARN("BoxLight: short payload, ignored");
        return ProcessMessage::STOP;
    }
    LOG_INFO("BoxLight: ver=%d cmd=%d -> periphLink", (int)p.payload.bytes[0], (int)p.payload.bytes[1]);
    linkSend(p.payload.bytes, 8);
    return ProcessMessage::STOP;
}

int32_t BoxLightModule::runOnce()
{
    // M3: parse event frames from the peripheral here and send them on port 260.
    while (periphLink.available()) {
        int c = periphLink.read();
        LOG_DEBUG("BoxLight: periphLink rx byte 0x%02x", c);
    }
    return 500;
}
