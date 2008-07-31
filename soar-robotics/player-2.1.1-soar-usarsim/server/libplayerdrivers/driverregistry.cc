/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
 *
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
/********************************************************************
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************/

/*
 * $Id: driverregistry.cc 6316 2008-04-14 19:51:33Z thjc $
 */
#if HAVE_CONFIG_H
  #include <config.h>  // to get the INCLUDE_foo macros
#endif

#include <libplayercore/playercore.h>

/* prototype driver-specific init funcs */
#ifdef INCLUDE_ALSA
void Alsa_Register(DriverTable *table);
#endif

#ifdef INCLUDE_ARTOOLKITPLUS
void ARToolkitPlusDriver_Register(DriverTable *table);
#endif

#ifdef INCLUDE_BUMPERSAFE
void BumperSafe_Register(DriverTable *table);
#endif

#ifdef INCLUDE_CHATTERBOX
void Chatterbox_Register(DriverTable *table);
#endif

#ifdef INCLUDE_MRICP
void MrIcp_Register(DriverTable *table);
#endif

#ifdef INCLUDE_ND
void ND_Register(DriverTable *table);
#endif

#ifdef INCLUDE_MBICP
void mbicp_Register(DriverTable *table);
#endif

#ifdef INCLUDE_GARMINNMEA
void GarminNMEA_Register(DriverTable* table);
#endif

#ifdef INCLUDE_VMAPFILE
void VMapFile_Register(DriverTable* table);
#endif

#ifdef INCLUDE_MAPFILE
void MapFile_Register(DriverTable* table);
#endif

#ifdef INCLUDE_NIMU
void NIMU_Register(DriverTable* table);
#endif

#ifdef INCLUDE_RELAY
void Relay_Register(DriverTable* table);
#endif

#ifdef INCLUDE_MAPCSPACE
void MapCspace_Register(DriverTable* table);
#endif

#ifdef INCLUDE_MAPSCALE
void MapScale_Register(DriverTable* table);
#endif

#ifdef INCLUDE_AMTECPOWERCUBE
void AmtecPowerCube_Register(DriverTable* table);
#endif

#ifdef INCLUDE_CLODBUSTER
void ClodBuster_Register(DriverTable* table);
#endif

#ifdef INCLUDE_OBOT
void Obot_Register(DriverTable* table);
#endif

#ifdef INCLUDE_ER1
void ER_Register(DriverTable* table);
#endif

#ifdef INCLUDE_WAVEFRONT
void Wavefront_Register(DriverTable* table);
#endif

#ifdef INCLUDE_SEGWAYRMP
void SegwayRMP_Register(DriverTable* table);
#endif

#ifdef INCLUDE_LASERPOSEINTERPOLATOR
void LaserPoseInterp_Register(DriverTable* table);
#endif

#ifdef INCLUDE_SICKLMS200
void SickLMS200_Register(DriverTable* table);
#endif

#ifdef INCLUDE_SICKLMS400
void SickLMS400_Register(DriverTable* table);
#endif

#ifdef INCLUDE_SICKNAV200
void SickNAV200_Register(DriverTable* table);
#endif

#ifdef INCLUDE_SICKPLS
// PLS driver was integrated into LMS200
void SickLMS200_Register(DriverTable* table);
#endif

#ifdef INCLUDE_RS4LEUZE
void RS4LeuzeLaserDriver_Register(DriverTable* table);
#endif

#ifdef INCLUDE_PHIDGETRFID
void PhidgetRFID_Register(DriverTable* table);
#endif

#ifdef INCLUDE_PHIDGETIFK
void PhidgetIFK_Register(DriverTable* table);
#endif


#ifdef INCLUDE_SICKRFI341
void SickRFI341_Register(DriverTable* table);
#endif

#ifdef INCLUDE_SICKS3000
void SickS3000_Register(DriverTable* table);
#endif

#ifdef INCLUDE_SERIALSTREAM
void SerialStream_Register(DriverTable* table);
#endif

#ifdef INCLUDE_TCPSTREAM
void TCPStream_Register(DriverTable* table);
#endif


#ifdef INCLUDE_WBR914
void wbr914_Register(DriverTable* table);
#endif

#ifdef INCLUDE_ROOMBA
void Roomba_Register(DriverTable* table);
#endif

#ifdef INCLUDE_CREATE
void Create_Register(DriverTable* table);
#endif

#ifdef INCLUDE_GARCIA
void GarciaDriver_Register(DriverTable* table);
#endif

#ifdef INCLUDE_URGLASER
void URGLaserDriver_Register(DriverTable* table);
#endif

#ifdef INCLUDE_ACTS
void Acts_Register(DriverTable* table);
#endif

#ifdef INCLUDE_CMVISION
void CMVision_Register(DriverTable* table);
#endif

#ifdef INCLUDE_STATGRAB
void StatGrabDriver_Register(DriverTable* table);
#endif


#ifdef INCLUDE_CMUCAM2
void Cmucam2_Register(DriverTable* table);
// REMOVE void Cmucam2blobfinder_Register(DriverTable* table);
// REMOVE void Cmucam2ptz_Register(DriverTable* table);
#endif

#ifdef INCLUDE_UPCBARCODE
void UPCBarcode_Register(DriverTable* table);
#endif

#ifdef INCLUDE_SIMPLESHAPE
void SimpleShape_Register(DriverTable* table);
#endif

#ifdef INCLUDE_FESTIVAL
void Festival_Register(DriverTable* table);
#endif

#ifdef INCLUDE_SPHINX2
void Sphinx2_Register(DriverTable* table);
#endif

#ifdef INCLUDE_LASERBAR
void LaserBar_Register(DriverTable* table);
#endif

#ifdef INCLUDE_LASERBARCODE
void LaserBarcode_Register(DriverTable* table);
#endif

#ifdef INCLUDE_LASERVISUALBARCODE
void LaserVisualBarcode_Register(DriverTable* table);
#endif

#ifdef INCLUDE_LASERVISUALBW
void LaserVisualBW_Register(DriverTable* table);
#endif

#ifdef INCLUDE_LASERCSPACE
void LaserCSpace_Register(DriverTable* table);
#endif

#ifdef INCLUDE_LASERRESCAN
void LaserRescan_Register(DriverTable* table);
#endif

#ifdef INCLUDE_LASERCUTTER
void LaserCutter_Register(DriverTable* table);
#endif

#ifdef INCLUDE_LASERSAFE
void LaserSafe_Register(DriverTable *table);
#endif

#ifdef INCLUDE_SONYEVID30
void SonyEVID30_Register(DriverTable* table);
#endif

#ifdef INCLUDE_PTU46
void PTU46_Register(DriverTable* table);
#endif

#ifdef INCLUDE_CANONVCC4
void canonvcc4_Register(DriverTable* table);
#endif

#ifdef INCLUDE_FLOCKOFBIRDS
void FlockOfBirds_Register(DriverTable* table);
#endif

#ifdef INCLUDE_DUMMY
void Dummy_Register(DriverTable* table);
#endif

#ifdef INCLUDE_PASSTHROUGH
void PassThrough_Register(DriverTable* table);
#endif

#ifdef INCLUDE_LOGFILE
void WriteLog_Register(DriverTable* table);
void ReadLog_Register(DriverTable* table);
#endif

#ifdef INCLUDE_KARTOWRITER
void KartoLogger_Register(DriverTable* table);
#endif

#ifdef INCLUDE_P2OS
void P2OS_Register(DriverTable* table);
#endif

#ifdef INCLUDE_ERRATIC
void Erratic_Register(DriverTable* table);
#endif

#ifdef INCLUDE_RFLEX
void RFLEX_Register(DriverTable* table);
#endif

#ifdef INCLUDE_LINUXWIFI
void LinuxWiFi_Register(DriverTable *table);
#endif

#ifdef INCLUDE_AODV
void Aodv_Register(DriverTable *table);
#endif

#ifdef INCLUDE_IWSPY
void Iwspy_Register(DriverTable *table);
#endif

#ifdef INCLUDE_LINUXJOYSTICK
void LinuxJoystick_Register(DriverTable* table);
#endif

#ifdef INCLUDE_REB
void REB_Register(DriverTable *table);
#endif

#ifdef INCLUDE_KHEPERA
void Khepera_Register(DriverTable *table);
#endif

#ifdef INCLUDE_MIXER
void Mixer_Register(DriverTable* table);
#endif

#ifdef INCLUDE_RWI
void RWIBumper_Register(DriverTable* table);
void RWILaser_Register(DriverTable* table);
void RWIPosition_Register(DriverTable* table);
void RWIPower_Register(DriverTable* table);
void RWISonar_Register(DriverTable* table);
#endif

#ifdef INCLUDE_ISENSE
void InertiaCube2_Register(DriverTable* table);
#endif

#ifdef INCLUDE_MICROSTRAIN
void MicroStrain3DMG_Register(DriverTable* table);
#endif

#ifdef INCLUDE_YARPIMAGE
void Yarp_Image_Register(DriverTable* table);
#endif

#ifdef INCLUDE_MICA2
void Mica2_Register(DriverTable* table);
#endif

#ifdef INCLUDE_AMTECM5
void AmtecM5_Register(DriverTable* table);
#endif

#ifdef INCLUDE_RCORE_XBRIDGE
void RCore_XBridge_Register(DriverTable* table);
#endif

#ifdef INCLUDE_SR3000
void SR3000_Register(DriverTable* table);
#endif

#ifdef INCLUDE_ACCEL_CALIB
void Accel_Calib_Register(DriverTable* table);
#endif

#ifdef INCLUDE_XSENSMT
void XSensMT_Register(DriverTable* table);
#endif

#ifdef INCLUDE_LASERPTZCLOUD
void LaserPTZCloud_Register(DriverTable* table);
#endif

#ifdef INCLUDE_INAV
void INav_Register(DriverTable *table);
#endif

#ifdef INCLUDE_VFH
void VFH_Register(DriverTable *table);
#endif

#ifdef INCLUDE_MCL
void RegularMCL_Register(DriverTable* table);
#endif

#ifdef INCLUDE_AMCL
void AdaptiveMCL_Register(DriverTable* table);
#endif

#ifdef INCLUDE_LIFOMCOM
void LifoMCom_Register(DriverTable* table);
#endif

#ifdef INCLUDE_CAMERAV4L
void CameraV4L_Register(DriverTable *table);
#endif

#ifdef INCLUDE_CAMERAUVC
void CameraUVC_Register(DriverTable *table);
#endif

#ifdef INCLUDE_SPHERE
void SphereDriver_Register(DriverTable *table);
#endif

#ifdef INCLUDE_CAMERA1394
void Camera1394_Register(DriverTable *table);
#endif

#ifdef INCLUDE_IMAGESEQ
void ImageSeq_Register(DriverTable* table);
#endif

#ifdef INCLUDE_CAMERACOMPRESS
void CameraCompress_Register(DriverTable* table);
void CameraUncompress_Register(DriverTable* table);
#endif

#ifdef INCLUDE_SERVICE_ADV_MDNS
void ServiceAdvMDNS_Register(DriverTable* table);
#endif

#ifdef INCLUDE_FAKELOCALIZE
void FakeLocalize_Register(DriverTable* table);
#endif

#ifdef INCLUDE_STAGECLIENT
void StgSimulation_Register(DriverTable *table);
void StgLaser_Register(DriverTable *table);
void StgPosition_Register(DriverTable *table);
void StgSonar_Register(DriverTable *table);
void StgEnergy_Register(DriverTable *table);
void StgBlobfinder_Register(DriverTable *table);
void StgFiducial_Register(DriverTable *table);
//void StgBlinkenlight_Register(DriverTable *table);
#endif

#ifdef INCLUDE_NOMAD
void Nomad_Register(DriverTable *driverTable);
void NomadPosition_Register(DriverTable *driverTable);
void NomadSonar_Register(DriverTable *driverTable);
//void NomadBumper_Register(DriverTable *driverTable);
//void NomadSpeech_Register(DriverTable *driverTable);
#endif

#ifdef INCLUDE_INSIDEM300
void InsideM300_Register(DriverTable *driverTable);
#endif

#ifdef INCLUDE_SKYETEKM1
void SkyetekM1_Register(DriverTable *driverTable);
#endif

#ifdef INCLUDE_LASERTORANGER
void LaserToRanger_Register (DriverTable* table);
#endif

#ifdef INCLUDE_SONARTORANGER
void SonarToRanger_Register (DriverTable* table);
#endif

#ifdef INCLUDE_ROBOTEQ
void roboteq_Register (DriverTable* table);
#endif

#ifdef INCLUDE_ROBOTINO
void robotino_Register (DriverTable* table);
#endif

#ifdef INCLUDE_PBSLASER
void PBSDriver_Register (DriverTable* table);
#endif

#ifdef INCLUDE_POSTGIS
void PostGIS_Register (DriverTable* table);
#endif

#ifdef INCLUDE_VEC2MAP
void Vec2Map_Register(DriverTable * table);
#endif

#ifdef INCLUDE_BUMPER2LASER
void Bumper2Laser_Register(DriverTable * table);
#endif

#ifdef INCLUDE_LOCALBB
void LocalBB_Register (DriverTable* table);
#endif

#ifdef INCLUDE_GBXSICKACFR
void GbxSickAcfr_Register(DriverTable* table);
#endif

#ifdef INCLUDE_URG_NZ
void UrgDriver_Register(DriverTable* table);
#endif

#ifdef INCLUDE_USARSIM
void UsBot_Register(DriverTable *driverTable);
void UsPosition_Register(DriverTable *driverTable);
void UsPosition3d_Register(DriverTable *driverTable);
void UsFakeLocalize_Register(DriverTable* table);
void UsLaser_Register(DriverTable *driverTable);
void UsLaser3d_Register(DriverTable *driverTable);
void UsSonar_Register(DriverTable *driverTable);
void RhIr_Register(DriverTable *driverTable);
void RhPyro_Register(DriverTable *driverTable);
void UsFiducial_Register(DriverTable *driverTable);
void UsVictFiducial_Register(DriverTable *driverTable);
void UsPtz_Register(DriverTable *driverTable);
#endif


/*
 * this function will be called at startup.  all available devices should
 * be added to the driverTable here.  they will be instantiated later as
 * necessary.
 */
void
player_register_drivers()
{
#ifdef INCLUDE_ALSA
  Alsa_Register(driverTable);
#endif

#ifdef INCLUDE_ARTOOLKITPLUS
  ARToolkitPlusDriver_Register(driverTable);
#endif

#ifdef INCLUDE_BUMPERSAFE
  BumperSafe_Register(driverTable);
#endif

#ifdef INCLUDE_MRICP
  MrIcp_Register(driverTable);
#endif

  #ifdef INCLUDE_ND
  ND_Register(driverTable);
#endif

#ifdef INCLUDE_MBICP
  mbicp_Register(driverTable);
#endif

#ifdef INCLUDE_GARMINNMEA
  GarminNMEA_Register(driverTable);
#endif

#ifdef INCLUDE_MAPFILE
  MapFile_Register(driverTable);
#endif

#ifdef INCLUDE_VMAPFILE
  VMapFile_Register(driverTable);
#endif

#ifdef INCLUDE_NIMU
  NIMU_Register(driverTable);
#endif

#ifdef INCLUDE_RELAY
  Relay_Register(driverTable);
#endif

#ifdef INCLUDE_MAPCSPACE
  MapCspace_Register(driverTable);
#endif

#ifdef INCLUDE_MAPSCALE
  MapScale_Register(driverTable);
#endif

#ifdef INCLUDE_AMTECPOWERCUBE
  AmtecPowerCube_Register(driverTable);
#endif

#ifdef INCLUDE_CLODBUSTER
  ClodBuster_Register(driverTable);
#endif

#ifdef INCLUDE_OBOT
  Obot_Register(driverTable);
#endif

#ifdef INCLUDE_ER1
  ER_Register(driverTable);
#endif

#ifdef INCLUDE_WAVEFRONT
  Wavefront_Register(driverTable);
#endif

#ifdef INCLUDE_SEGWAYRMP
  SegwayRMP_Register(driverTable);
#endif

#ifdef INCLUDE_LASERPOSEINTERPOLATOR
  LaserPoseInterp_Register(driverTable);
#endif

#ifdef INCLUDE_GARCIA
  GarciaDriver_Register(driverTable);
#endif

#ifdef INCLUDE_SICKLMS200
  SickLMS200_Register(driverTable);
#endif

#ifdef INCLUDE_SICKLMS400
  SickLMS400_Register(driverTable);
#endif

#ifdef INCLUDE_SICKNAV200
  SickNAV200_Register(driverTable);
#endif

#ifdef INCLUDE_RS4LEUZE
  RS4LeuzeLaserDriver_Register(driverTable);
#endif

#ifdef INCLUDE_SICKPLS
#ifndef INCLUDE_SICKLMS200
  SickLMS200_Register(driverTable);
#endif
#endif

#ifdef INCLUDE_PHIDGETRFID
  PhidgetRFID_Register(driverTable);
#endif

#ifdef INCLUDE_PHIDGETIFK
  PhidgetIFK_Register(driverTable);
#endif

#ifdef INCLUDE_SICKRFI341
  SickRFI341_Register(driverTable);
#endif

#ifdef INCLUDE_SICKS3000
  SickS3000_Register(driverTable);
#endif

#ifdef INCLUDE_SERIALSTREAM
  SerialStream_Register(driverTable);
#endif

#ifdef INCLUDE_TCPSTREAM
  TCPStream_Register(driverTable);
#endif

#ifdef INCLUDE_WBR914
  wbr914_Register(driverTable);
#endif

#ifdef INCLUDE_ROOMBA
  Roomba_Register(driverTable);
#endif

#ifdef INCLUDE_CREATE
  Create_Register(driverTable);
#endif

#ifdef INCLUDE_URGLASER
  URGLaserDriver_Register(driverTable);
#endif

#ifdef INCLUDE_ACTS
  Acts_Register(driverTable);
#endif

#ifdef INCLUDE_CMVISION
  CMVision_Register(driverTable);
#endif

#ifdef INCLUDE_STATGRAB
  StatGrabDriver_Register(driverTable);
#endif

#ifdef INCLUDE_CHATTERBOX
  Chatterbox_Register(driverTable);
#endif

#ifdef INCLUDE_CMUCAM2
  Cmucam2_Register(driverTable);
  // REMOVE Cmucam2blobfinder_Register(driverTable);
  // REMOVE Cmucam2ptz_Register(driverTable);
#endif

#ifdef INCLUDE_UPCBARCODE
  UPCBarcode_Register(driverTable);
#endif

#ifdef INCLUDE_SIMPLESHAPE
  SimpleShape_Register(driverTable);
#endif

#ifdef INCLUDE_FESTIVAL
  Festival_Register(driverTable);
#endif

#ifdef INCLUDE_SPHINX2
  Sphinx2_Register(driverTable);
#endif

#ifdef INCLUDE_LASERBAR
  LaserBar_Register(driverTable);
#endif

#ifdef INCLUDE_LASERBARCODE
  LaserBarcode_Register(driverTable);
#endif

#ifdef INCLUDE_LASERVISUALBARCODE
  LaserVisualBarcode_Register(driverTable);
#endif

#ifdef INCLUDE_LASERVISUALBW
  LaserVisualBW_Register(driverTable);
#endif

#ifdef INCLUDE_LASERCSPACE
  LaserCSpace_Register(driverTable);
#endif

#ifdef INCLUDE_LASERRESCAN
  LaserRescan_Register(driverTable);
#endif

#ifdef INCLUDE_LASERCUTTER
  LaserCutter_Register(driverTable);
#endif

#ifdef INCLUDE_LASERSAFE
  LaserSafe_Register(driverTable);
#endif

#ifdef INCLUDE_RFLEX
  RFLEX_Register(driverTable);
#endif

#ifdef INCLUDE_SONYEVID30
  SonyEVID30_Register(driverTable);
#endif

#ifdef INCLUDE_PTU46
  PTU46_Register(driverTable);
#endif

#ifdef INCLUDE_CANONVCC4
  canonvcc4_Register(driverTable);
#endif

#ifdef INCLUDE_FLOCKOFBIRDS
  FlockOfBirds_Register(driverTable);
#endif

#ifdef INCLUDE_DUMMY
  Dummy_Register(driverTable);
#endif

#ifdef INCLUDE_PASSTHROUGH
  PassThrough_Register(driverTable);
#endif

#ifdef INCLUDE_LOGFILE
  WriteLog_Register(driverTable);
  ReadLog_Register(driverTable);
#endif

#ifdef INCLUDE_KARTOWRITER
  KartoLogger_Register(driverTable);
#endif

#ifdef INCLUDE_P2OS
  P2OS_Register(driverTable);
#endif

#ifdef INCLUDE_ERRATIC
  Erratic_Register(driverTable);
#endif

#ifdef INCLUDE_MIXER
  Mixer_Register(driverTable);
#endif

#ifdef INCLUDE_RWI
  RWIPosition_Register(driverTable);
  RWISonar_Register(driverTable);
  RWILaser_Register(driverTable);
  RWIBumper_Register(driverTable);
  RWIPower_Register(driverTable);
#endif

#ifdef INCLUDE_LINUXWIFI
  LinuxWiFi_Register(driverTable);
#endif

#ifdef INCLUDE_AODV
  Aodv_Register(driverTable);
#endif

#ifdef INCLUDE_IWSPY
  Iwspy_Register(driverTable);
#endif

#ifdef INCLUDE_LINUXJOYSTICK
  LinuxJoystick_Register(driverTable);
#endif

#ifdef INCLUDE_REB
  REB_Register(driverTable);
#endif

#ifdef INCLUDE_KHEPERA
  Khepera_Register(driverTable);
#endif

#ifdef INCLUDE_ISENSE
  InertiaCube2_Register(driverTable);
#endif

#ifdef INCLUDE_MICROSTRAIN
  MicroStrain3DMG_Register(driverTable);
#endif

#ifdef INCLUDE_YARPIMAGE
  Yarp_Image_Register(driverTable);
#endif

#ifdef INCLUDE_MICA2
  Mica2_Register(driverTable);
#endif

#ifdef INCLUDE_AMTECM5
  AmtecM5_Register(driverTable);
#endif

#ifdef INCLUDE_RCORE_XBRIDGE
  RCore_XBridge_Register(driverTable);
#endif

#ifdef INCLUDE_SR3000
  SR3000_Register(driverTable);
#endif

#ifdef INCLUDE_ACCEL_CALIB
  Accel_Calib_Register(driverTable);
#endif

#ifdef INCLUDE_XSENSMT
  XSensMT_Register(driverTable);
#endif

#ifdef INCLUDE_LASERPTZCLOUD
  LaserPTZCloud_Register(driverTable);
#endif

#ifdef INCLUDE_INAV
  INav_Register(driverTable);
#endif

#ifdef INCLUDE_VFH
  VFH_Register(driverTable);
#endif

#ifdef INCLUDE_MCL
  RegularMCL_Register(driverTable);
#endif

#ifdef INCLUDE_AMCL
  AdaptiveMCL_Register(driverTable);
#endif

#ifdef INCLUDE_LIFOMCOM
  LifoMCom_Register(driverTable);
#endif

#ifdef INCLUDE_CAMERAV4L
  CameraV4L_Register(driverTable);
#endif

#ifdef INCLUDE_CAMERAUVC
  CameraUVC_Register(driverTable);
#endif

#ifdef INCLUDE_SPHERE
  SphereDriver_Register(driverTable);
#endif

#ifdef INCLUDE_CAMERA1394
  Camera1394_Register(driverTable);
#endif

#ifdef INCLUDE_IMAGESEQ
  ImageSeq_Register(driverTable);
#endif

#ifdef INCLUDE_CAMERACOMPRESS
  CameraCompress_Register(driverTable);
  CameraUncompress_Register(driverTable);
#endif

#ifdef INCLUDE_SERVICE_ADV_MDNS
  ServiceAdvMDNS_Register(driverTable);
#endif

#ifdef INCLUDE_FAKELOCALIZE
  FakeLocalize_Register(driverTable);
#endif

#ifdef INCLUDE_NOMAD
  Nomad_Register(driverTable);
  NomadPosition_Register(driverTable);
  NomadSonar_Register(driverTable);
  //NomadBumper_Register(driverTable);
  //NomadSpeech_Register(driverTable);
#endif

#ifdef INCLUDE_STAGECLIENT
  StgSimulation_Register(driverTable);
  StgLaser_Register(driverTable);
  //StgPosition_Register(driverTable);
  //StgSonar_Register(driverTable);
  //StgFiducial_Register(driverTable);
  //StgBlobfinder_Register(driverTable);

  //StgEnergy_Register(driverTable);
  //StgBlinkenlight_Register(driverTable);
#endif

#ifdef INCLUDE_INSIDEM300
  InsideM300_Register(driverTable);
#endif

#ifdef INCLUDE_SKYETEKM1
  SkyetekM1_Register(driverTable);
#endif

#ifdef INCLUDE_LASERTORANGER
  LaserToRanger_Register(driverTable);
#endif

#ifdef INCLUDE_SONARTORANGER
  SonarToRanger_Register(driverTable);
#endif

#ifdef INCLUDE_ROBOTEQ
  roboteq_Register(driverTable);
#endif

#ifdef INCLUDE_ROBOTINO
  robotino_Register(driverTable);
#endif

#ifdef INCLUDE_PBSLASER
  PBSDriver_Register(driverTable);
#endif

#ifdef INCLUDE_POSTGIS
  PostGIS_Register(driverTable);
#endif

#ifdef INCLUDE_VEC2MAP
  Vec2Map_Register(driverTable);
#endif

#ifdef INCLUDE_BUMPER2LASER
  Bumper2Laser_Register(driverTable);
#endif

#ifdef INCLUDE_LOCALBB
  LocalBB_Register(driverTable);
#endif

#ifdef INCLUDE_GBXSICKACFR
  GbxSickAcfr_Register(driverTable);
#endif

#ifdef INCLUDE_URG_NZ
  UrgDriver_Register(driverTable);
#endif

#ifdef INCLUDE_USARSIM
  UsBot_Register(driverTable);
  UsPosition_Register(driverTable);
  UsPosition3d_Register(driverTable);
  UsFakeLocalize_Register(driverTable); 
  UsLaser_Register(driverTable);
  UsLaser3d_Register(driverTable);
  UsSonar_Register(driverTable);
  RhIr_Register(driverTable);
  RhPyro_Register(driverTable);
  UsFiducial_Register(driverTable);
  UsVictFiducial_Register(driverTable);
  UsPtz_Register(driverTable);
#endif

}
