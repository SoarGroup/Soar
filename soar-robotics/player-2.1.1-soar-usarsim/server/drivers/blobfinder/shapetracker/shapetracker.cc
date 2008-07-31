/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey et al
 *                      gerkey@usc.edu    
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
///////////////////////////////////////////////////////////////////////////
//
// Desc: Driver for detecting UPC barcodes from camera
// Author: Andrew Howard
// Date: 15 Feb 2004
// CVS: $Id: shapetracker.cc 4232 2007-11-01 22:16:23Z gerkey $
//
// Theory of operation:
//   TODO
//
// Requires:
//   Camera device.
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_shapetracker shapetracker
    @brief Tracks shapes in camera images.

@todo This driver is currently disabled because it needs to be updated to
the Player 2.0 API.

@todo Document this driver

@author Andrew Howard
*/
/** @} */

#include <libplayercore/playercore.h>
#include <base/imagebase.h>

#include <opencv/cv.h>
//#include <opencv/highgui.h>

#define winName "ShapeTracker"

//#include "orientation.h"

// Info on potential shapes.
class Shape 
{
  public: Shape() : id(-1), ax(0),ay(0),bx(0),by(0) {}

  // Id (-1) if undetermined.
  public: int id;

  // Shape bounding coords
  public: int ax, ay, bx, by;
};



// Driver for detecting laser retro-reflectors.
class ShapeTracker : public ImageBase
{
  // Constructor
  public: ShapeTracker( ConfigFile* cf, int section);

  // Look for barcodes in the image.  
  private: int ProcessFrame();

  private: void FindShapes();
  private: void KalmanFilter();

  private: void CreateBinaryImage(IplImage *src, IplImage *dest);

  // Write the device data (the data going back to the client).
  private: void WriteData();

  // Calculate angle between three points
  private: double CalcAngle( CvPoint *pt1, CvPoint *pt2, CvPoint *pt0);

  private: void ContrastStretch( IplImage *src, IplImage *gray );

  private: IplImage *mainImage;
  private: IplImage *workImage;

  // Histogram attributes
  private: CvHistogram *hist;
  private: int histSize;
  private: unsigned char lut[256];
  private: CvMat *lutMat; 

  private: double threshold;
  private: int vertices;

  private: Shape shapes[256];
  private: unsigned int shapeCount;

  // Kalmna filters used to track a shape
  private: CvKalman *kalmanX;
  private: CvKalman *kalmanY;
  private: int kalmanFirst;

  private: CvPoint orientPoint;
  private: double trackVelocityX;
  private: double trackVelocityY;
  private: double trackHeading;

};


// Initialization function
Driver* ShapeTracker_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new ShapeTracker( cf, section)));
}


// a driver registration function
void ShapeTracker_Register(DriverTable* table)
{
  table->AddDriver("shapetracker", ShapeTracker_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
ShapeTracker::ShapeTracker( ConfigFile* cf, int section)
	: ImageBase(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_BLOBFINDER_CODE)
{
  this->mainImage = NULL;
  this->workImage = NULL;
  this->hist = NULL;

  this->threshold = 80;
  this->vertices = 8;

  this->shapeCount = 0;
}


////////////////////////////////////////////////////////////////////////////////
// Find all the shapes in the image
void ShapeTracker::FindShapes()
{
  double s, t1;
  CvRect rect;
  int i;

  CvMemStorage *storage = cvCreateMemStorage(0);
  CvSeq *contour = 0, *result = 0;

  // Find contours on a binary image
  cvFindContours(this->workImage, storage, &contour, sizeof(CvContour), 
      CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

  for(; contour != 0; contour = contour->h_next)
  {    
    // Approximates polygonal curves with desired precision
    result = cvApproxPoly(contour, sizeof(CvContour), storage, 
        CV_POLY_APPROX_DP, cvContourPerimeter(contour)*0.02, 0);

    printf("contour %p %d\n", contour, result->total);
    
  
    if ( result->total > 4 &&
        (result->total/2.0 == (int)(result->total/2)) &&
        fabs(cvContourArea(result, CV_WHOLE_SEQ)) > 50)
    {
      s = 0;
      for(i = 0; i < result->total +1; i++)
      {
        if(i >= 2)
        {
          t1 = fabs(this->CalcAngle(
                (CvPoint*)cvGetSeqElem(result, i ),
                (CvPoint*)cvGetSeqElem(result, i-2),
                (CvPoint*)cvGetSeqElem(result, i-1)));
          s = s > t1 ? s : t1;
          
        }
      }
      if(s < 0.5)
      {
        rect = cvBoundingRect(result,0);

        if (rect.x >5 && rect.y>5)/* && 
            rect.width < this->cameraData.width-5 && 
            rect.height < this->cameraData.height-5)*/
        {
          this->shapes[this->shapeCount].id = result->total;
          this->shapes[this->shapeCount].ax = rect.x;
          this->shapes[this->shapeCount].ay = rect.y;
          this->shapes[this->shapeCount].bx = rect.x + rect.width;
          this->shapes[this->shapeCount].by = rect.y + rect.height;

          this->shapeCount = (this->shapeCount+1) % 255;
          //cvDrawContours(this->workImage, result, 255, 0, 0, 3, 8);
          //cvSaveImage("workContours.jpg", this->workImage);
         // printf("Total[%d] XY[%d %d]\n",result->total, rect.x, rect.y);

        }

        //cvDrawContours(this->mainImage, result, 255, 0, 0, 5, 8);

        //this->orientPoint = getcentralpoint(this->mainImage, result);
      }
    }
  }

  cvReleaseMemStorage(&storage);
}


////////////////////////////////////////////////////////////////////////////////
// Run a Kalman filter 
void ShapeTracker::KalmanFilter()
{
  /*CvMoments moments;
  CvPoint pt3, pt4;
  int xmean, ymean;


  float xmeanp, ymeanp, xmeanvp, ymeanvp;
  //float orientation = 0.0;
  float vx, vy;

  //Calculates all moments up to third order of a polygon or rasterized shape
  cvMoments(this->workImage, &moments, 1);

  xmean = (int)(moments.m10/moments.m00);
  ymean = (int)(moments.m01/moments.m00);

  if ((xmean != 0) && (ymean != 0))
  {
    CvPoint tmpOrient;

    // Retrieves central moment from moment state structure
    float cm02 = cvGetCentralMoment(&moments, 0, 2);
    float cm20 = cvGetCentralMoment(&moments, 2, 0);
    float cm11 = cvGetCentralMoment(&moments, 1, 1);

    tmpOrient.x = xmean;
    tmpOrient.y = ymean;

    this->trackHeading = getorientation(tmpOrient, this->orientPoint);

    //fprintf(stderr, " heading %f\n", heading);
    //orientation = atan2(cm20 -cm02, 2*cm11)*180.0/(2*M_PI);
    if(this->kalmanFirst == 1)
    {
      fprintf(stderr, " x %f %f\n", (float)xmean, (float)ymean);
      this->kalmanX->state_post->data.fl[0] = (float) xmean;
      this->kalmanY->state_post->data.fl[1] = (float) 0.0;

      kalmany->state_post->data.fl[0] = (float) ymean;
      kalmany->state_post->data.fl[1] = (float) 0.0;
      this->kalmanFirst = 0;
    }
  }


  // If we have no x and y point, assume center of image
  if (xmean == 0 && ymean == 0)
  {
    xmean = (this->cameraData.width/2);
    ymean = (this->cameraData.height/2);
  }


  if (this->kalmanFirst == 0)
  {
    CvMat *predictionx, *predictiony;

    // Predict the next position
    predictionx = cvKalmanPredict(this->kalmanX, 0);
    predictiony = cvKalmanPredict(this->kalmanY, 0);

    //if(debug > 1)
      //{
        //xmeanp = predictionx->data.fl[0];
        //ymeanp = predictiony->data.fl[0];

        //xmeanvp = predictionx->data.fl[1];
        // ymeanvp = predictiony->data.fl[1];
        //fprintf(stderr, "predict %f %f %f %f\n",xmeanp,xmeanvp,ymeanp, ymeanvp);
        //fprintf(stderr, "meas %f %f\n",(float)xmean, (float)ymean);
    //}

    if(xmean == (width/2) && ymean == (height/2))
    {

      pt3.x=(int)predictionx->data.fl[0];
      pt4.x=(int)predictionx->data.fl[1];

      pt3.y = (int)predictiony->data.fl[0];
      pt4.y=(int)predictiony->data.fl[1];

      this->kalmanX->state_post->data.fl[0] = predictionx->data.fl[0];
      this->kalmanX->state_post->data.fl[1] = predictionx->data.fl[1];
      this->kalmanY->state_post->data.fl[0] = predictiony->data.fl[0];
      this->kalmanY->state_post->data.fl[1] = predictiony->data.fl[1];
    } else {

      CvMat *corrx, *corry;

      this->measurementX->data.fl[0] = (float)xmean;
      this->measurementY->data.fl[0] = (float)ymean;

      corrx = cvKalmanCorrect(this->kalmanX, this->measurementX);
      corry = cvKalmanCorrect(this->kalmanY, this->measurementY);

      pt3.x=(int)(corrx->data.fl[0]);
      pt4.x=(int)(corrx->data.fl[1]);
      pt3.y = (int)(corry->data.fl[0]);
      pt4.x=(int)(corry->data.fl[1]);

    }


    //if(debug == 3)
    //{
      //fprintf(stderr, "post %d %d %d %d\n", pt3.x, pt3.y, pt4.x, pt4.y);
      //fprintf(stderr, "pre after %f %f %f %f\n",
          //this->kalmanX->state_post->data.fl[0],
          //this->kalmanY->state_post->data.fl[0],
          //this->kalmanX->state_post->data.fl[1],
          //this->kalmanY->state_post->data.fl[1]);
    //}

    //cvCircle(init_image, pt3, 15.0, CV_RGB(255,0,0), 2);

    this->trackVelocityX = (float)(pt3.x - (this->cameraData.width/2)) / 
      (this->cameraData.width/2);
    this->trackVelocityY = (float)(-pt3.y + (this->cameraData.height/2)) /
      (this->cameraData.height/2);

  }

  //if(reset_kalman == 1)
  //{
    //if(debug == 2)
    //{
      //fprintf(stderr, " xmean  %d, ymean %d\n",xmean, ymean);
    //}
    //vx = (float)(xmean - (width/2))/(width/2);
    //vy = (float)(-ymean + (height/2))/(height/2);
    //this->kalmanFirst =1; // reset the kalman filter
  //}

  if(this->trackVelocityX > 1.0)
    this->trackVelocityX = 1.0;
  if(this->trackVelocityX < -1.0)
    this->trackVelocityX = -1.0;

  if(this->trackVelocityY > 1.0)
    this->trackVelocityY = 1.0;
  if(this->trackVelocityY < -1.0)
    this->trackVelocityY = -1.0;

  //visual_servo_command.vel_right = vx;
  //visual_servo_command.vel_forward = vy;
  //visual_servo_command.heading_offset = (float)heading;
  //visual_servo_command.tracking_state = HV_OBJECT_TRACK_STATE;

  //if(debug > 1 ){
    //fprintf(stderr, "Sent x %f, y %f, z %f, state %d\n",
        //visual_servo_command.vel_right, 
        //visual_servo_command.vel_forward,
        //visual_servo_command.heading_offset,
        //visual_servo_command.tracking_state);
  //}

  //put_data_on_image(width, height, vx, vy, heading, font);
  //draw_lines(width, height, heading, pt3);
*/
  return;
}

////////////////////////////////////////////////////////////////////////////////
// Reset the kalman filter
/*void ShapeTracker::ResetKalman()
{
  this->kalmanFirst = 1;
}*/

////////////////////////////////////////////////////////////////////////////////
// Look for stuff in the image.
int ShapeTracker::ProcessFrame()
{

  // Reset the shape count
  this->shapeCount = 0;

  // Create a new mainImage if it doesn't already exist
  //if (this->mainImage == NULL)
    this->mainImage = cvCreateImage( cvSize(this->stored_data.width,
          this->stored_data.height), IPL_DEPTH_8U, 3);

  // Create a work image if it doesn't already exist
  //if (this->workImage == NULL)
    workImage = cvCreateImage( cvSize(this->stored_data.width,
          this->stored_data.height), IPL_DEPTH_8U, 1);

  if (this->hist == NULL)
  {
    this->histSize = 256;
    float range0[] = {0, 256};
    float *ranges[] = {range0};

    this->hist = cvCreateHist(1, &histSize, CV_HIST_ARRAY, ranges, 1);
    this->lutMat = cvCreateMatHeader(1, 256, CV_8UC1);
    cvSetData(this->lutMat, this->lut, 0);
  }

  // Initialize the main image
  memcpy(this->mainImage->imageData, this->stored_data.image, 
         this->mainImage->imageSize);

  // Make dest a gray scale image
  cvCvtColor(this->mainImage, this->workImage, CV_BGR2GRAY);

  //cvSaveImage("orig.jpg", this->mainImage);

  this->ContrastStretch( this->mainImage, this->workImage );

  //cvSaveImage("hist.jpg", this->workImage);

  cvCvtColor(this->mainImage, this->workImage, CV_BGR2GRAY);

  // Create a binary image
  cvThreshold(this->workImage, this->workImage, this->threshold, 255, 
      CV_THRESH_BINARY);

  //cvSaveImage("work.jpg", this->workImage);

  // Find all the shapes in the image
  this->FindShapes();


  //this->KalmanFilter();
  

  // Clear the images
  cvReleaseImage(&(this->workImage));
  this->workImage = NULL;
  cvReleaseImage(&(this->mainImage));
  this->mainImage = NULL;
  
  WriteData();
  
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Update the device data (the data going back to the client).
void ShapeTracker::WriteData()
{
  unsigned int i;
  Shape *shape;
  player_blobfinder_data_t data;

  // Se the image dimensions
  data.width = (this->stored_data.width);
  data.height = (this->stored_data.height);

  data.blobs_count = shapeCount;
  data.blobs = (player_blobfinder_blob_t*)calloc(shapeCount,sizeof(data.blobs[0]));
  
  // Go through the blobs
  for (i = 0; i < this->shapeCount; i++)
  {
    shape = this->shapes + i;

    // Set the data to pass back
    data.blobs[i].id = shape->id;
    data.blobs[i].color = 0;  // TODO
    data.blobs[i].area = ((int) ((shape->bx - shape->ax) * (shape->by - shape->ay)));
    data.blobs[i].x = ((int) ((shape->bx + shape->ax) / 2));
    data.blobs[i].y = ((int) ((shape->by + shape->ay) / 2));
    data.blobs[i].left = ((int) (shape->ax));
    data.blobs[i].top = ((int) (shape->ay));
    data.blobs[i].right = ((int) (shape->bx));
    data.blobs[i].bottom = ((int) (shape->by));
    data.blobs[i].range = (0);
  }

  // Copy data to server.
  Publish(device_addr,PLAYER_MSGTYPE_DATA,PLAYER_BLOBFINDER_DATA_BLOBS,&data,sizeof(data));
  free(data.blobs);
  
  return;
}

////////////////////////////////////////////////////////////////////////////////
// Calculate the angle between three points
double ShapeTracker::CalcAngle( CvPoint *pt1, CvPoint *pt2, CvPoint *pt0)
{
  double dx1 = pt1->x - pt0->x;
  double dy1 = pt1->y - pt0->y;
  double dx2 = pt2->x - pt0->x;
  double dy2 = pt2->y - pt0->y;
  return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

void ShapeTracker::ContrastStretch( IplImage *src, IplImage *gray )
{
  int high = 0;
  int low = 0;
  int index;
  float hist_value;
  float scale_factor;
  IplImage *R = cvCreateImage(cvGetSize(src), src->depth, 1);
  IplImage *G = cvCreateImage(cvGetSize(src), src->depth, 1);
  IplImage *B = cvCreateImage(cvGetSize(src), src->depth, 1);

  cvCalcHist(&gray, this->hist, 0, NULL);

  for(index = 0; index < this->histSize; index++){
    hist_value = cvQueryHistValue_1D(this->hist, index);
    if(hist_value != 0){
      low = index;
      break;
    }
  } 

  for(index = this->histSize-1; index >= 0; index--){
    hist_value = cvQueryHistValue_1D(this->hist, index);
    if(hist_value != 0){
      high = index;
      break;
    }
  }             

  scale_factor = 255.0f/(float)(high - low);
  for(index = 0; index < 256; index++){
    if((index >= low) && (index <= high))
    {
      this->lut[index] = (unsigned char)((float)(index - low)*scale_factor);
    }
    if(index > high) this->lut[index] = 255;
  }

  cvCvtPixToPlane(src, R, G, B, NULL);
  cvLUT(R, R, this->lutMat);
  cvLUT(G, G, this->lutMat);
  cvLUT(B, B, this->lutMat);
  cvCvtPlaneToPix(R, G, B, NULL, src);

  cvReleaseImage(&R);
  cvReleaseImage(&G);
  cvReleaseImage(&B);
}

