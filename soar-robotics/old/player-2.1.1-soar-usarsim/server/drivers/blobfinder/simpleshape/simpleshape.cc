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
// Desc: Driver for detecting simple shapes in a camera image
// Author: Andrew Howard
// Date: 15 Feb 2004
// CVS: $Id: simpleshape.cc 4367 2008-02-18 19:42:49Z thjc $
//
///////////////////////////////////////////////////////////////////////////

/** @ingroup drivers */
/** @{ */
/** @defgroup driver_simpleshape simpleshape
 * @brief Visual shape-detection

@todo This driver is currently disabled because it needs to be updated to
the Player 2.0 API.

The simpleshape driver looks for simple geometric shapes in a camera
image.  The user must supply a @e model of the target shape, in the
form of a binary image (such as the one shown below).

@image html simpleshape_h.gif "Sample model for the simpleshape detector"

@par Compile-time dependencies

- OpenCV

@par Requires

- This driver acquires image data from a @ref interface_camera
  interface.

@par Provides

- This driver provides detected shapes through a @ref
  interface_blobfinder interface.

- This driver also supplies processed image data through a @ref
  interface_camera interface (this data is intended mostly for
  debugging).  Note that the dimensions of the output image are twice
  that of the input image: the output image is divided into four
  parts, each showing one step in the detection process.  From
  top-to-bottom and left-to-right, these are: original image
  (monochrome), edge image, contour image, detected shapes.

@image html simpleshape_output.gif "Output image (debugging)"

@par Configuration requests

- none

@par Configuration file options

- model (string)
  - Default: NULL
  - Filename of the model image file.  This should by a binary,
    grayscale image.

- canny_thresh (float tuple) 
  - Default: [40 20]
  - Thresholds for the Canny edge detector.

- match_thresh (float tuple) 
  - Default: [0.50 20.0 0.20]
  - Match thresholds (?)

@par Example

@verbatim
driver
(
  name "simpleshape"
  requires ["camera:0"]
  provides ["blobfinder:1" "camera:1"]
  model "simpleshape_h.pgm"
)
@endverbatim

@author Andrew Howard
*/
/** @} */

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../../base/imagebase.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

// Invariant feature set for a contour
class FeatureSet
{
  // Contour moments
  public: CvMoments moments;

  // Compactness
  public: double compact;

  // Elliptical variance
  public: double variance;

  // Vertices
  public: int vertexCount;
  public: int vertexString[256];
};


// Info on potential shapes.
class Shape 
{
  // Id (-1) if undetermined.
  public: int id;

  // Shape bounding coords
  public: int ax, ay, bx, by;
};


// Driver for detecting laser retro-reflectors.
class SimpleShape : public ImageBase
{
  public: 
    // Constructor
    SimpleShape( ConfigFile* cf, int section);

    // Setup/shutdown routines.
    virtual int Setup();
    virtual int Shutdown();

  // Load a shape model
  private: int LoadModel();

  // Process the image
  private: int ProcessFrame();

  // Having pre-processed the image, find some shapes
  private: void FindShapes();

  // Extract a feature set for the given contour
  private: void ExtractFeatureSet(CvContour *contour, FeatureSet *feature);

  // Compute similarity measure on features
  private: int MatchFeatureSet(FeatureSet *a, FeatureSet *b);

  // Write the outgoing blobfinder data
  private: void WriteBlobfinderData();

  private: void WriteCameraData();

  // Model data (this is the shape to search for)
  private: const char *modelFilename;
  private: CvMemStorage *modelStorage;
  private: CvSeq *modelContour;
  private: FeatureSet modelFeatureSet;

  // Images
  private: IplImage *inpImage;
  private: IplImage *outImage;
  private: CvMat outSubImages[4];
  private: IplImage *workImage;

  // Parameters
  private: double cannyThresh1, cannyThresh2;
  private: double matchThresh[3];

  // List of potential shapes
  private: Shape shapes[256];
  private: unsigned int shapeCount;
  
  private: player_devaddr_t blobfinder_addr;
  private: player_devaddr_t debugcam_addr;

  private: bool debugcam;
};


// Initialization function
Driver* SimpleShape_Init( ConfigFile* cf, int section)
{
  return ((Driver*) (new SimpleShape( cf, section)));
}


// a driver registration function
void SimpleShape_Register(DriverTable* table)
{
  table->AddDriver("simpleshape", SimpleShape_Init);
}


////////////////////////////////////////////////////////////////////////////////
// Constructor
SimpleShape::SimpleShape( ConfigFile* cf, int section)
	: ImageBase(cf, section, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN)
{
  this->inpImage = NULL;
  this->outImage = NULL;
  this->workImage = NULL;
  memset(&(this->blobfinder_addr), 0, sizeof blobfinder_addr);
  memset(&(this->debugcam_addr), 0, sizeof debugcam_addr);
  this->debugcam = true;

  if (cf->ReadDeviceAddr(&(this->blobfinder_addr), section, "provides",
                           PLAYER_BLOBFINDER_CODE, -1, NULL))
  {
    this->SetError(-1);
    return;
  }
  if (this->AddInterface(this->blobfinder_addr))
  {
    this->SetError(-1);
    return;
  }
  if (cf->ReadDeviceAddr(&(this->debugcam_addr), section, "provides",
                           PLAYER_CAMERA_CODE, -1, NULL))
  {
    PLAYER_WARN("debug preview will not be available");
    this->debugcam = false;
  } else if (this->AddInterface(this->debugcam_addr))
  {
    this->SetError(-1);
    return;
  }

  // Filename for the target shape image
  this->modelFilename = cf->ReadFilename(section, "model", NULL);

  // Parameters
  this->cannyThresh1 = cf->ReadTupleFloat(section, "canny_thresh", 0, 40);
  this->cannyThresh2 = cf->ReadTupleFloat(section, "canny_thresh", 1, 20);

  this->matchThresh[0] = cf->ReadTupleFloat(section, "match_thresh", 0, 0.50);
  this->matchThresh[1] = cf->ReadTupleFloat(section, "match_thresh", 1, 20.0);
  this->matchThresh[2] = cf->ReadTupleFloat(section, "match_thresh", 2, 0.20);  

  this->shapeCount = 0;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device (called by server thread).
int SimpleShape::Setup()
{
  // Load the shape model
  if (this->LoadModel() != 0)
    return -1;

  return ImageBase::Setup();

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device (called by server thread).
int SimpleShape::Shutdown()
{
  ImageBase::Shutdown();
  
  // Free images
  if (this->inpImage)
    cvReleaseImage(&(this->inpImage));
  if (this->outImage)
    cvReleaseImage(&(this->outImage));

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Load a shape model
int SimpleShape::LoadModel()
{
  IplImage *img, *work;
  CvSize size;
  CvSeq *contour, *maxContour;
  double area, maxArea;

  // Load the image
  img = cvLoadImage(this->modelFilename, 0);
  if (img == NULL)
  {
    PLAYER_ERROR("failed to load model file");
    return -1;
  }

  // Create work image
  size = cvSize(img->width, img->height);
  work = cvCreateImage(size, IPL_DEPTH_8U, 1);

  // Find edges
  cvCanny(img, work, this->cannyThresh1, this->cannyThresh2);

  // Extract contours
  this->modelStorage = cvCreateMemStorage(0);
  cvFindContours(work, this->modelStorage, &contour, sizeof(CvContour), 
                 CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
  
  // Find the contour with the largest area (we will use the outer
  // contour only)
  maxArea = 0;
  maxContour = NULL;
  for (; contour != NULL; contour = contour->h_next)
  {
    area = fabs(cvContourArea(contour));
    if (area > maxArea)
    {
      maxArea = area;
      maxContour = contour;
    }
  }
  if (maxContour == NULL)
  {
    PLAYER_ERROR("no usable contours in model image");
    return -1;
  }
  this->modelContour = maxContour;

  // Record some features of the contour; we will use these to
  // recognise it later.
  this->ExtractFeatureSet((CvContour*) this->modelContour, &this->modelFeatureSet);

  // Free the image
  cvReleaseImage(&work);
  cvReleaseImage(&img);

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Look for stuff in the image.
int SimpleShape::ProcessFrame()
{
  CvSize size;
  int i, j, width, height, depth;
  unsigned char * row1;
  unsigned char * row2;

  width = this->stored_data.width;
  height = this->stored_data.height;
  assert(width > 0 && height > 0);
    
  // Create input and output image if it doesnt exist
  size = cvSize(width, height);
  if (this->inpImage == NULL)
    this->inpImage = cvCreateImage(size, IPL_DEPTH_8U, 1);
  size = cvSize(2 * width, 2 * height);
  if ((this->outImage == NULL) && (this->debugcam))
  {
    this->outImage = cvCreateImage(size, IPL_DEPTH_8U, 1);
    cvGetSubRect(this->outImage, this->outSubImages + 0, cvRect(0, 0, width, height));
    cvGetSubRect(this->outImage, this->outSubImages + 1, cvRect(width, 0, width, height));
    cvGetSubRect(this->outImage, this->outSubImages + 2, cvRect(0, height, width, height));
    cvGetSubRect(this->outImage, this->outSubImages + 3, cvRect(width, height, width, height));
  }

  // Create a main image and copy in the pixels
  switch (this->stored_data.format)
  {
  case PLAYER_CAMERA_FORMAT_MONO8:
    // Copy pixels to input image (grayscale)
    assert(this->inpImage->imageSize >= static_cast<int>(this->stored_data.image_count));
    memcpy(this->inpImage->imageData, this->stored_data.image, this->inpImage->imageSize);
    break;
  case PLAYER_CAMERA_FORMAT_RGB888:
    depth = this->stored_data.bpp / 8;
    assert((depth == 4) || (depth == 3));
    assert(this->inpImage->imageSize >= static_cast<int>((this->stored_data.image_count) / depth));
    for (i = 0; i < height; i++)
    {
      row1 = (reinterpret_cast<unsigned char *>(this->stored_data.image)) + (i * width * depth);
      row2 = (reinterpret_cast<unsigned char *>(this->inpImage->imageData)) + (i * width);
      for (j = 0; j < width; j++)
      {
        row2[0] = static_cast<unsigned char>((static_cast<double>(row1[0]) * 0.3) + (static_cast<double>(row1[1]) * 0.59) + (static_cast<double>(row1[2]) * 0.11));
        row1 += depth;
        row2++;
      }
    }
    break;
  default:
    PLAYER_WARN1("image format [%d] is not supported", this->stored_data.format);
    return -1;
  }

  // Copy original image to output
  if (this->debugcam)
  {
    cvSetZero(this->outImage);
    cvCopy(this->inpImage, this->outSubImages + 0);
  }

  // Clone the input image to our workspace
  this->workImage = cvCloneImage(this->inpImage);

  // Find all the shapes in the working image
  this->FindShapes();

  // Free temp storage
  cvReleaseImage(&this->workImage);
  this->workImage = NULL;

  this->WriteBlobfinderData();
  if (this->debugcam) this->WriteCameraData();

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Having pre-processed the image, find some shapes
void SimpleShape::FindShapes()
{
  int sim;
  double area;
  FeatureSet featureSet;
  CvMemStorage *storage;
  CvSeq *contour;
  CvRect rect;
  Shape *shape;

  // Reset the shape count
  this->shapeCount = 0;

  // Find edges
  cvCanny(this->workImage, this->workImage, this->cannyThresh1, this->cannyThresh2);

  // Copy edges to output image
  if (this->debugcam)
    cvCopy(this->workImage, this->outSubImages + 1);

  // Find contours on a binary image
  storage = cvCreateMemStorage(0);
  cvFindContours(this->workImage, storage, &contour, sizeof(CvContour), 
                 CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
    
  for(; contour != NULL; contour = contour->h_next)
  {
    rect = cvBoundingRect(contour);
    area = fabs(cvContourArea(contour));

    // Discard small/open contours
    if (area < 5 * 5)
      continue;

    // Discard the countour generated from the image border
    if (rect.x < 5 || rect.y < 5)
      continue;
    if (rect.x + rect.width >= this->workImage->width - 5)
      continue;
    if (rect.y + rect.height >= this->workImage->height - 5)
      continue;

    
    // Draw eligable contour on the output image; useful for debugging
    if (this->debugcam)
      cvDrawContours(this->outSubImages + 2, contour, CV_RGB(255, 255, 255),
                     CV_RGB(255, 255, 255), 0, 1, 8);

    // Compute the contour features
    this->ExtractFeatureSet((CvContour*) contour, &featureSet);

    // Match against the model
    sim = this->MatchFeatureSet(&featureSet, &this->modelFeatureSet);
    if (sim > 0)
      continue;

    // Draw contour on the main image; useful for debugging
    if (this->debugcam)
    {
      cvDrawContours(this->outSubImages + 3, contour, CV_RGB(128, 128, 128),
                     CV_RGB(128, 128, 128), 0, 1, 8);
      cvRectangle(this->outSubImages + 3, cvPoint(rect.x, rect.y),
                  cvPoint(rect.x + rect.width, rect.y + rect.height), CV_RGB(255, 255, 255), 1);
    }

    // Check for overrun
    if (this->shapeCount >= (sizeof(this->shapes) / sizeof(class Shape)))
    {
      PLAYER_WARN("image contains too many shapes");
      break;
    }
    
    // Add the shape to our internal list
    shape = this->shapes + this->shapeCount++;
    shape->id = -1;
    shape->ax = rect.x;
    shape->ay = rect.y;
    shape->bx = rect.x + rect.width;
    shape->by = rect.y + rect.height;
  }

  cvReleaseMemStorage(&storage);
  return;
}


////////////////////////////////////////////////////////////////////////////////
// Extract a feature set for the given contour
void SimpleShape::ExtractFeatureSet(CvContour *contour, FeatureSet *feature)
{
  int i;
  CvBox2D box;
  CvPoint *p;
  CvRect rect;
  CvSeq *poly;
  double aa, bb;
  double dx, dy;
  double var;
  
  // Get the moments (we will use the Hu invariants)
  cvMoments(contour, &feature->moments);

  // Compute the compactness measure: perimeter squared divided by
  // area.  
  rect = cvBoundingRect(contour);
  feature->compact = (contour->total * contour->total) / fabs(cvContourArea(contour));

  // Compute elliptical variance
  box = cvFitEllipse2(contour);

  aa = box.size.width * box.size.width / 4;
  bb = box.size.height * box.size.height / 4;

  var = 0.0;
  for (i = 0; i < contour->total; i++)
  {
    p = CV_GET_SEQ_ELEM(CvPoint, contour, i);
    dx = (p->x - box.center.x);
    dy = (p->y - box.center.y);
    var += dx * dx / aa + dy * dy / bb;
  }
  var /= contour->total;

  feature->variance = var;

  // Fit a polygon
  poly = cvApproxPoly(contour, sizeof(CvContour), NULL, CV_POLY_APPROX_DP,
                      cvContourPerimeter(contour) * 0.02, 0);
  feature->vertexCount = poly->total;

  CvPoint *a, *b, *c;
  double ax, ay, bx, by, cx, cy;
  double d, n, m;
  
  // Construct a string describing the polygon (used for syntactic
  // matching)
  for (i = 0; i < poly->total; i++)
  {
    a = CV_GET_SEQ_ELEM(CvPoint, poly, i);
    b = CV_GET_SEQ_ELEM(CvPoint, poly, (i + 1) % poly->total);
    c = CV_GET_SEQ_ELEM(CvPoint, poly, (i + 2) % poly->total);

    // Compute normalized segment vectors
    ax = b->x - a->x;
    ay = b->y - a->y;
    d = sqrt(ax * ax + ay * ay);
    ax /= d;
    ay /= d;

    bx = -ay;
    by = +ax;
    
    cx = c->x - b->x;
    cy = c->y - b->y;
    d = sqrt(cx * cx + cy * cy);
    cx /= d;
    cy /= d;

    // Compute projections
    n = cx * ax + cy * ay;
    m = cx * bx + cy * by;

    // Add a symbol; right now this is just -1, +1, corresponding to
    // an inside or outside corner
    assert((size_t) i < sizeof(feature->vertexString) / sizeof(feature->vertexString[0]));
    feature->vertexString[i] = (m < 0 ? -1 : +1);
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Compute similarity measure on features
int SimpleShape::MatchFeatureSet(FeatureSet *a, FeatureSet *b)
{
  int i, j;
  int sim, minSim;
  int na, nb;
  
  if (a->vertexCount != b->vertexCount)
    return INT_MAX;

  minSim = INT_MAX;
  
  // Look for the lowest dissimalarity by trying all possible
  // string shifts
  for (i = 0; i < a->vertexCount; i++)
  {
    sim = 0;
    
    for (j = 0; j < a->vertexCount; j++)
    {
      na = a->vertexString[j];
      nb = b->vertexString[(j + i) % a->vertexCount];
      sim += (na != nb);
    }

    if (sim < minSim)
      minSim = sim;
  }

  return minSim;
}


////////////////////////////////////////////////////////////////////////////////
// Write blobfinder data
void SimpleShape::WriteBlobfinderData()
{
  unsigned int i;
  //size_t size;
  Shape *shape;
  player_blobfinder_data_t data;

  // Se the image dimensions
  data.width = (this->stored_data.width);
  data.height = (this->stored_data.height);

  data.blobs_count = (this->shapeCount);
  data.blobs = (player_blobfinder_blob_t*)calloc(shapeCount,sizeof(data.blobs[0]));
  for (i = 0; i < this->shapeCount; i++)
  {
    shape = this->shapes + i;

    // Set the data to pass back
    data.blobs[i].id = shape->id;  // TODO
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
  Publish(blobfinder_addr,PLAYER_MSGTYPE_DATA,PLAYER_BLOBFINDER_DATA_BLOBS,&data);
  if (data.blobs) free(data.blobs);
  data.blobs = NULL;
  data.blobs_count = 0;

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Write camera data; this is a little bit naughty: we re-use the
// input camera data, but modify the pixels
void SimpleShape::WriteCameraData()
{
  player_camera_data_t * outCameraData;

  if (this->outImage == NULL)
    return;

  outCameraData = reinterpret_cast<player_camera_data_t *>(malloc(sizeof(player_camera_data_t)));
  assert(outCameraData);
  // Do some byte swapping
  outCameraData->width = this->outImage->width;
  outCameraData->height = this->outImage->height;
  outCameraData->bpp = 8;
  outCameraData->format = PLAYER_CAMERA_FORMAT_MONO8;
  outCameraData->compression = PLAYER_CAMERA_COMPRESS_RAW;
  outCameraData->fdiv = 0;
  outCameraData->image_count = this->outImage->imageSize;
  outCameraData->image = NULL;
  if (outCameraData->image_count)
  {
    outCameraData->image = reinterpret_cast<uint8_t *>(malloc(outCameraData->image_count));
    // Copy in the pixels
    memcpy(outCameraData->image, this->outImage->imageData, outCameraData->image_count);
  }
  Publish(this->device_addr,
          PLAYER_MSGTYPE_DATA, PLAYER_CAMERA_DATA_STATE,
          reinterpret_cast<void *>(outCameraData),
          0, NULL, false); // this call should also free outCameraData and outCameraData->image
}
