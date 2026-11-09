#pragma once
#include "SinglePortModule.h"
#include "concurrency/OSThread.h"

/**
 * Trust-Up MeshBox link module. Forwards port-260 BoxLight frames to the peripheral ESP
 * over UART2 and broadcasts event frames from the peripheral back onto the mesh.
 */
class BoxLightModule : public SinglePortModule, private concurrency::OSThread
{
  public:
    BoxLightModule();
    using SinglePortModule::allocDataPacket;   // expose for the file-local send helper

  protected:
    virtual ProcessMessage handleReceived(const meshtastic_MeshPacket &mp) override;
    virtual int32_t runOnce() override;
};

extern BoxLightModule *boxLightModule;
