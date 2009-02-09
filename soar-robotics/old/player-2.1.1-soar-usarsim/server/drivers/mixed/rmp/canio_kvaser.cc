
#include "canio_kvaser.h"


CANIOKvaser::CANIOKvaser() : DualCANIO()
{
  for (int i =0; i < DUALCAN_NR_CHANNELS; i++) 
  {
    channels[i] = -1;
  }
}

CANIOKvaser::~CANIOKvaser()
{
}

/* Initializes the class by opening 2 can channels at the given
 * frequency, which should be one of the #defined BAUD_*K values
 *
 * returns: 0 on success, negative on error
 */
int
CANIOKvaser::Init(long channel_freq)
{
  int ret;

  // Open up both CAN channels
  
  for (int i =0; i < DUALCAN_NR_CHANNELS; i++) 
  {
    if((channels[i] = 
        canOpenChannel(i, canWANT_EXCLUSIVE | canWANT_EXTENDED)) < 0) {
      return channels[i];
    }
    
    // set the channel params: 500Kbps ... CANLIB will set the other params
    // to defaults if we use BAUD_500K
    //    if ((ret = canSetBusParams(channels[i], channel_freq, 4, 3, 1, 1, 0)) < 0) {
    if ((ret = canSetBusParams(channels[i], channel_freq, 0, 0, 0, 0, 0)) < 0) 
      return ret;
    
    // set filter to only accept packets we are interested in...
    // that is, messages with IDs 0x400, 0x401, ..., 0x404
    if ((ret = canAccept(channels[i], 0x400, canFILTER_SET_MASK_STD)) < 0) {
      return ret;
    }

    if ((ret = canAccept(channels[i], 0x400, canFILTER_SET_CODE_STD)) < 0) {
      return ret;
    }

    // turn on the bus!
    if ((ret = canBusOn(channels[i])) < 0) 
      return ret;
  }
  
  return 0;
}
  
/* Closes the CAN channels
 *
 * returns: 0 on success, negative otherwise
 */
int
CANIOKvaser::Shutdown()
{
  int ret;
  for(int i =0 ; i < DUALCAN_NR_CHANNELS; i++) 
  {
    if(channels[i] >= 0) 
    {
      if((ret = canClose(channels[i])) < 0) 
	return ret;
    }
  }
  return 0;
}
  
/* Writes the given packet out on both channels
 *
 * returns: 0 on success, negative error code otherwise
 */
int
CANIOKvaser::WritePacket(CanPacket &pkt)
{
  int ret;

  //printf("CANIO: WRITE: pkt: %s\n", pkt.toString());

  for (int i=0; i < DUALCAN_NR_CHANNELS; i++) {

    if ((ret = canWriteWait(channels[i], pkt.id, pkt.msg, pkt.dlc, 
			pkt.flags, 1000)) < 0) {
      printf("CANIO: write wait error %d\n", ret);
      return ret;
    }

    
    if ((ret = canWriteSync(channels[i], 10000)) < 0) {
      printf("CANIO: error %d on write sync\n", ret);
      switch (ret) {
      case canERR_TIMEOUT:
	printf("CANIO: TIMEOUT error\n");
	break;
      default:
	break;
      }
      return ret;
    }
    
  }

  return 0;
}

/* Reads a packet.  Looks like we can just read from one channel.
 * writes it into msg (must already be allocated), and returns id, dlc, and flags
 *
 * returns: # bytes in msg if a packet is read, 0 if no packet available, and
 * negative on error
 */
int
CANIOKvaser::ReadPacket(CanPacket *pkt, int channel)
{
  int ret=0;
  long unsigned time;

  if((ret = canReadWait(channels[channel], &(pkt->id), &(pkt->msg), 
		     &(pkt->dlc), &(pkt->flags), &time, -1)) < 0) 
  {
    // either no messages or an error
    if(ret == canERR_NOMSG)
      return 0;
    else
      return ret;
  }

  return pkt->dlc;
}


