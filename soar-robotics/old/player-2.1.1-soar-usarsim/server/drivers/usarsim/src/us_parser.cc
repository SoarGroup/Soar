/*
 * Desc: Libraray for parsing Gamebots messages
 * Author: Jijun Wang
 * Date: 13 MAY 2004
// Modified: 
// 3 Mars 2005 	   Erik Winter 	added Ringhorne IR
// 11 Mars 2005    Erik Winter 	added RinghornePyro
// 14 Mars 2005    Erik Winter 	implemented rh_get_pyro, rh_get_pyro_geom
// 14 Mars 2005    Erik Winter 	added RHPyro config, implemented rh_get_pyro_config
// 14 Mars 2005    Erik Winter Started porting USARSim to Player1.6
// 15 Mars 2005    Erik Winter Continued porting, it compiles but gives segmentation faults 
// 18 Mars 2005    Erik Winter Changed ir_geom from player_ir_pose_req_t to player_ir_pose__t in the 1.6 version
// 07 June 2005    Bulit in support for INU
// 20 Nov 2006     Nils Kehrein Added all-at-once parsing for laser config/geom
// 22 Nov 2006     Florian Halbritter Added support for UsarSim RangeScanner3d (providing pointcloud3d data).
 */

#include "us_parser.h"

/**
 *
 */
// Define the indices of horizontal and vertical angles in the arrays
// for nicer handling later on.
#define HORIZ 1
#define VERT 0
int us_get_word(char* data, int pos, char* word)
{
  char *p, *pNext;
  int next;

  if (data == NULL || pos < 0 || pos >= strlen(data)) return -1;
  for (p = data + pos;*p == ' ';p++);
  if ((pNext = strchr(p,' '))==NULL) next = strlen(p);
  else next = pNext - p;
  if (word!=NULL)
  {
    // trying to avoid buffer overflows:
    strncpy(word,p,next);
    word[next] = '\0';
  }
  return next + pos;
}
/**
 * get the pointer to the message body
 */
char* us_get_body(char* data)
{
  if (data==NULL) return NULL;
  return data + us_get_word(data,0,NULL);
}
/**
 * get the availabe data type and message body pointer
 */
int us_get_type(char* data, char** ppBody)
{
  char head[16];
  head[0] = '\0';
  for(int i = 1; i < 16; i++)head[i] = 'A';
  int res = 0;
  if (data==NULL) return -1;
  *ppBody = data + us_get_word(data,0,head);
  if (!strcmp(head,"SEN"))
  {
    //PLAYER_MSG1(5,"us_parser:SEN %s\n",*ppBody);
    if (strstr(*ppBody,"{Type Sonar}")!=NULL) res |= US_DATA_SONAR;
    else if (strstr(*ppBody,"{Type RangeScanner}")!=NULL) res |= US_DATA_LASER;
    else if (strstr(*ppBody,"{Type RangeScanner3D}")!=NULL || strstr(*ppBody,"{Type 3DRangeScanner}")!=NULL) res |= US_DATA_LASER3D;
    else if (strstr(*ppBody,"{Type IR}")!=NULL) res |= RH_DATA_IR;
    else if (strstr(*ppBody,"{Type RhPyro}")!=NULL) res |= RH_DATA_PYRO;
    else if (strstr(*ppBody,"{Type INS}")!=NULL) res |= US_DATA_INS;
    else if (strstr(*ppBody,"{Type Odometry}")!=NULL) res |= US_DATA_ODOM | US_DATA_POSITION | US_DATA_POSITION3D;
    else if (strstr(*ppBody,"{Type Encoder}")!=NULL) res |= US_DATA_ODOM | US_DATA_ENCODER;
    else if (strstr(*ppBody,"{Type RFID}")!=NULL) res |= US_DATA_FIDUCIAL;
    else if (strstr(*ppBody,"{Type VictSensor}")!=NULL) res |= US_DATA_VICTIM_FIDUCIAL;
    else if (strstr(*ppBody,"{Type GroundTruth}")!=NULL) res |= US_DATA_GROUND_TRUTH;
  }
  else if (!strcmp(head,"GEO"))
  {
    PLAYER_MSG1(5,"us_parser:GEO %s\n",*ppBody);
    if (strstr(*ppBody,"{Type Sonar}")!=NULL) res |= US_GEOM_SONAR;
    else if (strstr(*ppBody,"{Type RangeScanner}")!=NULL) res |= US_GEOM_LASER;
    else if (strstr(*ppBody,"{Type RangeScanner3D}")!=NULL || strstr(*ppBody,"{Type 3DRangeScanner}")!=NULL) res |= US_GEOM_LASER3D;
    else if (strstr(*ppBody,"{Type IR}")!=NULL) res |= RH_GEOM_IR;
    else if (strstr(*ppBody,"{Type RhPyro}")!=NULL) res |= RH_GEOM_PYRO;
    else if (strstr(*ppBody,"{Type Camera}")!=NULL) res |= US_DATA_PTZ;
    else if (strstr(*ppBody,"{Type GroundVehicle}")!=NULL) res |= US_GEOM_ROBOT;  //for ptz zoom
  }
  
  else if (!strcmp(head,"MIS"))
  {
    //PLAYER_MSG1(5,"us_parser:MIS %s",*ppBody);
    if (strstr(*ppBody,"{Type PanTilt}")!=NULL) res |= US_GEOM_PTZ|US_DATA_PTZ; //for location geom and pan tilt data
  }
  else if (!strcmp(head,"CONF"))
  {
    PLAYER_MSG1(5,"us_parser:CONF %s\n",*ppBody);
    if (strstr(*ppBody,"{Type Sonar}")!=NULL) res |= US_CONF_SONAR;
    else if (strstr(*ppBody,"{Type RangeScanner}")!=NULL) res |= US_CONF_LASER;
    else if (strstr(*ppBody,"{Type RangeScanner3D}")!=NULL || strstr(*ppBody,"{Type 3DRangeScanner}")!=NULL) res |= US_CONF_LASER3D;
    else if (strstr(*ppBody,"{Type RhIr}")!=NULL) res |= RH_CONF_IR;
    else if (strstr(*ppBody,"{Type RhPyro}")!=NULL) res |= RH_CONF_PYRO;
    else if (strstr(*ppBody,"{Type Camera}")!=NULL) res |= US_CONF_CAMERA;  //for ptz zoom
    else if (strstr(*ppBody,"{Type GroundVehicle}")!=NULL) res |= US_CONF_ROBOT;  //for ptz zoom
  }
  else if (!strcmp(head,"STA"))
  {
    //PLAYER_MSG1(6,"us_parser:STA %s",*ppBody);
    res |= US_STATUS;
  }
  else res = -1;
  return res;
}
/**
 * get the first data segment that is between "{" and "}" from the position pos.
 */
int us_get_segment(char* data, int pos, char* segment)
{
  char *p1, *p2;
  char *tmp=""; 
  if (data==NULL || pos<0 || pos>=strlen(data)) return -1;
  if ((p1 = strchr(data+pos,'{'))==NULL) return -1;
  p1 += 1;
  if ((p2 = strchr(p1,'}'))==NULL) return -1;
  if (segment!=NULL)
  {
    // trying to avoid buffer overflows
    tmp[p2-p1+1]='\0';
    strncpy(tmp,p1,p2-p1);
  }
  segment=tmp;
  return p2-data+1;
}
/**
 * get the first data segment starts with name from the position pos.
 */
int us_get_segmentByName(char* data, int pos, char* name, char* segment)
{
  char *p1, *p2;
  char tmpStr[128];
  if (data == NULL ||
	 name == NULL ||
	 pos < 0 ||
	 pos >= strlen(data) ) return -1;
  tmpStr[0]='{';
  strncpy(tmpStr+1,name,sizeof(tmpStr));
  if ((p1 = strstr(data+pos,tmpStr)) == NULL) return -1;
  p1 += 1;
  if ((p2 = strchr(p1,'}')) == NULL) return -1;
  if (segment!=NULL)
  {
    //trying to avoid buffer overflows:
    segment[p2-p1+1]='\0';
    strncpy(segment,p1,p2-p1);
  }
  return p2-data+1;
}
/**
 * get the name value pair in a segment.
 */
int us_get_value(char* segment, char* name, char* value)
{
  char *p;
  if (segment==NULL || name==NULL) return -1;
  if ((p = strstr(segment,name))==NULL) return -1;

  return us_get_word(segment,p+strlen(name)-segment,value);
}

// get the first name value pair in a message.
int us_get_value2(char* data, char* name, char* value)
{
  char *p1, *p2;
  char tmpStr[128];
  int pos = 0;

  if (data == NULL || name == NULL) return -1;
  tmpStr[0]='{'; 
  //strncpy(tmpStr+1,name,sizeof(tmpStr));
  strcpy(tmpStr+1,name);
  if ((p1 = strstr(data + pos,tmpStr)) == NULL) return -1;
  p1 += strlen(tmpStr);
  if ((p2 = strchr(p1,' ')) == NULL) return -1;
  pos = us_get_word(data,p2-data+1,value);
  if(pos > 1 && pos < 4096) {
    if (data[pos-1] == '}')
	 {
	   pos-=1;
	   value[strlen(value)-1] = 0;
	 }
    return pos;
  }
  else {
    PLAYER_MSG2(1,"!!!!!!!!!!!!!!!!!!!!!!!!!!!error  pos %d data length %d\n",pos,strlen(data));
  } 
  return -1;
}
/**
 *
 */
int us_get_position(char* data, player_position2d_data_t *position)
{
  char tmp[128];
  char *p1, *p2;

  static float old_yaw = 0, oldxpos = 0, oldypos = 0;
  static struct timeval old_time = {0,0};
  struct timeval time;
  float xpos, ypos, yaw, xspeed, yspeed;
  double time_diff;

  gettimeofday(&time, NULL);
  if (position==NULL) return -1;
  if (us_get_value2(data, "Pose", tmp)==-1) return -1;
  // find first value (x) of pose triple
  if ((p2 = strchr(tmp,','))==NULL) return -1;
  *p2 = 0; // terminte string
  xpos = atof(tmp);
  position->pos.px = xpos;
  // find 2nd (y) value
  p1 = p2+1;
  if ((p2 = strchr(p1,','))==NULL) return -1;
  *p2 = 0;
  ypos = (-1.0) * atof(p1);
  position->pos.py = ypos;
  p1 = p2+1;
  // find last value (angle)
  yaw = atof(p1);
  position->pos.pa = (-1.0) * yaw;
  // calculate speeds
  time_diff = (double)(time.tv_sec - old_time.tv_sec) +
              (double)(time.tv_usec - old_time.tv_usec)/1000000;
  position->vel.pa = (yaw - old_yaw) / time_diff;
  xspeed = (xpos - oldxpos)/time_diff;
  yspeed = (ypos - oldypos)/time_diff;
  // TODO: kompletter bloedsinn?
  position->vel.px = sqrt(pow(xspeed,2)+pow(yspeed,2));
  position->vel.py = 0;
  position->stall = 0;

  oldxpos = xpos;
  oldypos = ypos;
  old_yaw = yaw;
  old_time = time;

  return 0;
}
/*
 *
 */
int us_get_enc(char* data, int & ticks_left, int & ticks_right)
{
  char tmp[128];
  char tmp2[128];
 
  if (us_get_value2(data, "Name ECLeft Tick", tmp)==-1) return -1;
  if (us_get_value2(data, "Name ECRight Tick", tmp2)==-1) return -1;
  tmp2[strlen(tmp2)-3]='\0';
  ticks_left = atoi(tmp);
  ticks_right = atoi(tmp2);
  return 0;
}
/**
 *
 */
int us_get_position3d(char* data, player_position3d_data_t *position3d)
{
  char tmp[128];
  char *p1, *p2;
  static struct timeval old_time = {0, 0};
  struct timeval time;
  float xpos, ypos, zpos;
  static float oldxpos = 0, oldypos = 0, oldzpos = 0;
  double time_diff;
  gettimeofday(&time, NULL);

  if (position3d==NULL) return -1;
  if (us_get_value2(data, "Pose", tmp)==-1) return -1;
  if ((p2 = strchr(tmp,','))==NULL) return -1;
  *p2 = 0;
  xpos = atof(tmp);
  position3d->pos.px = xpos;
  p1 = p2+1;
  if ((p2 = strchr(p1,','))==NULL) return -1;
  *p2 = 0;
  ypos =(-1.0) * atof(p1);
  position3d->pos.py = ypos;
  p1 = p2+1;
  zpos = 0;//atof(p1); since odometry does not return zpos
  position3d->pos.pz = zpos;

  time_diff = (double)(time.tv_sec - old_time.tv_sec) +
              (double)(time.tv_usec - old_time.tv_usec) / 1000000;  

  position3d->vel.px = (xpos - oldxpos) / time_diff;
  position3d->vel.py = (ypos - oldypos) / time_diff;
  position3d->vel.pz = 0;//(zpos - oldzpos)/time_diff;

  position3d->stall = 0;

  oldxpos = xpos;
  oldypos = ypos;
  oldzpos = zpos;

  old_time = time;

  return 0;
}
/**
 *
 */
int us_get_groundTruth(char* data, player_localize_data_t *location)
{
  //printf( "us_get_groundTruth: %s", data );

  char tmp[128];
  char *p1, *p2;
  struct timeval time;
  float xpos, ypos, yaw;

  if (us_get_value2(data, "Location", tmp)==-1) return -1; 
  if ((p2 = strchr(tmp,','))==NULL) return -1;
  *p2 = 0;
  xpos = atof(tmp);
  p1 = p2+1;
  if ((p2 = strchr(p1,','))==NULL) return -1;
  *p2 = 0;
  ypos = (-1.0) * atof(p1);
  if (us_get_value2(data, "Orientation", tmp)==-1) return -1; 
  if ((p2 = strchr(tmp,','))==NULL) return -1;
  p1 = p2+1;
  if ((p2 = strchr(p1,','))==NULL) return -1;
  *p2 = 0;
  p1 = p2+1;
 
  yaw = atof(p1);
  location->pending_count = 0;
  location->hypoths_count = 1;
  gettimeofday(&time, NULL);
  location->pending_time = time.tv_sec;
  location->hypoths[0].mean.px = xpos;
  location->hypoths[0].mean.py = ypos;
  location->hypoths[0].mean.pa = (-1.0) * yaw;
   // zero covariance and max weight
  location->hypoths[0].cov[0] = 0;
  location->hypoths[0].cov[1] = 0;
  location->hypoths[0].cov[2] = 0;
  location->hypoths[0].alpha = 1e6;
  return 0;
}
/**
 *
 */
int us_get_inu(char* data, player_position3d_data_t *inu){
  static struct timeval old_time;
  struct timeval time;
  static double oldyaw = 0;
  char value[64];
  float pitch, roll, yaw;
  int p;
  double time_diff;
  
  if (!inu || !data) return -1;
  gettimeofday(&time, NULL);
  p = us_get_value2(data, "Orientation", value);
  if (p == -1) return -1;
 
  p = sscanf(value, "%f,%f,%f", &roll, &pitch, &yaw);
  if(p < 3){
    return -1;
  }
  roll *= 1000;
  pitch *= -1000;
  yaw *= -1000;

  inu->pos.proll = roll;
  inu->pos.ppitch = pitch;
  inu->pos.pyaw = yaw;

  //Calculate yawspeed
  time_diff = (double)(time.tv_sec - old_time.tv_sec) +
              (double)(time.tv_usec - old_time.tv_usec)/1000000;
  // to make both yaw and oldyaw have the same sign
  if((yaw*oldyaw)<0){
    if(yaw>3)
	 oldyaw=oldyaw+2*M_PI*1000;
    else{
	 if(yaw<-3)
	   oldyaw=oldyaw-2*M_PI*1000;
    }
  }
  inu->vel.pyaw = (yaw-oldyaw)/time_diff;
  inu->vel.proll = 0;
  inu->vel.ppitch = 0;
  old_time = time;
  oldyaw = yaw;
  return 0;
}
/**
 *
 */
int us_get_sonar(char* data,char* name, player_sonar_data_t *sonar)
{
  char seg[128];
  char dummyName[128];
  char val[64];
  int pos = 0;
  short count;
  char *p;
  char tmp[7];
  p = data;
  count = 0;
  if (sonar == NULL) return -1;
  sprintf(dummyName,"Name %s",name);
  while ((pos = us_get_segmentByName(p,pos,dummyName,seg)) > 0)
  {
    if (us_get_value(seg,"Range",val) == -1) return -1;
    strncpy(tmp,val,7);
    sonar->ranges[count] = atof(tmp);
    count++;
    sonar->ranges_count = count;
  }
  return 0;
}
/**
 *
 */
int rh_get_ir(char* data, player_ir_data_t *ir)
{
  char seg[128];
  char val[64];
  int pos = 0;
  short count = 0;
  char *p = data;

  if (ir==NULL) return -1;
  while ((pos = us_get_segmentByName(p,0,"Name",seg))>0)
  {
    if (us_get_value(seg,"Range",val)==-1) return -1;
    ir->ranges[count] = atof(val);
    count++;
    p += pos;
  }
  ir->ranges_count = count;
  return 0;
}
/**
 * todo: get the uncertainty in the uposes from some conf
 */
int rh_get_pyro(char* data, player_fiducial_data_t *pyro)
{
 
  char positions[US_MAX_MSG_LEN], *p1, *p2;
  float posX,posY,posZ;

  if (pyro==NULL) return -1;
  if (us_get_value2(data, "Pose", positions)==-1) return -1;

  p1 = positions;
  p2 = strchr(p1,',');
  *p2 = 0;
  posX = atof(p1);
  p1 = p2+1;
  
  p2 = strchr(p1,',');  
  *p2 = 0;  
  posY = atof(p1);
  p1 = p2+1;
  
  p2 = strchr(p1,'}');
  *p2 = 0;
  posZ = atof(p1);
    
  pyro->fiducials_count = 1;//we only keep one (the latest) item in the list
  pyro->fiducials[0].id = -1; // unidentified
  pyro->fiducials[0].pose.px = posX; // x mm
  pyro->fiducials[0].pose.py = posY;// y mm
  pyro->fiducials[0].pose.pz = posZ;// z mm
  pyro->fiducials[0].pose.proll = 0; // roll mrad
  pyro->fiducials[0].pose.ppitch = atan(posX/posZ)*1000.0;// pitch mrad
  pyro->fiducials[0].pose.pyaw = atan(posY/posX)*1000.0;// yaw mrad
  
  //uncertainties
  pyro->fiducials[0].upose.px = 0.2 * pyro->fiducials[0].pose.px;
  pyro->fiducials[0].upose.py = 0.2 * pyro->fiducials[0].pose.py;
  pyro->fiducials[0].upose.pz = 0.2 * pyro->fiducials[0].pose.pz;
  pyro->fiducials[0].upose.proll = 0.2 * pyro->fiducials[0].pose.proll;
  pyro->fiducials[0].upose.ppitch = 0.2 * pyro->fiducials[0].pose.ppitch;
  pyro->fiducials[0].upose.pyaw = 0.2 * pyro->fiducials[0].pose.pyaw;
  
  return 0;     
}
/**
 * 
 */
int us_get_laser(char* data, char* name, player_laser_data_t *laser)
{
  char ranges[US_MAX_MSG_LEN], *p1, *p2;
  int count = 0;
  static int laser_id = 0;
  
  if(name == NULL) return -1;
  if(laser == NULL) return -1;
  
  us_get_value2(data, "Name", name);
  if(strlen(name) == 0) {
    return -1;
  }
  if (us_get_value2(data, "Range", ranges) == -1) {
    return -1;
  }
  p1 = ranges;
  while ((p2 = strchr(p1,','))>0)
  {
    *p2 = 0;
    laser->intensity[count] = 0; // right now we have no intensity info
    laser->ranges[count++] = atof(p1);
    p1 = p2+1;
  }
  laser->intensity[count] = (uint8_t)128;
  laser->intensity_count = 0;
  laser->ranges[count++] = atof(p1);
  laser->ranges_count = count;
  laser->id = laser_id;
  laser_id++;

  return 0;
}
/**
 *
 */
int us_get_ptz(char* data,char* name, player_ptz_data_t *ptz,player_ptz_geom_t *ptz_geom)
{
  /*
  char dummyName[128];
  char tmp[128];
  
  char seg[256];
  char val[128];
  int pos = 0;
  short count = 0;
  char *p = data, *p1, *p2;

  
  if (ptz==NULL) return -1;
  us_get_value(data,"Name",dummyName);
  if(strstr(dummyName,name) == NULL) return -1;
  sprintf(dummyName,"Name %s",name);
  
  if (us_get_value2(data, "Camera", tmp)==-1) return -1;
  if ((p2 = strchr(tmp,','))==NULL) return -1;
  *p2 = 0;
  ptz->tilt = atof(tmp);
  //fprintf(stderr,"T=%s|%d ",tmp, (int16_t)(atof(tmp)*US_UU_DEG));
  p1 = p2+1;
  if ((p2 = strchr(p1,','))==NULL) return -1;
  *p2 = 0;
  ptz->pan = (-1.0) * atof(p1);
  //fprintf(stderr,"P=%s|%d\n",p1, (int16_t)(atof(p1)*US_UU_DEG));
  if (us_get_value2(data, "CameraVel", tmp)==-1) return -1;
  if ((p2 = strchr(tmp,','))==NULL) return -1;
  *p2 = 0;
  ptz->tiltspeed = atof(tmp);
  p1 = p2+1;
  if ((p2 = strchr(p1,','))==NULL) return -1;
  *p2 = 0;
  ptz->panspeed = (-1.0) * atof(p1);


  while ((pos = us_get_segmentByName(p,pos,dummyName,seg))>0)
  {
  //printf("%s",p);
    if (ptz_geom==NULL) return -1;
    //if ((pos =us_get_segmentByName(p,0,dummyName,seg))==-1) return -1;
         
    if (us_get_value(seg,"Part CameraBase Location",val)==-1) return -1;
    if ((p2 = strchr(val,','))==NULL) return -1;
    *p2 = 0;
    ptz_geom->pos.px = atof(val);
    p1 = p2+1;
    if ((p2 = strchr(p1,','))==NULL) return -1;
    *p2 = 0;
    ptz_geom->pos.py = (-1.0) * atof(p1);
    p1 = p2+1;
    ptz_geom->pos.pz = (-1.0) * atof(p1);
    
    //fprintf(stderr,"sonar# %d\n",ntohs(sonar_geom->pose_count));
    //usarsim doesn't provide information about the size of sensors
    ptz_geom->size.sw = 0.1;
    ptz_geom->size.sl = 0.1;
    ptz_geom->size.sh = 0.1;
  }

  */
  ptz->tilt = 0.1;
  ptz->pan = 0.1;
  ptz->tiltspeed = 0.1;
  ptz->panspeed = 0.1;
  ptz_geom->pos.px = 0.0;
  ptz_geom->pos.py = 0.0;
  ptz_geom->pos.pz = 0.0;
  ptz_geom->size.sw = 0.1;
  ptz_geom->size.sl = 0.1;
  ptz_geom->size.sh = 0.1; 
  return 0;
}
/**
 *
 */ 
int us_get_sonar_geom(char* data,char* name, player_sonar_geom_t *sonar_geom,int index)
{
  char seg[256];
  char val[128];
  char dummyName[128];
  int pos = 0;
  short count = 0;
  char *p = data, *p1, *p2;
  float tmp;

  if (sonar_geom == NULL) return -1;
  sprintf(dummyName,"Name %s",name); 
  
  if (sonar_geom==NULL) return -1;
  while ((pos = us_get_segmentByName(p,pos,dummyName,seg)) > 0)
  {
    if (us_get_value(seg,"Location",val) == -1) return -1;
    if ((p2 = strchr(val,',')) == NULL) return -1;
    *p2 = 0;
    sonar_geom->poses[count].px = atof(val);
    p1 = p2+1;
    if ((p2 = strchr(p1,',')) == NULL) return -1;
    *p2 = 0;
    sonar_geom->poses[count].py = (-1.0) * atof(p1);

    if (us_get_value(seg,"Orientation",val) == -1) return -1;
    if ((p1 = strchr(val,',')) == 0) return -1;
    p2 = strchr(p1+1,',');
    p2+=1;
    tmp=atof(p2);
    sonar_geom->poses[count].pyaw = (-1.0) * tmp;
    count ++;
    sonar_geom->poses_count = count;
  }
  
  return 0;
}
/**
 *
 */
int us_get_camera_config(char* data,char* name, player_ptz_data_t *ptz)
{
  char val[128];
  char dummyName[128];

  us_get_value(data,"Name",dummyName);
  if(strstr(dummyName,name) == NULL) return -1;  
  //todo CameraFov not CameraDefFov
  if (us_get_value2(data,"CameraDefFov",val)==-1) return -1;
  ptz->zoom = atof(val);
  return 0;
}
/**
 *
 */
int rh_get_ir_geom(char* data, player_ir_pose_t *ir_geom)
{
  char seg[256];
  char val[128];
  int pos = 0;
  short count = 0;
  char *p = data, *p1, *p2;
  float tmp;
  
  if (ir_geom==NULL) return -1;
  while ((pos = us_get_segmentByName(p,0,"Name",seg))>0)
  {
    if (us_get_value(seg,"Location",val)==-1) return -1;
    if ((p2 = strchr(val,','))==NULL) return -1;
    *p2 = 0;
    ir_geom->poses[count].px = atof(val);
    p1 = p2+1;
    if ((p2 = strchr(p1,','))==NULL) return -1;
    *p2 = 0;
    ir_geom->poses[count].py = (-1.0) * atof(p1);

    if (us_get_value(seg,"Orientation",val)==-1) return -1;
    if ((p1 = strchr(val,','))==0) return -1;
    p2 = strchr(p1+1,',');
    p2+=1;
    tmp=atof(p2);
    ir_geom->poses[count].pyaw = (-1.0) * tmp;
    count ++;
    p += pos;
    ir_geom->poses_count = count;
  }

  return 0;
}
/**
 *
 */
int rh_get_pyro_geom(char* data, player_fiducial_geom_t *pyro_geom)
{  
  //The pyro's position in the robot. It should virtually be placed in the robot's center.
  pyro_geom->pose.px = 0;
  pyro_geom->pose.py = 0;
  pyro_geom->pose.pyaw = 0;
  
  pyro_geom->size.sw = 50;
  pyro_geom->size.sl = 50;
  return 0;
}
/*
 *
 */
int rh_get_pyro_config(char* data, player_fiducial_fov_t *pyro_conf)
{
  char val[128];

  if (pyro_conf==NULL) return -1;
  
  pyro_conf->min_range = 0;

  if (us_get_value2(data,"MaxRange",val)==-1) return -1;
  pyro_conf->max_range = atof(val);

  if (us_get_value2(data,"FovY",val)==-1) return -1;
  pyro_conf->view_angle =atof(val);
  return 0;
}

/*
 * Parses laser geometry for several lasers at once
 */
int us_get_laser_geom_all(char* data, map<char*, player_laser_geom_t*> *mGeom)
{
  char val[128], dummyName[128], seg[256];
  
  bzero( val, 128 );
  bzero( dummyName, 128 );
  bzero( seg, 256 );
  
  char *p = data, *name, *p1, *p2;
  int pos, ppos;
  float tmp;

  if(mGeom == NULL) return -1;

  while ((p = strstr(p, "{Name")) != NULL)
  {
    ppos = 0;
    name = new char[256];
    us_get_value2(p, "Name", name);
    if(strlen(name) == 0) {
      return -2;
    }
    
    (*mGeom)[name] = new player_laser_geom_t;
    //fprintf(stdout, "%s -- %s", name, p);
    //fflush(stdout);

    sprintf(dummyName, "Name %s", name);
    if ((ppos = us_get_segmentByName(p,ppos,dummyName,seg)) == -1) return -3;
    if (us_get_value(p,"Location",val) == -1) return -4;
    if ((p2 = strchr(val,',')) == NULL) return -5;
    *p2 = 0;
    (*mGeom)[name]->pose.px = atof(val);
    p1 = p2+1;
    if ((p2 = strchr(p1,',')) == NULL) return -6;
    *p2 = 0;
    (*mGeom)[name]->pose.py = -1.0 * atof(p1);
    
    if (us_get_value(seg,"Orientation",val)==-1) return -7;
    if ((p1 = strchr(val,',')) == 0) return -8;
    p2 = strchr(p1+1,',');
    p2+=1;
    tmp=atof(p2);
    (*mGeom)[name]->pose.pyaw = -1.0 * tmp;
    (*mGeom)[name]->size.sw = 0.1;
    (*mGeom)[name]->size.sl = 0.1;

    p += 5; // magic number: we've to jump over the current Name segment to the next one
  }
  return 1;
}

/*
 *
 */
int us_get_laser_geom(char* data,char* name, player_laser_geom_t *laser_geom)
{
  char dummyName[128];
  char seg[256];
  char val[128];
  int pos = 0;
  char *p = data, *p1, *p2;
  float tmp;

  if(name == NULL) return -1;
  if(laser_geom == NULL) return -1;
  
  us_get_value2(data, "Name", name);
  if(strlen(name) == 0) {
    return -1;
  }
  sprintf(dummyName,"Name %s",name);
  
  if ((pos = us_get_segmentByName(p,pos,dummyName,seg)) == -1) return -1;
  if (us_get_value(seg,"Location",val)==-1) return -1;
  if ((p2 = strchr(val,','))==NULL) return -1;
  *p2 = 0;
  laser_geom->pose.px = atof(val);
  p1 = p2+1;
  if ((p2 = strchr(p1,','))==NULL) return -1;
  *p2 = 0;
  laser_geom->pose.py = -1.0 * atof(p1);
  
  if (us_get_value(seg,"Orientation",val)==-1) return -1;
  if ((p1 = strchr(val,','))==0) return -1;
  p2 = strchr(p1+1,',');
  p2+=1;
  tmp=atof(p2);
  laser_geom->pose.pyaw = -1.0 * tmp;
  laser_geom->size.sw = 0.1;
  laser_geom->size.sl = 0.1;
  
  return 1;
}

/*
 * Parses laser config information for several lasers at once
 *
 * Only makes sense if command 'GETCONF {Type RangeScanner}' was sent
 * without any 'Name' field
 */
int us_get_laser_config_all(char* data, map<char*, player_laser_config_t*> *mConf)
{
  char val[128];
  char *p = data, *name;
  float fov;
  int pos;

  if (mConf == NULL) return -1;

  // search for Name fields
  while ((p = strstr(p, "{Name")) != NULL)
  {
    name = new char[256];
    us_get_value2(p, "Name", name);
    if(strlen(name) == 0) {
      return -2;
    }
    
    (*mConf)[name] = new player_laser_config_t;

    if (us_get_value2(p, "MaxRange", val)==-1) return -3;
    (*mConf)[name]->max_range = atof(val);
  
    if (us_get_value2(p, "Resolution", val)==-1) return -4;
    (*mConf)[name]->resolution = atof(val);
    
    if (us_get_value2(p, "Fov", val)==-1) return -5;
    fov = atof(val);
  
    (*mConf)[name]->min_angle = fov * -0.5;
    (*mConf)[name]->max_angle = fov * 0.5;
    (*mConf)[name]->range_res = 0.01;
    (*mConf)[name]->intensity = 0; // usarsim doesn't support intensity

    p += 5; // magic number: we've to jump over the current Name segment to the next one
  }

  return 0;
}

/*
 * Parses USARSIM laser config information for single laser only
 */
int us_get_laser_config(char* data, char* name, player_laser_config_t *laser_conf)
{
  char val[128];
  float fov;

  if (name == NULL) return -1;
  if (laser_conf == NULL) return -1;
  
  us_get_value2(data,"Name",name);
  if(strlen(name) == 0) {
    return -1;
  }
  if (us_get_value2(data,"MaxRange",val)==-1) return -1;
  laser_conf->max_range = atof(val);

  if (us_get_value2(data,"Resolution",val)==-1) return -1;
  laser_conf->resolution = atof(val);
  if (us_get_value2(data,"Fov",val)==-1) return -1;
  fov = atof(val);

  laser_conf->min_angle = fov * -0.5;
  laser_conf->max_angle = fov * 0.5;
  laser_conf->range_res = 0.01;  
  laser_conf->intensity = 0; // usarsim doesn't support intensity

  return 0;
}
/*
 *
 */
// int us_get_victim_fiducial(char* origdata, player_victim_fiducial_data_t *fid)
// {
// 	char tmp[128];
// 	char *p1, *p2;
// 	char *data;
// 	uint16_t count;
// 	int pos = 0;
// 	int32_t timestamp = 0;
// 	
// 	data=origdata;
// 	
// 	if (fid == NULL) return -1;
// 	memset(fid, 0, sizeof(player_victim_fiducial_data_t));
// 	count = fid->fiducials_count;
// 
// 	// time must be present
// 	if (us_get_value2(data, "Time", tmp)==-1) return -1;
// 	timestamp = atof(tmp)*1000;
// 	
// 	// if status is present
// 	if (us_get_value2(data, "Status", tmp) == 0)
// 	{
// 		// it must be this
// 		if (strcmp( tmp, "NoVictims" ) != 0) return -1;
// 		
// 		// it is, so no data
// 		return 0;
// 	}
// 	
// 	// we have data, name must be present
// 	if (us_get_value2(data, "Name", tmp) == -1) return -1;
// 	
// 	// and it must equal this
// 	if (strcmp( tmp, "VictSensor" ) != -1) return -1;
// 
// 	// loop through data
// 	while (1){
// 		count = fid->fiducials_count;
// 		
// 		if ( (pos = us_get_value2(data, "PartName", tmp)) == -1) return 0;
// 		fid->fiducials[count].timestamp = timestamp;
// 		data += pos;
// 		strncpy(fid->fiducials[count].status,tmp, PLAYER_FIDUCIAL_MAX_STATUS_LENGTH - 1);
// 		
// 		if ((pos = us_get_value2(data, "Location", tmp))==-1) return -1;
// 		data += pos;
// 		if ((p2 = strchr(tmp,','))==NULL) return -1;
// 		*p2 = 0;
// 		fid->fiducials[count].pose.px = atof(tmp);
// 		p1 = p2+1;
// 		if ((p2 = strchr(p1,','))==NULL) return -1;
// 		*p2 = 0;
// 		fid->fiducials[count].pose.py =  (-1.0) * atof(p1);
// 		p1 = p2+1;
// 		fid->fiducials[count].pose.pyaw =  (-1.0) * atof(p1);
// 		fid->fiducials_count = (count + 1)%PLAYER_FIDUCIAL_MAX_SAMPLES;
// 		
// 		//if ( (pos = us_get_value2(data, "ID", tmp))==-1) return 0;
// 		//strncpy(fid->fiducials[count].id,tmp, PLAYER_FIDUCIAL_MAX_ID_LENGTH - 1);
// 		///if ((pos = us_get_value2(data, "Status", tmp))==-1) return 0;
// 		//data += pos;
// 		//strncpy(fid->fiducials[count].status,tmp, PLAYER_FIDUCIAL_MAX_STATUS_LENGTH - 1);
// 	}
// 	return 0;
// }
/**
 *
 */
int us_get_victim_fiducial(char* origdata, player_fiducial_data_t *fid)
{
	//fflush(stdout);
	//printf( "us_get_victim_fiducial..." );
	char tmp[128];
	char *p1, *p2;
	char *data;
	uint16_t count;
	int pos = 0;
	data = origdata;
	if (fid==NULL) 
	{
		//printf("fid null\n");
		return -1;
	}
	//memset(fid, 0, sizeof(player_fiducial_data_t));
	fid->fiducials_count = 0;

	// time must be present
	if (us_get_value2(data, "Time", tmp)==-1) 
	{
		//printf("no time\n");
		return -1;
	}
	
	// if status is present
	if (us_get_value2(data, "Status", tmp) != -1)
	{
		// it must be this
		if (strcmp( tmp, "NoVictims" ) != 0)
		{
			//printf("no NoVictims: '%s'\n", tmp);
			return -1;
		}
		
		// it is, so no data
		//printf("OK: no data\n");
		return 0;
	}
	
	// we have data, name must be present
	if (us_get_value2(data, "Name", tmp) == -1)
	{
		//printf("no Name\n");
		return -1;
	}
	
	// and it must equal this
	if (strcmp( tmp, "VictSensor" ) != 0) 
	{
		//printf("Name not VictSensor\n");
		return -1;
	}

	//printf( "origdata: %s\n", origdata );
	while (1){
		//fflush( stdout );
		
		count = fid->fiducials_count;

		if ( (pos = us_get_value2(data, "PartName", tmp)) == -1) 
		{
			//printf("OK: count: %d\n", count );
			return 0;
		}
		// part name currently ignored
		data += pos;
		fid->fiducials[count].id = 0;
		
		if ((pos=us_get_value2(data, "Location", tmp)) == -1)
		{
			//printf("no Location\n" );
			return -1;
		}
		
		data += pos;
		if ((p2 = strchr(tmp,',')) == NULL)
		{
			//printf("x error\n" );
			return -1;
		}
		*p2 = 0;
		fid->fiducials[count].pose.px = atof(tmp);
		p1 = p2+1;
		if ((p2 = strchr(p1,',')) == NULL)
		{
			//printf("y error\n" );
			return -1;
		}
		*p2 = 0;
		fid->fiducials[count].pose.py = (-1.0) * atof(p1);
		p1 = p2+1;
		fid->fiducials[count].pose.pz = (-1.0) * atof(p1);
		fid->fiducials_count = (count + 1)%PLAYER_FIDUCIAL_MAX_SAMPLES;
	}
	//printf("strange: count: %d\n", count );
	return 0;
}
int us_get_fiducial(char* origdata, player_fiducial_data_t *fid)
{
	char tmp[128];
	char *p1, *p2;
	char *data;
	uint16_t count;
	int pos = 0;
	data = origdata;
	if (fid==NULL) return -1;
	//memset(fid, 0, sizeof(player_fiducial_data_t));
	fid->fiducials_count = 0;

	//printf( "origdata: %s\n", origdata );
	while (1){
		//fflush( stdout );
		
		count = fid->fiducials_count;
		if ( (pos=us_get_value2(data, "ID", tmp)) == -1) {
		  return 0;
		}
		data += pos;
		fid->fiducials[count].id = atoi(tmp);
		if ((pos=us_get_value2(data, "Location", tmp)) == -1)  return -1;
		data += pos;
		if ((p2 = strchr(tmp,',')) == NULL) return -1;
		*p2 = 0;
		fid->fiducials[count].pose.px = atof(tmp);
		p1 = p2+1;
		if ((p2 = strchr(p1,',')) == NULL) return -1;
		*p2 = 0;
		fid->fiducials[count].pose.py = (-1.0) * atof(p1);
		p1 = p2+1;
		fid->fiducials[count].pose.pz = (-1.0) * atof(p1);
		fid->fiducials_count = (count + 1)%PLAYER_FIDUCIAL_MAX_SAMPLES;
		
		//printf( "id: %d\n", fid->fiducials[count].id );
	}
	return 0;
}

/**
 *
 */
int us_get_robot_config(char* data, char *steeringType, double &robotMass, double &maxSpeed,
				    double &maxTorque, double &maxFrontSteer, double &maxRearSteer)
{
  char tmp[128];
  char *p1, *p2;
  if(steeringType == NULL) return -1;
  if (us_get_value2(data,"SteeringType",steeringType)==-1) return -1;
  if (us_get_value2(data,"Mass",tmp)==-1) return -1;
  robotMass = atof(tmp);
  if (us_get_value2(data,"MaxSpeed",tmp)==-1) return -1;
  maxSpeed = atof(tmp);
  if (us_get_value2(data,"MaxTorque",tmp)==-1) return -1;
  maxTorque = atof(tmp);
  if (us_get_value2(data,"MaxFrontSteer",tmp)==-1) return -1;
  maxFrontSteer = atof(tmp);
  //@todo there is a problem sigseg (?last value in string?)
  /*
  if (us_get_value2(data,"MaxRearSteer",tmp)==-1) return -1;
  //strtok(tmp, "}");
  maxRearSteer = atof(tmp);
  */
  return 0; 
}
/**
 *
 */
int us_get_robot_geom(char* data, player_bbox3d_t *dimensions, double* COG, double &wheelRadius,
				    double &maxWheelSeparation, double &wheelBase)
{
  char tmp[128];
  char *p1, *p2;
  if(dimensions == NULL) return -1;
  if (us_get_value2(data, "Dimensions", tmp)==-1) return -1;
  if ((p2 = strchr(tmp,','))==NULL) return -1;
  *p2 = 0;
  dimensions->sl = atof(tmp);
  p1 = p2+1;
  if ((p2 = strchr(p1,','))==NULL) return -1;
  *p2 = 0;
  dimensions->sw =  atof(p1);
  p1 = p2+1;
  dimensions->sh =   atof(p1);
  if (us_get_value2(data, "COG", tmp)==-1) return -1;
  if ((p2 = strchr(tmp,','))==NULL) return -1;
  *p2 = 0;
  COG[0] = atof(tmp);
  p1 = p2+1;
  if ((p2 = strchr(p1,','))==NULL) return -1;
  *p2 = 0;
  COG[1] =  atof(p1);
  p1 = p2+1;
  COG[2] =  atof(p1);
  if (us_get_value2(data,"WheelRadius",tmp)==-1) return -1;
  wheelRadius = atof(tmp);
  if (us_get_value2(data,"WheelSeparation",tmp)==-1) return -1;
  maxWheelSeparation = atof(tmp);
  /*
  if (us_get_value2(data,"WheelBase",tmp)==-1) return -1;
  PLAYER_MSG1(3,"base tmp %s",tmp);
  wheelBase = atof(tmp); 
  */
  return 0; 
}

//////////////// UsLaser3d changes /////////////////////

/*
 * Transforms the given sensory input into a list of 3D point data. 
 * The signal is of the form: {Name sensorName} {Resolution vertRes, horizRes} {FOV vertFOV, horizFOV} {Range r1, r2, ...}
 * Assumes that the scans are traversed row-wise, i.e. that for one fixed vertical angle first all possible horizontal 
 * angles are scanned before the procedure is repeated for the next vertical angle.
 */
int us_get_laser3d(char* data, char* name, player_pointcloud3d_data_t *laser3d)
{  
  static int filecount = 0;
  
  char tmp[US_MAX_MSG_LEN], *p1, *p2;
  float ranges[US_MAX_MSG_LEN], delta_angles[2], angles[2];
  int count = 0, index = 0;  
  char* filename = new char[64];;

  float horiz_angle, vert_angle;
  float x,y,z;
  
  // Check whether ther is nothing to do:
  if(name == NULL || laser3d == NULL) {
    return -1;
  }
  us_get_value2(data, "Name", name);
  if(strlen(name) == 0) {
    return -1;
  }
  
  // ---- Parse delta values ----
  // from {Resolution x,y}
  // where vertical resolution = x
  //       horizontal resolution = y
    
  if (us_get_value2(data, "Resolution", tmp) == -1) {
    return -1;
  }
  
  count = 0;
  p1 = tmp;  
  while ((p2 = strchr(p1,','))>0)
  {
    *p2 = 0;
    delta_angles[count++] = atof(p1);
    p1 = p2+1;
  }  
  delta_angles[count++] = atof(p1);   
  
  if(count > 2) {
    fprintf(stderr, "Something went wrong. Too many 'Resolution' values (%i)", count);
    return -1;
  }
  
  // ---- Parse angle values ----
  // from {FOV x,y}
  // where vertical max angle = -(x/2)
  //       horizontal max angle = -(y/2)
 
  
  if (us_get_value2(data, "FOV", tmp) == -1) {
    return -1;
  }
  
  count = 0;
  p1 = tmp;  
  while ((p2 = strchr(p1,','))>0)
  {
    *p2 = 0;
    angles[count++] = atof(p1);
    p1 = p2+1;
  }  
  angles[count++] = atof(p1); 
  
  if(count > 2) {
    fprintf(stderr, "Something went wrong. Too many 'FOV' values (%i)", count);
    return -1;
  } 

  // ---- Parse range values ----
  // from {Range r1, r2, r3, ...}
  
  if (us_get_value2(data, "Range", tmp) == -1) {
    return -1;
  }
  
  count = 0;
  p1 = tmp;  
  while ((p2 = strchr(p1,','))>0)
  {
    *p2 = 0;
    ranges[count++] = atof(p1);
    p1 = p2+1;
  }  
  ranges[count++] = atof(p1); 
  
  // ---- Convert into point cloud ----
   
  laser3d->points_count = count;
  
  // Save scans in text files
  std::ofstream out;  
  sprintf(filename, "scans/scan%03i.3d", (filecount++));
  out.open (filename, ofstream::out | ofstream::trunc); 

  // This assumes that the range data has been scanned row-wise!
  for(vert_angle = 0; vert_angle < angles[VERT]; vert_angle += delta_angles[VERT]) {
    for(horiz_angle = 0; horiz_angle < angles[HORIZ]; horiz_angle += delta_angles[HORIZ]) { 
  
      // Calculate (x,y,z)-coordinates
      x = ranges[index] * sin(vert_angle) * sin(horiz_angle);
      y = ranges[index] * sin(vert_angle) * cos(horiz_angle);    
      z = ranges[index] * cos(vert_angle);
      
      // Save current point
      out<<x<<" "<<y<<" "<<z<<std::endl;
      
      // Store point in pointcloud
      laser3d->points[index].point.px = x;
      laser3d->points[index].point.py = y;
      laser3d->points[index].point.pz = z;
      
      index++;
    }
  }
  
  out.close();
  
  printf("Parsed Laser3d data.\n");
  fflush(stdout);

  return 0;
}

/*
 * Parses the configuration of a 3d laser scanner from the given input.
 * The result will contain information about the FOVs and resolutions (both, horizontal and vertical).
 */
int us_get_laser3d_config(char* data, char* name, player_laser3d_config_t *laser3d_conf)
{
  char tmp[US_MAX_MSG_LEN], *p1, *p2;
  float delta_angles[2], angles[2];
  int count = 0;

  // Check whether there is nothing to do:
  if (name == NULL || laser3d_conf) {
    return -1;
  }  
  us_get_value2(data,"Name",name);
  if(strlen(name) == 0) {
    return -1;
  }

  // ---- Parse resolution values ----
					 
  if (us_get_value2(data, "Resolution", tmp) == -1) {
    return -1;
  }
  
  count = 0;
  p1 = tmp;  
  while ((p2 = strchr(p1,','))>0)
  {
    *p2 = 0;
    delta_angles[count++] = atof(p1);
    p1 = p2+1;
  }  
  delta_angles[count++] = atof(p1); 
  
  if(count > 2) {
    fprintf(stderr, "Something went wrong. Too many 'Resolution' values (%i)", count);
    return -1;
  }

  // ---- Parse FOV values ----
  
  if (us_get_value2(data, "FOV", tmp) == -1) {
    return -1;
  }
  
  count = 0;
  p1 = tmp;  
  while ((p2 = strchr(p1,','))>0)
  {
    *p2 = 0;
    angles[count++] = atof(p1);
    p1 = p2+1;
  }  
  angles[count++] = atof(p1); 
  
  if(count > 2) {
    fprintf(stderr, "Something went wrong. Too many 'FOV' values (%i)", count);
    return -1;
  }
 
  // Save configuration.
  laser3d_conf->min_angles[0] = angles[0];
  laser3d_conf->min_angles[1] = angles[1];
  laser3d_conf->resolutions[0] = delta_angles[0];
  laser3d_conf->resolutions[1] = delta_angles[1];

  return 0;
}

/*
* Parses laser3d config for several laser3ds at once.
*/
int us_get_laser3d_config_all(char* data, map<char*, player_laser3d_config_t*> *mConf)
{
  char val[128];
  char *p = data, *name;

  if (mConf == NULL) return -1;

  // search for Name fields
  while ((p = strstr(p, "{Name")) != NULL)
  {
    name = new char[256];
    us_get_value2(p, "Name", name);
    if(strlen(name) == 0) {
      return -2;
    }
    
    (*mConf)[name] = new player_laser3d_config_t;

    us_get_laser3d_config(p, name, (*mConf)[name]);

    p += 5; // magic number: we've to jump over the current Name segment to the next one
  }

  return 0;
}

/*
 * Parses laser3d geometry for several laser3ds at once.
 */
int us_get_laser3d_geom_all(char* data, map<char*, player_laser3d_geom_t*> *mGeom)
{
  char val[128], dummyName[128], seg[256];
  char *p = data, *name, *p1, *p2;
  int pos, ppos;
  float tmp;

  if(mGeom == NULL) return -1;

  while ((p = strstr(p, "{Name")) != NULL)
  {
    ppos = 0;
    name = new char[256];
    us_get_value2(p, "Name", name);
    if(strlen(name) == 0) {
      return -2;
    }
    
    (*mGeom)[name] = new player_laser3d_geom_t;
    //fprintf(stdout, "%s -- %s", name, p);
    //fflush(stdout);

    sprintf(dummyName, "Name %s", name);
    if ((ppos = us_get_segmentByName(p,ppos,dummyName,seg)) == -1) return -3;
    if (us_get_value(p,"Location",val) == -1) return -4;
    if ((p2 = strchr(val,',')) == NULL) return -5;
    *p2 = 0;
    (*mGeom)[name]->pose.px = atof(val);
    p1 = p2+1;
    if ((p2 = strchr(p1,',')) == NULL) return -6;
    *p2 = 0;
    (*mGeom)[name]->pose.py = -1.0 * atof(p1);
    
    if (us_get_value(seg,"Orientation",val)==-1) return -7;
    if ((p1 = strchr(val,',')) == 0) return -8;
    p2 = strchr(p1+1,',');
    p2+=1;
    tmp=atof(p2);
    (*mGeom)[name]->pose.pa = -1.0 * tmp;
    (*mGeom)[name]->size.sw = 0.1;
    (*mGeom)[name]->size.sl = 0.1;

    p += 5; // magic number: we've to jump over the current Name segment to the next one
  }
  return 1;
}

/*
 * Parses geometry model for a single laser3d.
 */
int us_get_laser3d_geom(char* data,char* name, player_laser3d_geom_t *laser3d_geom)
{
  char dummyName[128];
  char seg[256];
  char val[128];
  int pos = 0;
  char *p = data, *p1, *p2;
  float tmp;

  if(name == NULL) return -1;
  if(laser3d_geom == NULL) return -1;
  
  us_get_value2(data, "Name", name);
  if(strlen(name) == 0) {
    return -1;
  }
  sprintf(dummyName,"Name %s",name);
  
  if ((pos = us_get_segmentByName(p,pos,dummyName,seg)) == -1) return -1;
  if (us_get_value(seg,"Location",val)==-1) return -1;
  if ((p2 = strchr(val,','))==NULL) return -1;
  *p2 = 0;
  laser3d_geom->pose.px = atof(val);
  p1 = p2+1;
  if ((p2 = strchr(p1,','))==NULL) return -1;
  *p2 = 0;
  laser3d_geom->pose.py = -1.0 * atof(p1);
  
  if (us_get_value(seg,"Orientation",val)==-1) return -1;
  if ((p1 = strchr(val,','))==0) return -1;
  p2 = strchr(p1+1,',');
  p2+=1;
  tmp=atof(p2);
  laser3d_geom->pose.pa = -1.0 * tmp;
  laser3d_geom->size.sw = 0.1;
  laser3d_geom->size.sl = 0.1;
  
  return 1;
}

