#pragma once
#include "SinglePortModule.h"
#include "concurrency/OSThread.h"

/**
 * Trust-Up MeshBox LED module.
 * Receives 8-byte BoxLight commands on PortNum 260 (fleet protocol identity — never change)
 * and will drive the WS2812B pattern engine (Phase 3). Phase 2: log-only skeleton.
 */
class BoxLightModule : public SinglePortModule, private concurrency::OSThread
{
  public:
    BoxLightModule();

  protected:
    virtual ProcessMessage handleReceived(const meshtastic_MeshPacket &mp) override;
    virtual int32_t runOnce() override;
};

extern BoxLightModule *boxLightModule;