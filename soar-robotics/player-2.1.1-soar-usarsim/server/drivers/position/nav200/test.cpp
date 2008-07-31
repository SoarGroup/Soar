/*#include <stdlib.h>
#include <time.h>
#include "nav200.h"


int main(int argc,char ** argv)
{
  //Standby mode
  PositionXY reflector;
  Nav200 testing;
  testing.Initialise("/dev/ttyS0");
  if (testing.EnterStandby())
  {
    printf("\n\n\nEntered Standby\n\n");
//     if(testing.rotateDirection(0))
//       printf("direction of rotation is anti-clockwise\n");
//     return 0;
    printf("Version = %08X\n",testing.GetVersionNumber());
    printf("Serial = %d\n", testing.GetDeviceSerial());
    printf("Version = %s\n",testing.GetVersionString());



      if(testing.InsertReflectorPosition(1,0,100,200))
        printf("reflectors added in layer 1\n");

      if(testing.InsertReflectorPosition(1,0,300,400))
        printf("reflectors added in layer 1\n");

      if(testing.InsertReflectorPosition(1,0,500,600))
        printf("reflectors added in layer 1\n");
//       {
//         printf("reflector added: %d: %d, %d\n",i,reflectordata[i].x,reflectordata[i].y);
//       }



//     if(testing.GetReflectorPosition(0, 0, reflector)) // get reflector
//       printf("\nGot Reflector 01: position X=%d, Y=%d\n",reflector.x,reflector.y);
//     if(testing.GetReflectorPosition(0, 1, reflector)) // get reflector
//       printf("Got Reflector 02: position X=%d, Y=%d\n",reflector.x,reflector.y);
//     if(testing.GetReflectorPosition(0, 2, reflector)) // get reflector
//       printf("Got Reflector 03: position X=%d, Y=%d\n",reflector.x,reflector.y);
//     if(testing.GetReflectorPosition(0, 3, reflector)) // get reflector
//       printf("Got Reflector 04: position X=%d, Y=%d\n",reflector.x,reflector.y);


// //     delete reflectors
//     if(testing.DeleteReflectorPosition(0,0,reflector))
// //       printf("\n");
//       printf("\n1:Got Reflector: position X=%d, Y=%d\n",reflector.x,reflector.y);
//     if(testing.DeleteReflectorPosition(0,0,reflector))
// //       printf("\n");
//       printf("2:Got Reflector: position X=%d, Y=%d\n",reflector.x,reflector.y);
//     if(testing.DeleteReflectorPosition(0,0,reflector))
// //       printf("\n");
//       printf("3:Got Reflector: position X=%d, Y=%d\n",reflector.x,reflector.y);
  }

  //Mapping mode
  PositionXY reflectordata[4];
  int max = 0;
  if(testing.EnterMapping())
  {
    printf("\n\n\nEntered mapping\n\n");

    max = testing.StartMapping(0, 0, 0, 0, 100);

    printf("Number of reflectors scanned = %d\n",max);

    for(int i=0; i<max; i++)
    {
        if(testing.MappingPosition(0,i,reflectordata[i]))
          printf("position of Reflector %d: position X=%d, Y=%d\n",i, reflectordata[i].x,reflectordata[i].y);
    }

  }


  if(testing.EnterStandby())
  {
     for(int i=max-1; i>=0; i--)
      if(testing.InsertReflectorPosition(0,0,reflectordata[i].x,reflectordata[i].y))
      {
        printf("reflector added: %d: %d, %d\n",i,reflectordata[i].x,reflectordata[i].y);
      }
   }

//   printf("\ntesting on mapping mode is done\n");

  LaserPos laser;
  if(testing.EnterPositioning())
  {
      printf("\n\n\nEntered positioning mode\n\n");
      if(testing.EnterPositioningInput(3))
        printf("\nactive position mode with sliding\n\n");

//       if(testing.ChangeLayer(0))
//         printf("\n\nlayer changed\n");
// 
//       if(testing.ChangeLayerDefPosition(0, 0, 0, 0))
//         printf("\nselect new layer with position definition\n");


      if(testing.SetActionRadii(200,1000))
        printf("changed operation radii\n");


      if(testing.SelectNearest(max))
        printf("selected N nearest\n");


      if(testing.GetPositionAuto(laser))
        printf("Position of the laser scanner: X=%d, Y=%d, orientation=%d, quality=%d, number of reflectors = %d\n",laser.pos.x,laser.pos.y,laser.orientation,laser.quality,laser.number);


      if(testing.GetPositionSpeed(1000,1000,laser))
        printf("Position of the laser scanner: X=%d, Y=%d, orientation=%d, quality=%d, number of reflectors = %d\n",laser.pos.x,laser.pos.y,laser.orientation,laser.quality,laser.number);
      if(testing.GetPositionSpeedVelocity(1000,1000,25,laser))
        printf("Position of the laser scanner: X=%d, Y=%d, orientation=%d, quality=%d, number of reflectors = %d\n",laser.pos.x,laser.pos.y,laser.orientation,laser.quality,laser.number);
      if(testing.GetPositionSpeedVelocityAbsolute(1000,1000,25,laser))
        printf("Position of the laser scanner: X=%d, Y=%d, orientation=%d, quality=%d, number of reflectors = %d\n",laser.pos.x,laser.pos.y,laser.orientation,laser.quality,laser.number);

      if(testing.ChangeLayer(1))
        printf("\n\nlayer changed\n");

      if(testing.ChangeLayerDefPosition(0, 0, 0, 0))
        printf("\nselect new layer with position definition\n");




  }


  printf("\nEnd of positioning mode\n\n\n\n");


  // upload mode
  ReflectorData upload_reflector;
  testing.EnterStandby();
  if(testing.EnterUpload())
  {
      printf("\n\n\nEntered upload mode\n\n\n");
      if(testing.GetUploadTrans(0,upload_reflector))
        printf("Uploaded reflector is: X = %d, Y = %d, layer = %d, number = %d\n",upload_reflector.pos.x,upload_reflector.pos.y,upload_reflector.layer,upload_reflector.number);
   if(testing.GetUploadTrans(0,upload_reflector))
        printf("Uploaded reflector is: X = %d, Y = %d, layer = %d, number = %d\n",upload_reflector.pos.x,upload_reflector.pos.y,upload_reflector.layer,upload_reflector.number);
   if(testing.GetUploadTrans(0,upload_reflector))
        printf("Uploaded reflector is: X = %d, Y = %d, layer = %d, number = %d\n",upload_reflector.pos.x,upload_reflector.pos.y,upload_reflector.layer,upload_reflector.number);
  }


  // download mode

  testing.EnterStandby();
  if(testing.EnterDownload())
  {
      printf("\n\n\nEntered download mode\n\n\n");
      if(testing.DownloadReflector(2,0,111,222))
        printf("Downloaded reflector\n");
      if(testing.DownloadReflector(2,1,333,444))
        printf("Downloaded reflector\n");
      if(testing.DownloadReflector(2,2,555,666))
        printf("Downloaded reflector\n");
      if(testing.DownloadReflector(2,255,0,0))
        printf("Downloaded is done\n\n");
  }

  // upload mode
  ReflectorData upload_reflectors[5];
  ReflectorData temp;
  testing.EnterStandby();
  if(testing.EnterUpload())
  {
      printf("\n\n\nEntered upload mode\n\n\n");
      int i=0;
      while(1)
      {
        if(testing.GetUploadTrans(2,temp))
          printf("Uploaded reflector is: X = %d, Y = %d, layer = %d, number = %d\n",temp.pos.x,temp.pos.y,temp.layer,temp.number);
        if(temp.number==255)
          break;
        upload_reflectors[i].pos.x = temp.pos.x;
        upload_reflectors[i].pos.y = temp.pos.y;
        upload_reflectors[i].layer = temp.layer;
        upload_reflectors[i].number = temp.number;
        i++;
      }
  }




  printf("download mode ends\n\n\n");

  if(testing.EnterStandby())
  {
      for(int ii=0; ii<max; ii++)
        if(testing.DeleteReflectorPosition(0,0,reflector))
          printf("Delete Reflector: position X=%d, Y=%d\n",reflector.x,reflector.y);

      for(int ii=0; ii<3; ii++)
        if(testing.DeleteReflectorPosition(1,0,reflector))
          printf("Delete Reflector in layer 1: position X=%d, Y=%d\n",reflector.x,reflector.y);

      for(int ii=0; ii<3; ii++)
        if(testing.DeleteReflectorPosition(2,0,reflector))
          printf("Delete Reflector in layer 2: position X=%d, Y=%d\n",reflector.x,reflector.y);

  }


  return 0;
}



//not working list
//   bool rotateDirection(uint8_t direction); //absolutely not working for some unknown reason
//   bool DeleteReflectorPosition(uint8_t layer, uint8_t number, PositionXY & reflector); <-- return incorrect X value
*/

int main()
{}
