#include <linux/types.h>
#include <linux/videodev2.h>
#include <string.h>

class UvcInterface;

#ifndef UVCINTERFACE_H_
#define UVCINTERFACE_H_

class UvcInterface
{
	public:
		UvcInterface(char const *sDevice,int aWidth=320,int aHeight=240):device(sDevice),frame(0),frameSize(0),fd(-1),width(aWidth),height(aHeight){buffer[0]=0;buffer[1]=0;}
		~UvcInterface(void) {device=0;Close();}
		
		int Open(void);
		int Close(void);
		int Read(void);
		
		int GetWidth(void) const;
		int GetHeight(void) const;
		
		int GetFrameSize(void) const {return frameSize;}
		void CopyFrame(unsigned char *dest) const {memcpy(dest,frame,frameSize);}

		bool IsOpen(void) const {return fd!=-1;}
		
	private:
		char const *device;

		unsigned char *frame;
		int frameSize;
		
		unsigned char *buffer[2];
		int length[2];
		
		int fd;
		
		v4l2_capability cap;
		v4l2_format fmt;
		
		static const int dht_size;
		static const unsigned char dht_data[];
		
		int width;
		int height;
};

#endif /*UVCINTERFACE_H_*/
