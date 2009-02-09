#ifndef _KVASER_CANLIB_
#define _KVASER_CANLIB_

#include <canlib.h>
#include "canio.h"

class CANIOKvaser : public DualCANIO
{
  private:
    canHandle channels[2];
    
  public:
    CANIOKvaser();
    virtual ~CANIOKvaser();
    virtual int Init(long channel_freq);
    virtual int ReadPacket(CanPacket *pkt, int channel);
    virtual int WritePacket(CanPacket &pkt);
    virtual int Shutdown();
};

#endif
