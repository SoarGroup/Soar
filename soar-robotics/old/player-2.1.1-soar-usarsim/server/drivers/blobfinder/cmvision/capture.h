/* $Id: capture.h 3142 2005-09-25 23:35:06Z thjc $
 *
 * The base class for capture classes that feed data to CMVision
 */

#ifndef __CAPTURE_H__
#define __CAPTURE_H__

typedef long long stamp_t;

class capture 
{
  protected:
    unsigned char *current; // most recently captured frame
    stamp_t timestamp;      // frame time stamp
    int width,height;       // dimensions of video frame
    bool captured_frame;

  public:
    capture() {current=0; captured_frame = false;}
    virtual ~capture() {};

    // you must define these in the subclass
    virtual bool initialize(int nwidth,int nheight) = 0;
    virtual void close() = 0;
    virtual unsigned char *captureFrame() = 0;
};

#endif // __CAPTURE_H__
