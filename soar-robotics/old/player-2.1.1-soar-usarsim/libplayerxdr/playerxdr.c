
#include <libplayerxdr/playerxdr.h>
#include <string.h>

#include <stdlib.h>

int xdr_player_devaddr_t (XDR* xdrs, player_devaddr_t * msg)
{   if(xdr_u_int(xdrs,&msg->host) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->robot) != 1)
    return(0);
  if(xdr_u_short(xdrs,&msg->interf) != 1)
    return(0);
  if(xdr_u_short(xdrs,&msg->index) != 1)
    return(0);
  return(1);
}
int 
player_devaddr_pack(void* buf, size_t buflen, player_devaddr_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_devaddr_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_devaddr_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_devaddr_t_copy(player_devaddr_t *dest, const player_devaddr_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_devaddr_t));
  return sizeof(player_devaddr_t);
} 
void player_devaddr_t_cleanup(const player_devaddr_t *msg)
{
} 
player_devaddr_t * player_devaddr_t_clone(const player_devaddr_t *msg)
{      
  player_devaddr_t * clone = malloc(sizeof(player_devaddr_t));
  if (clone)
    player_devaddr_t_copy(clone,msg);
  return clone;
}
void player_devaddr_t_free(player_devaddr_t *msg)
{      
  player_devaddr_t_cleanup(msg);
  free(msg);
}
unsigned int player_devaddr_t_sizeof(player_devaddr_t *msg)
{
  return sizeof(player_devaddr_t);
} 

int xdr_player_msghdr_t (XDR* xdrs, player_msghdr_t * msg)
{   if(xdr_player_devaddr_t(xdrs,&msg->addr) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->type) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->subtype) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->timestamp) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->seq) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->size) != 1)
    return(0);
  return(1);
}
int 
player_msghdr_pack(void* buf, size_t buflen, player_msghdr_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_msghdr_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_msghdr_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_msghdr_t_copy(player_msghdr_t *dest, const player_msghdr_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_msghdr_t));
  return sizeof(player_msghdr_t);
} 
void player_msghdr_t_cleanup(const player_msghdr_t *msg)
{
} 
player_msghdr_t * player_msghdr_t_clone(const player_msghdr_t *msg)
{      
  player_msghdr_t * clone = malloc(sizeof(player_msghdr_t));
  if (clone)
    player_msghdr_t_copy(clone,msg);
  return clone;
}
void player_msghdr_t_free(player_msghdr_t *msg)
{      
  player_msghdr_t_cleanup(msg);
  free(msg);
}
unsigned int player_msghdr_t_sizeof(player_msghdr_t *msg)
{
  return sizeof(player_msghdr_t);
} 

int xdr_player_null_t (XDR* xdrs, player_null_t * msg)
{   return(1);
}
int 
player_null_pack(void* buf, size_t buflen, player_null_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_null_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_null_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_null_t_copy(player_null_t *dest, const player_null_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_null_t));
  return sizeof(player_null_t);
} 
void player_null_t_cleanup(const player_null_t *msg)
{
} 
player_null_t * player_null_t_clone(const player_null_t *msg)
{      
  player_null_t * clone = malloc(sizeof(player_null_t));
  if (clone)
    player_null_t_copy(clone,msg);
  return clone;
}
void player_null_t_free(player_null_t *msg)
{      
  player_null_t_cleanup(msg);
  free(msg);
}
unsigned int player_null_t_sizeof(player_null_t *msg)
{
  return sizeof(player_null_t);
} 

int xdr_player_point_2d_t (XDR* xdrs, player_point_2d_t * msg)
{   if(xdr_double(xdrs,&msg->px) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->py) != 1)
    return(0);
  return(1);
}
int 
player_point_2d_pack(void* buf, size_t buflen, player_point_2d_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_point_2d_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_point_2d_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_point_2d_t_copy(player_point_2d_t *dest, const player_point_2d_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_point_2d_t));
  return sizeof(player_point_2d_t);
} 
void player_point_2d_t_cleanup(const player_point_2d_t *msg)
{
} 
player_point_2d_t * player_point_2d_t_clone(const player_point_2d_t *msg)
{      
  player_point_2d_t * clone = malloc(sizeof(player_point_2d_t));
  if (clone)
    player_point_2d_t_copy(clone,msg);
  return clone;
}
void player_point_2d_t_free(player_point_2d_t *msg)
{      
  player_point_2d_t_cleanup(msg);
  free(msg);
}
unsigned int player_point_2d_t_sizeof(player_point_2d_t *msg)
{
  return sizeof(player_point_2d_t);
} 

int xdr_player_point_3d_t (XDR* xdrs, player_point_3d_t * msg)
{   if(xdr_double(xdrs,&msg->px) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->py) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->pz) != 1)
    return(0);
  return(1);
}
int 
player_point_3d_pack(void* buf, size_t buflen, player_point_3d_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_point_3d_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_point_3d_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_point_3d_t_copy(player_point_3d_t *dest, const player_point_3d_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_point_3d_t));
  return sizeof(player_point_3d_t);
} 
void player_point_3d_t_cleanup(const player_point_3d_t *msg)
{
} 
player_point_3d_t * player_point_3d_t_clone(const player_point_3d_t *msg)
{      
  player_point_3d_t * clone = malloc(sizeof(player_point_3d_t));
  if (clone)
    player_point_3d_t_copy(clone,msg);
  return clone;
}
void player_point_3d_t_free(player_point_3d_t *msg)
{      
  player_point_3d_t_cleanup(msg);
  free(msg);
}
unsigned int player_point_3d_t_sizeof(player_point_3d_t *msg)
{
  return sizeof(player_point_3d_t);
} 

int xdr_player_orientation_3d_t (XDR* xdrs, player_orientation_3d_t * msg)
{   if(xdr_double(xdrs,&msg->proll) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->ppitch) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->pyaw) != 1)
    return(0);
  return(1);
}
int 
player_orientation_3d_pack(void* buf, size_t buflen, player_orientation_3d_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_orientation_3d_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_orientation_3d_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_orientation_3d_t_copy(player_orientation_3d_t *dest, const player_orientation_3d_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_orientation_3d_t));
  return sizeof(player_orientation_3d_t);
} 
void player_orientation_3d_t_cleanup(const player_orientation_3d_t *msg)
{
} 
player_orientation_3d_t * player_orientation_3d_t_clone(const player_orientation_3d_t *msg)
{      
  player_orientation_3d_t * clone = malloc(sizeof(player_orientation_3d_t));
  if (clone)
    player_orientation_3d_t_copy(clone,msg);
  return clone;
}
void player_orientation_3d_t_free(player_orientation_3d_t *msg)
{      
  player_orientation_3d_t_cleanup(msg);
  free(msg);
}
unsigned int player_orientation_3d_t_sizeof(player_orientation_3d_t *msg)
{
  return sizeof(player_orientation_3d_t);
} 

int xdr_player_pose2d_t (XDR* xdrs, player_pose2d_t * msg)
{   if(xdr_double(xdrs,&msg->px) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->py) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->pa) != 1)
    return(0);
  return(1);
}
int 
player_pose2d_pack(void* buf, size_t buflen, player_pose2d_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_pose2d_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_pose2d_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_pose2d_t_copy(player_pose2d_t *dest, const player_pose2d_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_pose2d_t));
  return sizeof(player_pose2d_t);
} 
void player_pose2d_t_cleanup(const player_pose2d_t *msg)
{
} 
player_pose2d_t * player_pose2d_t_clone(const player_pose2d_t *msg)
{      
  player_pose2d_t * clone = malloc(sizeof(player_pose2d_t));
  if (clone)
    player_pose2d_t_copy(clone,msg);
  return clone;
}
void player_pose2d_t_free(player_pose2d_t *msg)
{      
  player_pose2d_t_cleanup(msg);
  free(msg);
}
unsigned int player_pose2d_t_sizeof(player_pose2d_t *msg)
{
  return sizeof(player_pose2d_t);
} 

int xdr_player_pose3d_t (XDR* xdrs, player_pose3d_t * msg)
{   if(xdr_double(xdrs,&msg->px) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->py) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->pz) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->proll) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->ppitch) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->pyaw) != 1)
    return(0);
  return(1);
}
int 
player_pose3d_pack(void* buf, size_t buflen, player_pose3d_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_pose3d_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_pose3d_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_pose3d_t_copy(player_pose3d_t *dest, const player_pose3d_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_pose3d_t));
  return sizeof(player_pose3d_t);
} 
void player_pose3d_t_cleanup(const player_pose3d_t *msg)
{
} 
player_pose3d_t * player_pose3d_t_clone(const player_pose3d_t *msg)
{      
  player_pose3d_t * clone = malloc(sizeof(player_pose3d_t));
  if (clone)
    player_pose3d_t_copy(clone,msg);
  return clone;
}
void player_pose3d_t_free(player_pose3d_t *msg)
{      
  player_pose3d_t_cleanup(msg);
  free(msg);
}
unsigned int player_pose3d_t_sizeof(player_pose3d_t *msg)
{
  return sizeof(player_pose3d_t);
} 

int xdr_player_bbox2d_t (XDR* xdrs, player_bbox2d_t * msg)
{   if(xdr_double(xdrs,&msg->sw) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->sl) != 1)
    return(0);
  return(1);
}
int 
player_bbox2d_pack(void* buf, size_t buflen, player_bbox2d_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_bbox2d_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_bbox2d_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_bbox2d_t_copy(player_bbox2d_t *dest, const player_bbox2d_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_bbox2d_t));
  return sizeof(player_bbox2d_t);
} 
void player_bbox2d_t_cleanup(const player_bbox2d_t *msg)
{
} 
player_bbox2d_t * player_bbox2d_t_clone(const player_bbox2d_t *msg)
{      
  player_bbox2d_t * clone = malloc(sizeof(player_bbox2d_t));
  if (clone)
    player_bbox2d_t_copy(clone,msg);
  return clone;
}
void player_bbox2d_t_free(player_bbox2d_t *msg)
{      
  player_bbox2d_t_cleanup(msg);
  free(msg);
}
unsigned int player_bbox2d_t_sizeof(player_bbox2d_t *msg)
{
  return sizeof(player_bbox2d_t);
} 

int xdr_player_bbox3d_t (XDR* xdrs, player_bbox3d_t * msg)
{   if(xdr_double(xdrs,&msg->sw) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->sl) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->sh) != 1)
    return(0);
  return(1);
}
int 
player_bbox3d_pack(void* buf, size_t buflen, player_bbox3d_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_bbox3d_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_bbox3d_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_bbox3d_t_copy(player_bbox3d_t *dest, const player_bbox3d_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_bbox3d_t));
  return sizeof(player_bbox3d_t);
} 
void player_bbox3d_t_cleanup(const player_bbox3d_t *msg)
{
} 
player_bbox3d_t * player_bbox3d_t_clone(const player_bbox3d_t *msg)
{      
  player_bbox3d_t * clone = malloc(sizeof(player_bbox3d_t));
  if (clone)
    player_bbox3d_t_copy(clone,msg);
  return clone;
}
void player_bbox3d_t_free(player_bbox3d_t *msg)
{      
  player_bbox3d_t_cleanup(msg);
  free(msg);
}
unsigned int player_bbox3d_t_sizeof(player_bbox3d_t *msg)
{
  return sizeof(player_bbox3d_t);
} 

int xdr_player_blackboard_entry_t (XDR* xdrs, player_blackboard_entry_t * msg)
{   if(xdr_u_int(xdrs,&msg->key_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->key = malloc(msg->key_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* key_p = msg->key;
    if(xdr_bytes(xdrs, (char**)&key_p, &msg->key_count, msg->key_count) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->group_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->group = malloc(msg->group_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* group_p = msg->group;
    if(xdr_bytes(xdrs, (char**)&group_p, &msg->group_count, msg->group_count) != 1)
      return(0);
  }
  if(xdr_u_short(xdrs,&msg->type) != 1)
    return(0);
  if(xdr_u_short(xdrs,&msg->subtype) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->data_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->data = malloc(msg->data_count*sizeof(uint8_t))) == NULL)
      return(0);
  }
  {
    uint8_t* data_p = msg->data;
    if(xdr_bytes(xdrs, (char**)&data_p, &msg->data_count, msg->data_count) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->timestamp_sec) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->timestamp_usec) != 1)
    return(0);
  return(1);
}
int 
player_blackboard_entry_pack(void* buf, size_t buflen, player_blackboard_entry_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_blackboard_entry_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_blackboard_entry_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_blackboard_entry_t_copy(player_blackboard_entry_t *dest, const player_blackboard_entry_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->key_count,&src->key_count,sizeof(uint32_t)*1); 
  if(src->key != NULL && src->key_count > 0)
  {
    if((dest->key = malloc(src->key_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->key = NULL;
  size += sizeof(char)*src->key_count;
  memcpy(dest->key,src->key,sizeof(char)*src->key_count); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->group_count,&src->group_count,sizeof(uint32_t)*1); 
  if(src->group != NULL && src->group_count > 0)
  {
    if((dest->group = malloc(src->group_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->group = NULL;
  size += sizeof(char)*src->group_count;
  memcpy(dest->group,src->group,sizeof(char)*src->group_count); 
  size += sizeof(uint16_t)*1;
  memcpy(&dest->type,&src->type,sizeof(uint16_t)*1); 
  size += sizeof(uint16_t)*1;
  memcpy(&dest->subtype,&src->subtype,sizeof(uint16_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->data_count,&src->data_count,sizeof(uint32_t)*1); 
  if(src->data != NULL && src->data_count > 0)
  {
    if((dest->data = malloc(src->data_count*sizeof(uint8_t))) == NULL)
      return(0);
  }
  else
    dest->data = NULL;
  size += sizeof(uint8_t)*src->data_count;
  memcpy(dest->data,src->data,sizeof(uint8_t)*src->data_count); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->timestamp_sec,&src->timestamp_sec,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->timestamp_usec,&src->timestamp_usec,sizeof(uint32_t)*1); 
  return(size);
}
void player_blackboard_entry_t_cleanup(const player_blackboard_entry_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->key); 
  free(msg->group); 
  free(msg->data); 
}
player_blackboard_entry_t * player_blackboard_entry_t_clone(const player_blackboard_entry_t *msg)
{      
  player_blackboard_entry_t * clone = malloc(sizeof(player_blackboard_entry_t));
  if (clone)
    player_blackboard_entry_t_copy(clone,msg);
  return clone;
}
void player_blackboard_entry_t_free(player_blackboard_entry_t *msg)
{      
  player_blackboard_entry_t_cleanup(msg);
  free(msg);
}
unsigned int player_blackboard_entry_t_sizeof(player_blackboard_entry_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->key_count; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->group_count; 
  size += sizeof(uint16_t)*1; 
  size += sizeof(uint16_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint8_t)*msg->data_count; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  return(size);
}

int xdr_player_segment_t (XDR* xdrs, player_segment_t * msg)
{   if(xdr_double(xdrs,&msg->x0) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->y0) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->x1) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->y1) != 1)
    return(0);
  return(1);
}
int 
player_segment_pack(void* buf, size_t buflen, player_segment_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_segment_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_segment_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_segment_t_copy(player_segment_t *dest, const player_segment_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_segment_t));
  return sizeof(player_segment_t);
} 
void player_segment_t_cleanup(const player_segment_t *msg)
{
} 
player_segment_t * player_segment_t_clone(const player_segment_t *msg)
{      
  player_segment_t * clone = malloc(sizeof(player_segment_t));
  if (clone)
    player_segment_t_copy(clone,msg);
  return clone;
}
void player_segment_t_free(player_segment_t *msg)
{      
  player_segment_t_cleanup(msg);
  free(msg);
}
unsigned int player_segment_t_sizeof(player_segment_t *msg)
{
  return sizeof(player_segment_t);
} 

int xdr_player_extent2d_t (XDR* xdrs, player_extent2d_t * msg)
{   if(xdr_double(xdrs,&msg->x0) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->y0) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->x1) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->y1) != 1)
    return(0);
  return(1);
}
int 
player_extent2d_pack(void* buf, size_t buflen, player_extent2d_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_extent2d_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_extent2d_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_extent2d_t_copy(player_extent2d_t *dest, const player_extent2d_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_extent2d_t));
  return sizeof(player_extent2d_t);
} 
void player_extent2d_t_cleanup(const player_extent2d_t *msg)
{
} 
player_extent2d_t * player_extent2d_t_clone(const player_extent2d_t *msg)
{      
  player_extent2d_t * clone = malloc(sizeof(player_extent2d_t));
  if (clone)
    player_extent2d_t_copy(clone,msg);
  return clone;
}
void player_extent2d_t_free(player_extent2d_t *msg)
{      
  player_extent2d_t_cleanup(msg);
  free(msg);
}
unsigned int player_extent2d_t_sizeof(player_extent2d_t *msg)
{
  return sizeof(player_extent2d_t);
} 

int xdr_player_color_t (XDR* xdrs, player_color_t * msg)
{   if(xdr_u_char(xdrs,&msg->alpha) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->red) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->green) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->blue) != 1)
    return(0);
  return(1);
}
int 
player_color_pack(void* buf, size_t buflen, player_color_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_color_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_color_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_color_t_copy(player_color_t *dest, const player_color_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_color_t));
  return sizeof(player_color_t);
} 
void player_color_t_cleanup(const player_color_t *msg)
{
} 
player_color_t * player_color_t_clone(const player_color_t *msg)
{      
  player_color_t * clone = malloc(sizeof(player_color_t));
  if (clone)
    player_color_t_copy(clone,msg);
  return clone;
}
void player_color_t_free(player_color_t *msg)
{      
  player_color_t_cleanup(msg);
  free(msg);
}
unsigned int player_color_t_sizeof(player_color_t *msg)
{
  return sizeof(player_color_t);
} 

int xdr_player_bool_t (XDR* xdrs, player_bool_t * msg)
{   if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_bool_pack(void* buf, size_t buflen, player_bool_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_bool_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_bool_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_bool_t_copy(player_bool_t *dest, const player_bool_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_bool_t));
  return sizeof(player_bool_t);
} 
void player_bool_t_cleanup(const player_bool_t *msg)
{
} 
player_bool_t * player_bool_t_clone(const player_bool_t *msg)
{      
  player_bool_t * clone = malloc(sizeof(player_bool_t));
  if (clone)
    player_bool_t_copy(clone,msg);
  return clone;
}
void player_bool_t_free(player_bool_t *msg)
{      
  player_bool_t_cleanup(msg);
  free(msg);
}
unsigned int player_bool_t_sizeof(player_bool_t *msg)
{
  return sizeof(player_bool_t);
} 

int xdr_player_uint32_t (XDR* xdrs, player_uint32_t * msg)
{   if(xdr_u_int(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_uint32_pack(void* buf, size_t buflen, player_uint32_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_uint32_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_uint32_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_uint32_t_copy(player_uint32_t *dest, const player_uint32_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_uint32_t));
  return sizeof(player_uint32_t);
} 
void player_uint32_t_cleanup(const player_uint32_t *msg)
{
} 
player_uint32_t * player_uint32_t_clone(const player_uint32_t *msg)
{      
  player_uint32_t * clone = malloc(sizeof(player_uint32_t));
  if (clone)
    player_uint32_t_copy(clone,msg);
  return clone;
}
void player_uint32_t_free(player_uint32_t *msg)
{      
  player_uint32_t_cleanup(msg);
  free(msg);
}
unsigned int player_uint32_t_sizeof(player_uint32_t *msg)
{
  return sizeof(player_uint32_t);
} 

int xdr_player_capabilities_req_t (XDR* xdrs, player_capabilities_req_t * msg)
{   if(xdr_u_int(xdrs,&msg->type) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->subtype) != 1)
    return(0);
  return(1);
}
int 
player_capabilities_req_pack(void* buf, size_t buflen, player_capabilities_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_capabilities_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_capabilities_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_capabilities_req_t_copy(player_capabilities_req_t *dest, const player_capabilities_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_capabilities_req_t));
  return sizeof(player_capabilities_req_t);
} 
void player_capabilities_req_t_cleanup(const player_capabilities_req_t *msg)
{
} 
player_capabilities_req_t * player_capabilities_req_t_clone(const player_capabilities_req_t *msg)
{      
  player_capabilities_req_t * clone = malloc(sizeof(player_capabilities_req_t));
  if (clone)
    player_capabilities_req_t_copy(clone,msg);
  return clone;
}
void player_capabilities_req_t_free(player_capabilities_req_t *msg)
{      
  player_capabilities_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_capabilities_req_t_sizeof(player_capabilities_req_t *msg)
{
  return sizeof(player_capabilities_req_t);
} 

int xdr_player_intprop_req_t (XDR* xdrs, player_intprop_req_t * msg)
{   if(xdr_u_int(xdrs,&msg->key_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->key = malloc(msg->key_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* key_p = msg->key;
    if(xdr_bytes(xdrs, (char**)&key_p, &msg->key_count, msg->key_count) != 1)
      return(0);
  }
  if(xdr_int(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_intprop_req_pack(void* buf, size_t buflen, player_intprop_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_intprop_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_intprop_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_intprop_req_t_copy(player_intprop_req_t *dest, const player_intprop_req_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->key_count,&src->key_count,sizeof(uint32_t)*1); 
  if(src->key != NULL && src->key_count > 0)
  {
    if((dest->key = malloc(src->key_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->key = NULL;
  size += sizeof(char)*src->key_count;
  memcpy(dest->key,src->key,sizeof(char)*src->key_count); 
  size += sizeof(int32_t)*1;
  memcpy(&dest->value,&src->value,sizeof(int32_t)*1); 
  return(size);
}
void player_intprop_req_t_cleanup(const player_intprop_req_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->key); 
}
player_intprop_req_t * player_intprop_req_t_clone(const player_intprop_req_t *msg)
{      
  player_intprop_req_t * clone = malloc(sizeof(player_intprop_req_t));
  if (clone)
    player_intprop_req_t_copy(clone,msg);
  return clone;
}
void player_intprop_req_t_free(player_intprop_req_t *msg)
{      
  player_intprop_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_intprop_req_t_sizeof(player_intprop_req_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->key_count; 
  size += sizeof(int32_t)*1; 
  return(size);
}

int xdr_player_dblprop_req_t (XDR* xdrs, player_dblprop_req_t * msg)
{   if(xdr_u_int(xdrs,&msg->key_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->key = malloc(msg->key_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* key_p = msg->key;
    if(xdr_bytes(xdrs, (char**)&key_p, &msg->key_count, msg->key_count) != 1)
      return(0);
  }
  if(xdr_double(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_dblprop_req_pack(void* buf, size_t buflen, player_dblprop_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_dblprop_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_dblprop_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_dblprop_req_t_copy(player_dblprop_req_t *dest, const player_dblprop_req_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->key_count,&src->key_count,sizeof(uint32_t)*1); 
  if(src->key != NULL && src->key_count > 0)
  {
    if((dest->key = malloc(src->key_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->key = NULL;
  size += sizeof(char)*src->key_count;
  memcpy(dest->key,src->key,sizeof(char)*src->key_count); 
  size += sizeof(double)*1;
  memcpy(&dest->value,&src->value,sizeof(double)*1); 
  return(size);
}
void player_dblprop_req_t_cleanup(const player_dblprop_req_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->key); 
}
player_dblprop_req_t * player_dblprop_req_t_clone(const player_dblprop_req_t *msg)
{      
  player_dblprop_req_t * clone = malloc(sizeof(player_dblprop_req_t));
  if (clone)
    player_dblprop_req_t_copy(clone,msg);
  return clone;
}
void player_dblprop_req_t_free(player_dblprop_req_t *msg)
{      
  player_dblprop_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_dblprop_req_t_sizeof(player_dblprop_req_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->key_count; 
  size += sizeof(double)*1; 
  return(size);
}

int xdr_player_strprop_req_t (XDR* xdrs, player_strprop_req_t * msg)
{   if(xdr_u_int(xdrs,&msg->key_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->key = malloc(msg->key_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* key_p = msg->key;
    if(xdr_bytes(xdrs, (char**)&key_p, &msg->key_count, msg->key_count) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->value_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->value = malloc(msg->value_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* value_p = msg->value;
    if(xdr_bytes(xdrs, (char**)&value_p, &msg->value_count, msg->value_count) != 1)
      return(0);
  }
  return(1);
}
int 
player_strprop_req_pack(void* buf, size_t buflen, player_strprop_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_strprop_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_strprop_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_strprop_req_t_copy(player_strprop_req_t *dest, const player_strprop_req_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->key_count,&src->key_count,sizeof(uint32_t)*1); 
  if(src->key != NULL && src->key_count > 0)
  {
    if((dest->key = malloc(src->key_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->key = NULL;
  size += sizeof(char)*src->key_count;
  memcpy(dest->key,src->key,sizeof(char)*src->key_count); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->value_count,&src->value_count,sizeof(uint32_t)*1); 
  if(src->value != NULL && src->value_count > 0)
  {
    if((dest->value = malloc(src->value_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->value = NULL;
  size += sizeof(char)*src->value_count;
  memcpy(dest->value,src->value,sizeof(char)*src->value_count); 
  return(size);
}
void player_strprop_req_t_cleanup(const player_strprop_req_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->key); 
  free(msg->value); 
}
player_strprop_req_t * player_strprop_req_t_clone(const player_strprop_req_t *msg)
{      
  player_strprop_req_t * clone = malloc(sizeof(player_strprop_req_t));
  if (clone)
    player_strprop_req_t_copy(clone,msg);
  return clone;
}
void player_strprop_req_t_free(player_strprop_req_t *msg)
{      
  player_strprop_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_strprop_req_t_sizeof(player_strprop_req_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->key_count; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->value_count; 
  return(size);
}

int xdr_player_position2d_data_t (XDR* xdrs, player_position2d_data_t * msg)
{   if(xdr_player_pose2d_t(xdrs,&msg->pos) != 1)
    return(0);
  if(xdr_player_pose2d_t(xdrs,&msg->vel) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->stall) != 1)
    return(0);
  return(1);
}
int 
player_position2d_data_pack(void* buf, size_t buflen, player_position2d_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position2d_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position2d_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position2d_data_t_copy(player_position2d_data_t *dest, const player_position2d_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position2d_data_t));
  return sizeof(player_position2d_data_t);
} 
void player_position2d_data_t_cleanup(const player_position2d_data_t *msg)
{
} 
player_position2d_data_t * player_position2d_data_t_clone(const player_position2d_data_t *msg)
{      
  player_position2d_data_t * clone = malloc(sizeof(player_position2d_data_t));
  if (clone)
    player_position2d_data_t_copy(clone,msg);
  return clone;
}
void player_position2d_data_t_free(player_position2d_data_t *msg)
{      
  player_position2d_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_position2d_data_t_sizeof(player_position2d_data_t *msg)
{
  return sizeof(player_position2d_data_t);
} 

int xdr_player_position2d_cmd_vel_t (XDR* xdrs, player_position2d_cmd_vel_t * msg)
{   if(xdr_player_pose2d_t(xdrs,&msg->vel) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_position2d_cmd_vel_pack(void* buf, size_t buflen, player_position2d_cmd_vel_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position2d_cmd_vel_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position2d_cmd_vel_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position2d_cmd_vel_t_copy(player_position2d_cmd_vel_t *dest, const player_position2d_cmd_vel_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position2d_cmd_vel_t));
  return sizeof(player_position2d_cmd_vel_t);
} 
void player_position2d_cmd_vel_t_cleanup(const player_position2d_cmd_vel_t *msg)
{
} 
player_position2d_cmd_vel_t * player_position2d_cmd_vel_t_clone(const player_position2d_cmd_vel_t *msg)
{      
  player_position2d_cmd_vel_t * clone = malloc(sizeof(player_position2d_cmd_vel_t));
  if (clone)
    player_position2d_cmd_vel_t_copy(clone,msg);
  return clone;
}
void player_position2d_cmd_vel_t_free(player_position2d_cmd_vel_t *msg)
{      
  player_position2d_cmd_vel_t_cleanup(msg);
  free(msg);
}
unsigned int player_position2d_cmd_vel_t_sizeof(player_position2d_cmd_vel_t *msg)
{
  return sizeof(player_position2d_cmd_vel_t);
} 

int xdr_player_position2d_cmd_pos_t (XDR* xdrs, player_position2d_cmd_pos_t * msg)
{   if(xdr_player_pose2d_t(xdrs,&msg->pos) != 1)
    return(0);
  if(xdr_player_pose2d_t(xdrs,&msg->vel) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_position2d_cmd_pos_pack(void* buf, size_t buflen, player_position2d_cmd_pos_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position2d_cmd_pos_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position2d_cmd_pos_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position2d_cmd_pos_t_copy(player_position2d_cmd_pos_t *dest, const player_position2d_cmd_pos_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position2d_cmd_pos_t));
  return sizeof(player_position2d_cmd_pos_t);
} 
void player_position2d_cmd_pos_t_cleanup(const player_position2d_cmd_pos_t *msg)
{
} 
player_position2d_cmd_pos_t * player_position2d_cmd_pos_t_clone(const player_position2d_cmd_pos_t *msg)
{      
  player_position2d_cmd_pos_t * clone = malloc(sizeof(player_position2d_cmd_pos_t));
  if (clone)
    player_position2d_cmd_pos_t_copy(clone,msg);
  return clone;
}
void player_position2d_cmd_pos_t_free(player_position2d_cmd_pos_t *msg)
{      
  player_position2d_cmd_pos_t_cleanup(msg);
  free(msg);
}
unsigned int player_position2d_cmd_pos_t_sizeof(player_position2d_cmd_pos_t *msg)
{
  return sizeof(player_position2d_cmd_pos_t);
} 

int xdr_player_position2d_cmd_car_t (XDR* xdrs, player_position2d_cmd_car_t * msg)
{   if(xdr_double(xdrs,&msg->velocity) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->angle) != 1)
    return(0);
  return(1);
}
int 
player_position2d_cmd_car_pack(void* buf, size_t buflen, player_position2d_cmd_car_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position2d_cmd_car_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position2d_cmd_car_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position2d_cmd_car_t_copy(player_position2d_cmd_car_t *dest, const player_position2d_cmd_car_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position2d_cmd_car_t));
  return sizeof(player_position2d_cmd_car_t);
} 
void player_position2d_cmd_car_t_cleanup(const player_position2d_cmd_car_t *msg)
{
} 
player_position2d_cmd_car_t * player_position2d_cmd_car_t_clone(const player_position2d_cmd_car_t *msg)
{      
  player_position2d_cmd_car_t * clone = malloc(sizeof(player_position2d_cmd_car_t));
  if (clone)
    player_position2d_cmd_car_t_copy(clone,msg);
  return clone;
}
void player_position2d_cmd_car_t_free(player_position2d_cmd_car_t *msg)
{      
  player_position2d_cmd_car_t_cleanup(msg);
  free(msg);
}
unsigned int player_position2d_cmd_car_t_sizeof(player_position2d_cmd_car_t *msg)
{
  return sizeof(player_position2d_cmd_car_t);
} 

int xdr_player_position2d_cmd_vel_head_t (XDR* xdrs, player_position2d_cmd_vel_head_t * msg)
{   if(xdr_double(xdrs,&msg->velocity) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->angle) != 1)
    return(0);
  return(1);
}
int 
player_position2d_cmd_vel_head_pack(void* buf, size_t buflen, player_position2d_cmd_vel_head_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position2d_cmd_vel_head_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position2d_cmd_vel_head_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position2d_cmd_vel_head_t_copy(player_position2d_cmd_vel_head_t *dest, const player_position2d_cmd_vel_head_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position2d_cmd_vel_head_t));
  return sizeof(player_position2d_cmd_vel_head_t);
} 
void player_position2d_cmd_vel_head_t_cleanup(const player_position2d_cmd_vel_head_t *msg)
{
} 
player_position2d_cmd_vel_head_t * player_position2d_cmd_vel_head_t_clone(const player_position2d_cmd_vel_head_t *msg)
{      
  player_position2d_cmd_vel_head_t * clone = malloc(sizeof(player_position2d_cmd_vel_head_t));
  if (clone)
    player_position2d_cmd_vel_head_t_copy(clone,msg);
  return clone;
}
void player_position2d_cmd_vel_head_t_free(player_position2d_cmd_vel_head_t *msg)
{      
  player_position2d_cmd_vel_head_t_cleanup(msg);
  free(msg);
}
unsigned int player_position2d_cmd_vel_head_t_sizeof(player_position2d_cmd_vel_head_t *msg)
{
  return sizeof(player_position2d_cmd_vel_head_t);
} 

int xdr_player_position2d_geom_t (XDR* xdrs, player_position2d_geom_t * msg)
{   if(xdr_player_pose3d_t(xdrs,&msg->pose) != 1)
    return(0);
  if(xdr_player_bbox3d_t(xdrs,&msg->size) != 1)
    return(0);
  return(1);
}
int 
player_position2d_geom_pack(void* buf, size_t buflen, player_position2d_geom_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position2d_geom_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position2d_geom_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position2d_geom_t_copy(player_position2d_geom_t *dest, const player_position2d_geom_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position2d_geom_t));
  return sizeof(player_position2d_geom_t);
} 
void player_position2d_geom_t_cleanup(const player_position2d_geom_t *msg)
{
} 
player_position2d_geom_t * player_position2d_geom_t_clone(const player_position2d_geom_t *msg)
{      
  player_position2d_geom_t * clone = malloc(sizeof(player_position2d_geom_t));
  if (clone)
    player_position2d_geom_t_copy(clone,msg);
  return clone;
}
void player_position2d_geom_t_free(player_position2d_geom_t *msg)
{      
  player_position2d_geom_t_cleanup(msg);
  free(msg);
}
unsigned int player_position2d_geom_t_sizeof(player_position2d_geom_t *msg)
{
  return sizeof(player_position2d_geom_t);
} 

int xdr_player_position2d_power_config_t (XDR* xdrs, player_position2d_power_config_t * msg)
{   if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_position2d_power_config_pack(void* buf, size_t buflen, player_position2d_power_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position2d_power_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position2d_power_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position2d_power_config_t_copy(player_position2d_power_config_t *dest, const player_position2d_power_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position2d_power_config_t));
  return sizeof(player_position2d_power_config_t);
} 
void player_position2d_power_config_t_cleanup(const player_position2d_power_config_t *msg)
{
} 
player_position2d_power_config_t * player_position2d_power_config_t_clone(const player_position2d_power_config_t *msg)
{      
  player_position2d_power_config_t * clone = malloc(sizeof(player_position2d_power_config_t));
  if (clone)
    player_position2d_power_config_t_copy(clone,msg);
  return clone;
}
void player_position2d_power_config_t_free(player_position2d_power_config_t *msg)
{      
  player_position2d_power_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_position2d_power_config_t_sizeof(player_position2d_power_config_t *msg)
{
  return sizeof(player_position2d_power_config_t);
} 

int xdr_player_position2d_velocity_mode_config_t (XDR* xdrs, player_position2d_velocity_mode_config_t * msg)
{   if(xdr_u_int(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_position2d_velocity_mode_config_pack(void* buf, size_t buflen, player_position2d_velocity_mode_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position2d_velocity_mode_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position2d_velocity_mode_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position2d_velocity_mode_config_t_copy(player_position2d_velocity_mode_config_t *dest, const player_position2d_velocity_mode_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position2d_velocity_mode_config_t));
  return sizeof(player_position2d_velocity_mode_config_t);
} 
void player_position2d_velocity_mode_config_t_cleanup(const player_position2d_velocity_mode_config_t *msg)
{
} 
player_position2d_velocity_mode_config_t * player_position2d_velocity_mode_config_t_clone(const player_position2d_velocity_mode_config_t *msg)
{      
  player_position2d_velocity_mode_config_t * clone = malloc(sizeof(player_position2d_velocity_mode_config_t));
  if (clone)
    player_position2d_velocity_mode_config_t_copy(clone,msg);
  return clone;
}
void player_position2d_velocity_mode_config_t_free(player_position2d_velocity_mode_config_t *msg)
{      
  player_position2d_velocity_mode_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_position2d_velocity_mode_config_t_sizeof(player_position2d_velocity_mode_config_t *msg)
{
  return sizeof(player_position2d_velocity_mode_config_t);
} 

int xdr_player_position2d_position_mode_req_t (XDR* xdrs, player_position2d_position_mode_req_t * msg)
{   if(xdr_u_int(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_position2d_position_mode_req_pack(void* buf, size_t buflen, player_position2d_position_mode_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position2d_position_mode_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position2d_position_mode_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position2d_position_mode_req_t_copy(player_position2d_position_mode_req_t *dest, const player_position2d_position_mode_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position2d_position_mode_req_t));
  return sizeof(player_position2d_position_mode_req_t);
} 
void player_position2d_position_mode_req_t_cleanup(const player_position2d_position_mode_req_t *msg)
{
} 
player_position2d_position_mode_req_t * player_position2d_position_mode_req_t_clone(const player_position2d_position_mode_req_t *msg)
{      
  player_position2d_position_mode_req_t * clone = malloc(sizeof(player_position2d_position_mode_req_t));
  if (clone)
    player_position2d_position_mode_req_t_copy(clone,msg);
  return clone;
}
void player_position2d_position_mode_req_t_free(player_position2d_position_mode_req_t *msg)
{      
  player_position2d_position_mode_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position2d_position_mode_req_t_sizeof(player_position2d_position_mode_req_t *msg)
{
  return sizeof(player_position2d_position_mode_req_t);
} 

int xdr_player_position2d_set_odom_req_t (XDR* xdrs, player_position2d_set_odom_req_t * msg)
{   if(xdr_player_pose2d_t(xdrs,&msg->pose) != 1)
    return(0);
  return(1);
}
int 
player_position2d_set_odom_req_pack(void* buf, size_t buflen, player_position2d_set_odom_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position2d_set_odom_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position2d_set_odom_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position2d_set_odom_req_t_copy(player_position2d_set_odom_req_t *dest, const player_position2d_set_odom_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position2d_set_odom_req_t));
  return sizeof(player_position2d_set_odom_req_t);
} 
void player_position2d_set_odom_req_t_cleanup(const player_position2d_set_odom_req_t *msg)
{
} 
player_position2d_set_odom_req_t * player_position2d_set_odom_req_t_clone(const player_position2d_set_odom_req_t *msg)
{      
  player_position2d_set_odom_req_t * clone = malloc(sizeof(player_position2d_set_odom_req_t));
  if (clone)
    player_position2d_set_odom_req_t_copy(clone,msg);
  return clone;
}
void player_position2d_set_odom_req_t_free(player_position2d_set_odom_req_t *msg)
{      
  player_position2d_set_odom_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position2d_set_odom_req_t_sizeof(player_position2d_set_odom_req_t *msg)
{
  return sizeof(player_position2d_set_odom_req_t);
} 

int xdr_player_position2d_speed_pid_req_t (XDR* xdrs, player_position2d_speed_pid_req_t * msg)
{   if(xdr_float(xdrs,&msg->kp) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->ki) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->kd) != 1)
    return(0);
  return(1);
}
int 
player_position2d_speed_pid_req_pack(void* buf, size_t buflen, player_position2d_speed_pid_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position2d_speed_pid_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position2d_speed_pid_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position2d_speed_pid_req_t_copy(player_position2d_speed_pid_req_t *dest, const player_position2d_speed_pid_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position2d_speed_pid_req_t));
  return sizeof(player_position2d_speed_pid_req_t);
} 
void player_position2d_speed_pid_req_t_cleanup(const player_position2d_speed_pid_req_t *msg)
{
} 
player_position2d_speed_pid_req_t * player_position2d_speed_pid_req_t_clone(const player_position2d_speed_pid_req_t *msg)
{      
  player_position2d_speed_pid_req_t * clone = malloc(sizeof(player_position2d_speed_pid_req_t));
  if (clone)
    player_position2d_speed_pid_req_t_copy(clone,msg);
  return clone;
}
void player_position2d_speed_pid_req_t_free(player_position2d_speed_pid_req_t *msg)
{      
  player_position2d_speed_pid_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position2d_speed_pid_req_t_sizeof(player_position2d_speed_pid_req_t *msg)
{
  return sizeof(player_position2d_speed_pid_req_t);
} 

int xdr_player_position2d_position_pid_req_t (XDR* xdrs, player_position2d_position_pid_req_t * msg)
{   if(xdr_float(xdrs,&msg->kp) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->ki) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->kd) != 1)
    return(0);
  return(1);
}
int 
player_position2d_position_pid_req_pack(void* buf, size_t buflen, player_position2d_position_pid_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position2d_position_pid_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position2d_position_pid_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position2d_position_pid_req_t_copy(player_position2d_position_pid_req_t *dest, const player_position2d_position_pid_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position2d_position_pid_req_t));
  return sizeof(player_position2d_position_pid_req_t);
} 
void player_position2d_position_pid_req_t_cleanup(const player_position2d_position_pid_req_t *msg)
{
} 
player_position2d_position_pid_req_t * player_position2d_position_pid_req_t_clone(const player_position2d_position_pid_req_t *msg)
{      
  player_position2d_position_pid_req_t * clone = malloc(sizeof(player_position2d_position_pid_req_t));
  if (clone)
    player_position2d_position_pid_req_t_copy(clone,msg);
  return clone;
}
void player_position2d_position_pid_req_t_free(player_position2d_position_pid_req_t *msg)
{      
  player_position2d_position_pid_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position2d_position_pid_req_t_sizeof(player_position2d_position_pid_req_t *msg)
{
  return sizeof(player_position2d_position_pid_req_t);
} 

int xdr_player_position2d_speed_prof_req_t (XDR* xdrs, player_position2d_speed_prof_req_t * msg)
{   if(xdr_float(xdrs,&msg->speed) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->acc) != 1)
    return(0);
  return(1);
}
int 
player_position2d_speed_prof_req_pack(void* buf, size_t buflen, player_position2d_speed_prof_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position2d_speed_prof_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position2d_speed_prof_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position2d_speed_prof_req_t_copy(player_position2d_speed_prof_req_t *dest, const player_position2d_speed_prof_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position2d_speed_prof_req_t));
  return sizeof(player_position2d_speed_prof_req_t);
} 
void player_position2d_speed_prof_req_t_cleanup(const player_position2d_speed_prof_req_t *msg)
{
} 
player_position2d_speed_prof_req_t * player_position2d_speed_prof_req_t_clone(const player_position2d_speed_prof_req_t *msg)
{      
  player_position2d_speed_prof_req_t * clone = malloc(sizeof(player_position2d_speed_prof_req_t));
  if (clone)
    player_position2d_speed_prof_req_t_copy(clone,msg);
  return clone;
}
void player_position2d_speed_prof_req_t_free(player_position2d_speed_prof_req_t *msg)
{      
  player_position2d_speed_prof_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position2d_speed_prof_req_t_sizeof(player_position2d_speed_prof_req_t *msg)
{
  return sizeof(player_position2d_speed_prof_req_t);
} 

int xdr_player_wifi_link_t (XDR* xdrs, player_wifi_link_t * msg)
{   if(xdr_u_int(xdrs,&msg->mac_count) != 1)
    return(0);
  {
    uint8_t* mac_p = msg->mac;
    if(xdr_bytes(xdrs, (char**)&mac_p, &msg->mac_count, 32) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->ip_count) != 1)
    return(0);
  {
    uint8_t* ip_p = msg->ip;
    if(xdr_bytes(xdrs, (char**)&ip_p, &msg->ip_count, 32) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->essid_count) != 1)
    return(0);
  {
    uint8_t* essid_p = msg->essid;
    if(xdr_bytes(xdrs, (char**)&essid_p, &msg->essid_count, 32) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->mode) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->freq) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->encrypt) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->qual) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->level) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->noise) != 1)
    return(0);
  return(1);
}
int 
player_wifi_link_pack(void* buf, size_t buflen, player_wifi_link_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_wifi_link_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_wifi_link_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_wifi_link_t_copy(player_wifi_link_t *dest, const player_wifi_link_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_wifi_link_t));
  return sizeof(player_wifi_link_t);
} 
void player_wifi_link_t_cleanup(const player_wifi_link_t *msg)
{
} 
player_wifi_link_t * player_wifi_link_t_clone(const player_wifi_link_t *msg)
{      
  player_wifi_link_t * clone = malloc(sizeof(player_wifi_link_t));
  if (clone)
    player_wifi_link_t_copy(clone,msg);
  return clone;
}
void player_wifi_link_t_free(player_wifi_link_t *msg)
{      
  player_wifi_link_t_cleanup(msg);
  free(msg);
}
unsigned int player_wifi_link_t_sizeof(player_wifi_link_t *msg)
{
  return sizeof(player_wifi_link_t);
} 

int xdr_player_wifi_data_t (XDR* xdrs, player_wifi_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->links_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->links = malloc(msg->links_count*sizeof(player_wifi_link_t))) == NULL)
      return(0);
  }
  {
    player_wifi_link_t* links_p = msg->links;
    if(xdr_array(xdrs, (char**)&links_p, &msg->links_count, msg->links_count, sizeof(player_wifi_link_t), (xdrproc_t)xdr_player_wifi_link_t) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->throughput) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->bitrate) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->mode) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->qual_type) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->maxqual) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->maxlevel) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->maxnoise) != 1)
    return(0);
  if(xdr_opaque(xdrs, (char*)&msg->ap, 32) != 1)
    return(0);
  return(1);
}
int 
player_wifi_data_pack(void* buf, size_t buflen, player_wifi_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_wifi_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_wifi_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_wifi_data_t_copy(player_wifi_data_t *dest, const player_wifi_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->links_count,&src->links_count,sizeof(uint32_t)*1); 
  if(src->links != NULL && src->links_count > 0)
  {
    if((dest->links = malloc(src->links_count*sizeof(player_wifi_link_t))) == NULL)
      return(0);
  }
  else
    dest->links = NULL;
  size += sizeof(player_wifi_link_t)*src->links_count;
  memcpy(dest->links,src->links,sizeof(player_wifi_link_t)*src->links_count); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->throughput,&src->throughput,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->bitrate,&src->bitrate,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->mode,&src->mode,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->qual_type,&src->qual_type,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->maxqual,&src->maxqual,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->maxlevel,&src->maxlevel,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->maxnoise,&src->maxnoise,sizeof(uint32_t)*1); 
  size += sizeof(char)*32;
  memcpy(dest->ap,src->ap,sizeof(char)*32); 
  return(size);
}
void player_wifi_data_t_cleanup(const player_wifi_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->links); 
}
player_wifi_data_t * player_wifi_data_t_clone(const player_wifi_data_t *msg)
{      
  player_wifi_data_t * clone = malloc(sizeof(player_wifi_data_t));
  if (clone)
    player_wifi_data_t_copy(clone,msg);
  return clone;
}
void player_wifi_data_t_free(player_wifi_data_t *msg)
{      
  player_wifi_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_wifi_data_t_sizeof(player_wifi_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_wifi_link_t)*msg->links_count; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*32; 
  return(size);
}

int xdr_player_wifi_mac_req_t (XDR* xdrs, player_wifi_mac_req_t * msg)
{   if(xdr_u_int(xdrs,&msg->mac_count) != 1)
    return(0);
  {
    uint8_t* mac_p = msg->mac;
    if(xdr_bytes(xdrs, (char**)&mac_p, &msg->mac_count, 32) != 1)
      return(0);
  }
  return(1);
}
int 
player_wifi_mac_req_pack(void* buf, size_t buflen, player_wifi_mac_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_wifi_mac_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_wifi_mac_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_wifi_mac_req_t_copy(player_wifi_mac_req_t *dest, const player_wifi_mac_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_wifi_mac_req_t));
  return sizeof(player_wifi_mac_req_t);
} 
void player_wifi_mac_req_t_cleanup(const player_wifi_mac_req_t *msg)
{
} 
player_wifi_mac_req_t * player_wifi_mac_req_t_clone(const player_wifi_mac_req_t *msg)
{      
  player_wifi_mac_req_t * clone = malloc(sizeof(player_wifi_mac_req_t));
  if (clone)
    player_wifi_mac_req_t_copy(clone,msg);
  return clone;
}
void player_wifi_mac_req_t_free(player_wifi_mac_req_t *msg)
{      
  player_wifi_mac_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_wifi_mac_req_t_sizeof(player_wifi_mac_req_t *msg)
{
  return sizeof(player_wifi_mac_req_t);
} 

int xdr_player_wifi_iwspy_addr_req_t (XDR* xdrs, player_wifi_iwspy_addr_req_t * msg)
{   if(xdr_opaque(xdrs, (char*)&msg->address, 32) != 1)
    return(0);
  return(1);
}
int 
player_wifi_iwspy_addr_req_pack(void* buf, size_t buflen, player_wifi_iwspy_addr_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_wifi_iwspy_addr_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_wifi_iwspy_addr_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_wifi_iwspy_addr_req_t_copy(player_wifi_iwspy_addr_req_t *dest, const player_wifi_iwspy_addr_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_wifi_iwspy_addr_req_t));
  return sizeof(player_wifi_iwspy_addr_req_t);
} 
void player_wifi_iwspy_addr_req_t_cleanup(const player_wifi_iwspy_addr_req_t *msg)
{
} 
player_wifi_iwspy_addr_req_t * player_wifi_iwspy_addr_req_t_clone(const player_wifi_iwspy_addr_req_t *msg)
{      
  player_wifi_iwspy_addr_req_t * clone = malloc(sizeof(player_wifi_iwspy_addr_req_t));
  if (clone)
    player_wifi_iwspy_addr_req_t_copy(clone,msg);
  return clone;
}
void player_wifi_iwspy_addr_req_t_free(player_wifi_iwspy_addr_req_t *msg)
{      
  player_wifi_iwspy_addr_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_wifi_iwspy_addr_req_t_sizeof(player_wifi_iwspy_addr_req_t *msg)
{
  return sizeof(player_wifi_iwspy_addr_req_t);
} 

int xdr_player_dio_data_t (XDR* xdrs, player_dio_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->count) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->bits) != 1)
    return(0);
  return(1);
}
int 
player_dio_data_pack(void* buf, size_t buflen, player_dio_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_dio_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_dio_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_dio_data_t_copy(player_dio_data_t *dest, const player_dio_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_dio_data_t));
  return sizeof(player_dio_data_t);
} 
void player_dio_data_t_cleanup(const player_dio_data_t *msg)
{
} 
player_dio_data_t * player_dio_data_t_clone(const player_dio_data_t *msg)
{      
  player_dio_data_t * clone = malloc(sizeof(player_dio_data_t));
  if (clone)
    player_dio_data_t_copy(clone,msg);
  return clone;
}
void player_dio_data_t_free(player_dio_data_t *msg)
{      
  player_dio_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_dio_data_t_sizeof(player_dio_data_t *msg)
{
  return sizeof(player_dio_data_t);
} 

int xdr_player_dio_cmd_t (XDR* xdrs, player_dio_cmd_t * msg)
{   if(xdr_u_int(xdrs,&msg->count) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->digout) != 1)
    return(0);
  return(1);
}
int 
player_dio_cmd_pack(void* buf, size_t buflen, player_dio_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_dio_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_dio_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_dio_cmd_t_copy(player_dio_cmd_t *dest, const player_dio_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_dio_cmd_t));
  return sizeof(player_dio_cmd_t);
} 
void player_dio_cmd_t_cleanup(const player_dio_cmd_t *msg)
{
} 
player_dio_cmd_t * player_dio_cmd_t_clone(const player_dio_cmd_t *msg)
{      
  player_dio_cmd_t * clone = malloc(sizeof(player_dio_cmd_t));
  if (clone)
    player_dio_cmd_t_copy(clone,msg);
  return clone;
}
void player_dio_cmd_t_free(player_dio_cmd_t *msg)
{      
  player_dio_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_dio_cmd_t_sizeof(player_dio_cmd_t *msg)
{
  return sizeof(player_dio_cmd_t);
} 

int xdr_player_wsn_node_data_t (XDR* xdrs, player_wsn_node_data_t * msg)
{   if(xdr_float(xdrs,&msg->light) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->mic) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->accel_x) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->accel_y) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->accel_z) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->magn_x) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->magn_y) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->magn_z) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->temperature) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->battery) != 1)
    return(0);
  return(1);
}
int 
player_wsn_node_data_pack(void* buf, size_t buflen, player_wsn_node_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_wsn_node_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_wsn_node_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_wsn_node_data_t_copy(player_wsn_node_data_t *dest, const player_wsn_node_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_wsn_node_data_t));
  return sizeof(player_wsn_node_data_t);
} 
void player_wsn_node_data_t_cleanup(const player_wsn_node_data_t *msg)
{
} 
player_wsn_node_data_t * player_wsn_node_data_t_clone(const player_wsn_node_data_t *msg)
{      
  player_wsn_node_data_t * clone = malloc(sizeof(player_wsn_node_data_t));
  if (clone)
    player_wsn_node_data_t_copy(clone,msg);
  return clone;
}
void player_wsn_node_data_t_free(player_wsn_node_data_t *msg)
{      
  player_wsn_node_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_wsn_node_data_t_sizeof(player_wsn_node_data_t *msg)
{
  return sizeof(player_wsn_node_data_t);
} 

int xdr_player_wsn_data_t (XDR* xdrs, player_wsn_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->node_type) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->node_id) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->node_parent_id) != 1)
    return(0);
  if(xdr_player_wsn_node_data_t(xdrs,&msg->data_packet) != 1)
    return(0);
  return(1);
}
int 
player_wsn_data_pack(void* buf, size_t buflen, player_wsn_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_wsn_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_wsn_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_wsn_data_t_copy(player_wsn_data_t *dest, const player_wsn_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_wsn_data_t));
  return sizeof(player_wsn_data_t);
} 
void player_wsn_data_t_cleanup(const player_wsn_data_t *msg)
{
} 
player_wsn_data_t * player_wsn_data_t_clone(const player_wsn_data_t *msg)
{      
  player_wsn_data_t * clone = malloc(sizeof(player_wsn_data_t));
  if (clone)
    player_wsn_data_t_copy(clone,msg);
  return clone;
}
void player_wsn_data_t_free(player_wsn_data_t *msg)
{      
  player_wsn_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_wsn_data_t_sizeof(player_wsn_data_t *msg)
{
  return sizeof(player_wsn_data_t);
} 

int xdr_player_wsn_cmd_t (XDR* xdrs, player_wsn_cmd_t * msg)
{   if(xdr_int(xdrs,&msg->node_id) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->group_id) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->device) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->enable) != 1)
    return(0);
  return(1);
}
int 
player_wsn_cmd_pack(void* buf, size_t buflen, player_wsn_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_wsn_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_wsn_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_wsn_cmd_t_copy(player_wsn_cmd_t *dest, const player_wsn_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_wsn_cmd_t));
  return sizeof(player_wsn_cmd_t);
} 
void player_wsn_cmd_t_cleanup(const player_wsn_cmd_t *msg)
{
} 
player_wsn_cmd_t * player_wsn_cmd_t_clone(const player_wsn_cmd_t *msg)
{      
  player_wsn_cmd_t * clone = malloc(sizeof(player_wsn_cmd_t));
  if (clone)
    player_wsn_cmd_t_copy(clone,msg);
  return clone;
}
void player_wsn_cmd_t_free(player_wsn_cmd_t *msg)
{      
  player_wsn_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_wsn_cmd_t_sizeof(player_wsn_cmd_t *msg)
{
  return sizeof(player_wsn_cmd_t);
} 

int xdr_player_wsn_power_config_t (XDR* xdrs, player_wsn_power_config_t * msg)
{   if(xdr_int(xdrs,&msg->node_id) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->group_id) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_wsn_power_config_pack(void* buf, size_t buflen, player_wsn_power_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_wsn_power_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_wsn_power_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_wsn_power_config_t_copy(player_wsn_power_config_t *dest, const player_wsn_power_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_wsn_power_config_t));
  return sizeof(player_wsn_power_config_t);
} 
void player_wsn_power_config_t_cleanup(const player_wsn_power_config_t *msg)
{
} 
player_wsn_power_config_t * player_wsn_power_config_t_clone(const player_wsn_power_config_t *msg)
{      
  player_wsn_power_config_t * clone = malloc(sizeof(player_wsn_power_config_t));
  if (clone)
    player_wsn_power_config_t_copy(clone,msg);
  return clone;
}
void player_wsn_power_config_t_free(player_wsn_power_config_t *msg)
{      
  player_wsn_power_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_wsn_power_config_t_sizeof(player_wsn_power_config_t *msg)
{
  return sizeof(player_wsn_power_config_t);
} 

int xdr_player_wsn_datatype_config_t (XDR* xdrs, player_wsn_datatype_config_t * msg)
{   if(xdr_u_char(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_wsn_datatype_config_pack(void* buf, size_t buflen, player_wsn_datatype_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_wsn_datatype_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_wsn_datatype_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_wsn_datatype_config_t_copy(player_wsn_datatype_config_t *dest, const player_wsn_datatype_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_wsn_datatype_config_t));
  return sizeof(player_wsn_datatype_config_t);
} 
void player_wsn_datatype_config_t_cleanup(const player_wsn_datatype_config_t *msg)
{
} 
player_wsn_datatype_config_t * player_wsn_datatype_config_t_clone(const player_wsn_datatype_config_t *msg)
{      
  player_wsn_datatype_config_t * clone = malloc(sizeof(player_wsn_datatype_config_t));
  if (clone)
    player_wsn_datatype_config_t_copy(clone,msg);
  return clone;
}
void player_wsn_datatype_config_t_free(player_wsn_datatype_config_t *msg)
{      
  player_wsn_datatype_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_wsn_datatype_config_t_sizeof(player_wsn_datatype_config_t *msg)
{
  return sizeof(player_wsn_datatype_config_t);
} 

int xdr_player_wsn_datafreq_config_t (XDR* xdrs, player_wsn_datafreq_config_t * msg)
{   if(xdr_int(xdrs,&msg->node_id) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->group_id) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->frequency) != 1)
    return(0);
  return(1);
}
int 
player_wsn_datafreq_config_pack(void* buf, size_t buflen, player_wsn_datafreq_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_wsn_datafreq_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_wsn_datafreq_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_wsn_datafreq_config_t_copy(player_wsn_datafreq_config_t *dest, const player_wsn_datafreq_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_wsn_datafreq_config_t));
  return sizeof(player_wsn_datafreq_config_t);
} 
void player_wsn_datafreq_config_t_cleanup(const player_wsn_datafreq_config_t *msg)
{
} 
player_wsn_datafreq_config_t * player_wsn_datafreq_config_t_clone(const player_wsn_datafreq_config_t *msg)
{      
  player_wsn_datafreq_config_t * clone = malloc(sizeof(player_wsn_datafreq_config_t));
  if (clone)
    player_wsn_datafreq_config_t_copy(clone,msg);
  return clone;
}
void player_wsn_datafreq_config_t_free(player_wsn_datafreq_config_t *msg)
{      
  player_wsn_datafreq_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_wsn_datafreq_config_t_sizeof(player_wsn_datafreq_config_t *msg)
{
  return sizeof(player_wsn_datafreq_config_t);
} 

int xdr_player_simulation_data_t (XDR* xdrs, player_simulation_data_t * msg)
{   if(xdr_u_char(xdrs,&msg->data) != 1)
    return(0);
  return(1);
}
int 
player_simulation_data_pack(void* buf, size_t buflen, player_simulation_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_simulation_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_simulation_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_simulation_data_t_copy(player_simulation_data_t *dest, const player_simulation_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_simulation_data_t));
  return sizeof(player_simulation_data_t);
} 
void player_simulation_data_t_cleanup(const player_simulation_data_t *msg)
{
} 
player_simulation_data_t * player_simulation_data_t_clone(const player_simulation_data_t *msg)
{      
  player_simulation_data_t * clone = malloc(sizeof(player_simulation_data_t));
  if (clone)
    player_simulation_data_t_copy(clone,msg);
  return clone;
}
void player_simulation_data_t_free(player_simulation_data_t *msg)
{      
  player_simulation_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_simulation_data_t_sizeof(player_simulation_data_t *msg)
{
  return sizeof(player_simulation_data_t);
} 

int xdr_player_simulation_cmd_t (XDR* xdrs, player_simulation_cmd_t * msg)
{   if(xdr_u_char(xdrs,&msg->cmd) != 1)
    return(0);
  return(1);
}
int 
player_simulation_cmd_pack(void* buf, size_t buflen, player_simulation_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_simulation_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_simulation_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_simulation_cmd_t_copy(player_simulation_cmd_t *dest, const player_simulation_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_simulation_cmd_t));
  return sizeof(player_simulation_cmd_t);
} 
void player_simulation_cmd_t_cleanup(const player_simulation_cmd_t *msg)
{
} 
player_simulation_cmd_t * player_simulation_cmd_t_clone(const player_simulation_cmd_t *msg)
{      
  player_simulation_cmd_t * clone = malloc(sizeof(player_simulation_cmd_t));
  if (clone)
    player_simulation_cmd_t_copy(clone,msg);
  return clone;
}
void player_simulation_cmd_t_free(player_simulation_cmd_t *msg)
{      
  player_simulation_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_simulation_cmd_t_sizeof(player_simulation_cmd_t *msg)
{
  return sizeof(player_simulation_cmd_t);
} 

int xdr_player_simulation_pose2d_req_t (XDR* xdrs, player_simulation_pose2d_req_t * msg)
{   if(xdr_u_int(xdrs,&msg->name_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->name = malloc(msg->name_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* name_p = msg->name;
    if(xdr_bytes(xdrs, (char**)&name_p, &msg->name_count, msg->name_count) != 1)
      return(0);
  }
  if(xdr_player_pose2d_t(xdrs,&msg->pose) != 1)
    return(0);
  return(1);
}
int 
player_simulation_pose2d_req_pack(void* buf, size_t buflen, player_simulation_pose2d_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_simulation_pose2d_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_simulation_pose2d_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_simulation_pose2d_req_t_copy(player_simulation_pose2d_req_t *dest, const player_simulation_pose2d_req_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->name_count,&src->name_count,sizeof(uint32_t)*1); 
  if(src->name != NULL && src->name_count > 0)
  {
    if((dest->name = malloc(src->name_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->name = NULL;
  size += sizeof(char)*src->name_count;
  memcpy(dest->name,src->name,sizeof(char)*src->name_count); 
  size += sizeof(player_pose2d_t)*1;
  memcpy(&dest->pose,&src->pose,sizeof(player_pose2d_t)*1); 
  return(size);
}
void player_simulation_pose2d_req_t_cleanup(const player_simulation_pose2d_req_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->name); 
}
player_simulation_pose2d_req_t * player_simulation_pose2d_req_t_clone(const player_simulation_pose2d_req_t *msg)
{      
  player_simulation_pose2d_req_t * clone = malloc(sizeof(player_simulation_pose2d_req_t));
  if (clone)
    player_simulation_pose2d_req_t_copy(clone,msg);
  return clone;
}
void player_simulation_pose2d_req_t_free(player_simulation_pose2d_req_t *msg)
{      
  player_simulation_pose2d_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_simulation_pose2d_req_t_sizeof(player_simulation_pose2d_req_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->name_count; 
  size += sizeof(player_pose2d_t)*1; 
  return(size);
}

int xdr_player_simulation_pose3d_req_t (XDR* xdrs, player_simulation_pose3d_req_t * msg)
{   if(xdr_u_int(xdrs,&msg->name_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->name = malloc(msg->name_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* name_p = msg->name;
    if(xdr_bytes(xdrs, (char**)&name_p, &msg->name_count, msg->name_count) != 1)
      return(0);
  }
  if(xdr_player_pose3d_t(xdrs,&msg->pose) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->simtime) != 1)
    return(0);
  return(1);
}
int 
player_simulation_pose3d_req_pack(void* buf, size_t buflen, player_simulation_pose3d_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_simulation_pose3d_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_simulation_pose3d_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_simulation_pose3d_req_t_copy(player_simulation_pose3d_req_t *dest, const player_simulation_pose3d_req_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->name_count,&src->name_count,sizeof(uint32_t)*1); 
  if(src->name != NULL && src->name_count > 0)
  {
    if((dest->name = malloc(src->name_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->name = NULL;
  size += sizeof(char)*src->name_count;
  memcpy(dest->name,src->name,sizeof(char)*src->name_count); 
  size += sizeof(player_pose3d_t)*1;
  memcpy(&dest->pose,&src->pose,sizeof(player_pose3d_t)*1); 
  size += sizeof(double)*1;
  memcpy(&dest->simtime,&src->simtime,sizeof(double)*1); 
  return(size);
}
void player_simulation_pose3d_req_t_cleanup(const player_simulation_pose3d_req_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->name); 
}
player_simulation_pose3d_req_t * player_simulation_pose3d_req_t_clone(const player_simulation_pose3d_req_t *msg)
{      
  player_simulation_pose3d_req_t * clone = malloc(sizeof(player_simulation_pose3d_req_t));
  if (clone)
    player_simulation_pose3d_req_t_copy(clone,msg);
  return clone;
}
void player_simulation_pose3d_req_t_free(player_simulation_pose3d_req_t *msg)
{      
  player_simulation_pose3d_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_simulation_pose3d_req_t_sizeof(player_simulation_pose3d_req_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->name_count; 
  size += sizeof(player_pose3d_t)*1; 
  size += sizeof(double)*1; 
  return(size);
}

int xdr_player_simulation_property_req_t (XDR* xdrs, player_simulation_property_req_t * msg)
{   if(xdr_u_int(xdrs,&msg->name_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->name = malloc(msg->name_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* name_p = msg->name;
    if(xdr_bytes(xdrs, (char**)&name_p, &msg->name_count, msg->name_count) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->prop_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->prop = malloc(msg->prop_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* prop_p = msg->prop;
    if(xdr_bytes(xdrs, (char**)&prop_p, &msg->prop_count, msg->prop_count) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->value_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->value = malloc(msg->value_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* value_p = msg->value;
    if(xdr_bytes(xdrs, (char**)&value_p, &msg->value_count, msg->value_count) != 1)
      return(0);
  }
  return(1);
}
int 
player_simulation_property_req_pack(void* buf, size_t buflen, player_simulation_property_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_simulation_property_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_simulation_property_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_simulation_property_req_t_copy(player_simulation_property_req_t *dest, const player_simulation_property_req_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->name_count,&src->name_count,sizeof(uint32_t)*1); 
  if(src->name != NULL && src->name_count > 0)
  {
    if((dest->name = malloc(src->name_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->name = NULL;
  size += sizeof(char)*src->name_count;
  memcpy(dest->name,src->name,sizeof(char)*src->name_count); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->prop_count,&src->prop_count,sizeof(uint32_t)*1); 
  if(src->prop != NULL && src->prop_count > 0)
  {
    if((dest->prop = malloc(src->prop_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->prop = NULL;
  size += sizeof(char)*src->prop_count;
  memcpy(dest->prop,src->prop,sizeof(char)*src->prop_count); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->value_count,&src->value_count,sizeof(uint32_t)*1); 
  if(src->value != NULL && src->value_count > 0)
  {
    if((dest->value = malloc(src->value_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->value = NULL;
  size += sizeof(char)*src->value_count;
  memcpy(dest->value,src->value,sizeof(char)*src->value_count); 
  return(size);
}
void player_simulation_property_req_t_cleanup(const player_simulation_property_req_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->name); 
  free(msg->prop); 
  free(msg->value); 
}
player_simulation_property_req_t * player_simulation_property_req_t_clone(const player_simulation_property_req_t *msg)
{      
  player_simulation_property_req_t * clone = malloc(sizeof(player_simulation_property_req_t));
  if (clone)
    player_simulation_property_req_t_copy(clone,msg);
  return clone;
}
void player_simulation_property_req_t_free(player_simulation_property_req_t *msg)
{      
  player_simulation_property_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_simulation_property_req_t_sizeof(player_simulation_property_req_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->name_count; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->prop_count; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->value_count; 
  return(size);
}

int xdr_player_camera_data_t (XDR* xdrs, player_camera_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->width) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->height) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->bpp) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->format) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->fdiv) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->compression) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->image_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->image = malloc(msg->image_count*sizeof(uint8_t))) == NULL)
      return(0);
  }
  {
    uint8_t* image_p = msg->image;
    if(xdr_bytes(xdrs, (char**)&image_p, &msg->image_count, msg->image_count) != 1)
      return(0);
  }
  return(1);
}
int 
player_camera_data_pack(void* buf, size_t buflen, player_camera_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_camera_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_camera_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_camera_data_t_copy(player_camera_data_t *dest, const player_camera_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->width,&src->width,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->height,&src->height,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->bpp,&src->bpp,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->format,&src->format,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->fdiv,&src->fdiv,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->compression,&src->compression,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->image_count,&src->image_count,sizeof(uint32_t)*1); 
  if(src->image != NULL && src->image_count > 0)
  {
    if((dest->image = malloc(src->image_count*sizeof(uint8_t))) == NULL)
      return(0);
  }
  else
    dest->image = NULL;
  size += sizeof(uint8_t)*src->image_count;
  memcpy(dest->image,src->image,sizeof(uint8_t)*src->image_count); 
  return(size);
}
void player_camera_data_t_cleanup(const player_camera_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->image); 
}
player_camera_data_t * player_camera_data_t_clone(const player_camera_data_t *msg)
{      
  player_camera_data_t * clone = malloc(sizeof(player_camera_data_t));
  if (clone)
    player_camera_data_t_copy(clone,msg);
  return clone;
}
void player_camera_data_t_free(player_camera_data_t *msg)
{      
  player_camera_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_camera_data_t_sizeof(player_camera_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint8_t)*msg->image_count; 
  return(size);
}

int xdr_player_map_info_t (XDR* xdrs, player_map_info_t * msg)
{   if(xdr_float(xdrs,&msg->scale) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->width) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->height) != 1)
    return(0);
  if(xdr_player_pose2d_t(xdrs,&msg->origin) != 1)
    return(0);
  return(1);
}
int 
player_map_info_pack(void* buf, size_t buflen, player_map_info_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_map_info_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_map_info_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_map_info_t_copy(player_map_info_t *dest, const player_map_info_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_map_info_t));
  return sizeof(player_map_info_t);
} 
void player_map_info_t_cleanup(const player_map_info_t *msg)
{
} 
player_map_info_t * player_map_info_t_clone(const player_map_info_t *msg)
{      
  player_map_info_t * clone = malloc(sizeof(player_map_info_t));
  if (clone)
    player_map_info_t_copy(clone,msg);
  return clone;
}
void player_map_info_t_free(player_map_info_t *msg)
{      
  player_map_info_t_cleanup(msg);
  free(msg);
}
unsigned int player_map_info_t_sizeof(player_map_info_t *msg)
{
  return sizeof(player_map_info_t);
} 

int xdr_player_map_data_t (XDR* xdrs, player_map_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->col) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->row) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->width) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->height) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->data_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->data = malloc(msg->data_count*sizeof(int8_t))) == NULL)
      return(0);
  }
  {
    int8_t* data_p = msg->data;
    if(xdr_bytes(xdrs, (char**)&data_p, &msg->data_count, msg->data_count) != 1)
      return(0);
  }
  return(1);
}
int 
player_map_data_pack(void* buf, size_t buflen, player_map_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_map_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_map_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_map_data_t_copy(player_map_data_t *dest, const player_map_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->col,&src->col,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->row,&src->row,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->width,&src->width,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->height,&src->height,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->data_count,&src->data_count,sizeof(uint32_t)*1); 
  if(src->data != NULL && src->data_count > 0)
  {
    if((dest->data = malloc(src->data_count*sizeof(int8_t))) == NULL)
      return(0);
  }
  else
    dest->data = NULL;
  size += sizeof(int8_t)*src->data_count;
  memcpy(dest->data,src->data,sizeof(int8_t)*src->data_count); 
  return(size);
}
void player_map_data_t_cleanup(const player_map_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->data); 
}
player_map_data_t * player_map_data_t_clone(const player_map_data_t *msg)
{      
  player_map_data_t * clone = malloc(sizeof(player_map_data_t));
  if (clone)
    player_map_data_t_copy(clone,msg);
  return clone;
}
void player_map_data_t_free(player_map_data_t *msg)
{      
  player_map_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_map_data_t_sizeof(player_map_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(int8_t)*msg->data_count; 
  return(size);
}

int xdr_player_map_data_vector_t (XDR* xdrs, player_map_data_vector_t * msg)
{   if(xdr_float(xdrs,&msg->minx) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->maxx) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->miny) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->maxy) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->segments_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->segments = malloc(msg->segments_count*sizeof(player_segment_t))) == NULL)
      return(0);
  }
  {
    player_segment_t* segments_p = msg->segments;
    if(xdr_array(xdrs, (char**)&segments_p, &msg->segments_count, msg->segments_count, sizeof(player_segment_t), (xdrproc_t)xdr_player_segment_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_map_data_vector_pack(void* buf, size_t buflen, player_map_data_vector_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_map_data_vector_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_map_data_vector_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_map_data_vector_t_copy(player_map_data_vector_t *dest, const player_map_data_vector_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(float)*1;
  memcpy(&dest->minx,&src->minx,sizeof(float)*1); 
  size += sizeof(float)*1;
  memcpy(&dest->maxx,&src->maxx,sizeof(float)*1); 
  size += sizeof(float)*1;
  memcpy(&dest->miny,&src->miny,sizeof(float)*1); 
  size += sizeof(float)*1;
  memcpy(&dest->maxy,&src->maxy,sizeof(float)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->segments_count,&src->segments_count,sizeof(uint32_t)*1); 
  if(src->segments != NULL && src->segments_count > 0)
  {
    if((dest->segments = malloc(src->segments_count*sizeof(player_segment_t))) == NULL)
      return(0);
  }
  else
    dest->segments = NULL;
  size += sizeof(player_segment_t)*src->segments_count;
  memcpy(dest->segments,src->segments,sizeof(player_segment_t)*src->segments_count); 
  return(size);
}
void player_map_data_vector_t_cleanup(const player_map_data_vector_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->segments); 
}
player_map_data_vector_t * player_map_data_vector_t_clone(const player_map_data_vector_t *msg)
{      
  player_map_data_vector_t * clone = malloc(sizeof(player_map_data_vector_t));
  if (clone)
    player_map_data_vector_t_copy(clone,msg);
  return clone;
}
void player_map_data_vector_t_free(player_map_data_vector_t *msg)
{      
  player_map_data_vector_t_cleanup(msg);
  free(msg);
}
unsigned int player_map_data_vector_t_sizeof(player_map_data_vector_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(float)*1; 
  size += sizeof(float)*1; 
  size += sizeof(float)*1; 
  size += sizeof(float)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_segment_t)*msg->segments_count; 
  return(size);
}

int xdr_player_aio_data_t (XDR* xdrs, player_aio_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->voltages_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->voltages = malloc(msg->voltages_count*sizeof(float))) == NULL)
      return(0);
  }
  {
    float* voltages_p = msg->voltages;
    if(xdr_array(xdrs, (char**)&voltages_p, &msg->voltages_count, msg->voltages_count, sizeof(float), (xdrproc_t)xdr_float) != 1)
      return(0);
  }
  return(1);
}
int 
player_aio_data_pack(void* buf, size_t buflen, player_aio_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_aio_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_aio_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_aio_data_t_copy(player_aio_data_t *dest, const player_aio_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->voltages_count,&src->voltages_count,sizeof(uint32_t)*1); 
  if(src->voltages != NULL && src->voltages_count > 0)
  {
    if((dest->voltages = malloc(src->voltages_count*sizeof(float))) == NULL)
      return(0);
  }
  else
    dest->voltages = NULL;
  size += sizeof(float)*src->voltages_count;
  memcpy(dest->voltages,src->voltages,sizeof(float)*src->voltages_count); 
  return(size);
}
void player_aio_data_t_cleanup(const player_aio_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->voltages); 
}
player_aio_data_t * player_aio_data_t_clone(const player_aio_data_t *msg)
{      
  player_aio_data_t * clone = malloc(sizeof(player_aio_data_t));
  if (clone)
    player_aio_data_t_copy(clone,msg);
  return clone;
}
void player_aio_data_t_free(player_aio_data_t *msg)
{      
  player_aio_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_aio_data_t_sizeof(player_aio_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(float)*msg->voltages_count; 
  return(size);
}

int xdr_player_aio_cmd_t (XDR* xdrs, player_aio_cmd_t * msg)
{   if(xdr_u_int(xdrs,&msg->id) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->voltage) != 1)
    return(0);
  return(1);
}
int 
player_aio_cmd_pack(void* buf, size_t buflen, player_aio_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_aio_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_aio_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_aio_cmd_t_copy(player_aio_cmd_t *dest, const player_aio_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_aio_cmd_t));
  return sizeof(player_aio_cmd_t);
} 
void player_aio_cmd_t_cleanup(const player_aio_cmd_t *msg)
{
} 
player_aio_cmd_t * player_aio_cmd_t_clone(const player_aio_cmd_t *msg)
{      
  player_aio_cmd_t * clone = malloc(sizeof(player_aio_cmd_t));
  if (clone)
    player_aio_cmd_t_copy(clone,msg);
  return clone;
}
void player_aio_cmd_t_free(player_aio_cmd_t *msg)
{      
  player_aio_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_aio_cmd_t_sizeof(player_aio_cmd_t *msg)
{
  return sizeof(player_aio_cmd_t);
} 

int xdr_player_planner_data_t (XDR* xdrs, player_planner_data_t * msg)
{   if(xdr_u_char(xdrs,&msg->valid) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->done) != 1)
    return(0);
  if(xdr_player_pose2d_t(xdrs,&msg->pos) != 1)
    return(0);
  if(xdr_player_pose2d_t(xdrs,&msg->goal) != 1)
    return(0);
  if(xdr_player_pose2d_t(xdrs,&msg->waypoint) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->waypoint_idx) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->waypoints_count) != 1)
    return(0);
  return(1);
}
int 
player_planner_data_pack(void* buf, size_t buflen, player_planner_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_planner_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_planner_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_planner_data_t_copy(player_planner_data_t *dest, const player_planner_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_planner_data_t));
  return sizeof(player_planner_data_t);
} 
void player_planner_data_t_cleanup(const player_planner_data_t *msg)
{
} 
player_planner_data_t * player_planner_data_t_clone(const player_planner_data_t *msg)
{      
  player_planner_data_t * clone = malloc(sizeof(player_planner_data_t));
  if (clone)
    player_planner_data_t_copy(clone,msg);
  return clone;
}
void player_planner_data_t_free(player_planner_data_t *msg)
{      
  player_planner_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_planner_data_t_sizeof(player_planner_data_t *msg)
{
  return sizeof(player_planner_data_t);
} 

int xdr_player_planner_cmd_t (XDR* xdrs, player_planner_cmd_t * msg)
{   if(xdr_player_pose2d_t(xdrs,&msg->goal) != 1)
    return(0);
  return(1);
}
int 
player_planner_cmd_pack(void* buf, size_t buflen, player_planner_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_planner_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_planner_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_planner_cmd_t_copy(player_planner_cmd_t *dest, const player_planner_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_planner_cmd_t));
  return sizeof(player_planner_cmd_t);
} 
void player_planner_cmd_t_cleanup(const player_planner_cmd_t *msg)
{
} 
player_planner_cmd_t * player_planner_cmd_t_clone(const player_planner_cmd_t *msg)
{      
  player_planner_cmd_t * clone = malloc(sizeof(player_planner_cmd_t));
  if (clone)
    player_planner_cmd_t_copy(clone,msg);
  return clone;
}
void player_planner_cmd_t_free(player_planner_cmd_t *msg)
{      
  player_planner_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_planner_cmd_t_sizeof(player_planner_cmd_t *msg)
{
  return sizeof(player_planner_cmd_t);
} 

int xdr_player_planner_waypoints_req_t (XDR* xdrs, player_planner_waypoints_req_t * msg)
{   if(xdr_u_int(xdrs,&msg->waypoints_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->waypoints = malloc(msg->waypoints_count*sizeof(player_pose2d_t))) == NULL)
      return(0);
  }
  {
    player_pose2d_t* waypoints_p = msg->waypoints;
    if(xdr_array(xdrs, (char**)&waypoints_p, &msg->waypoints_count, msg->waypoints_count, sizeof(player_pose2d_t), (xdrproc_t)xdr_player_pose2d_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_planner_waypoints_req_pack(void* buf, size_t buflen, player_planner_waypoints_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_planner_waypoints_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_planner_waypoints_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_planner_waypoints_req_t_copy(player_planner_waypoints_req_t *dest, const player_planner_waypoints_req_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->waypoints_count,&src->waypoints_count,sizeof(uint32_t)*1); 
  if(src->waypoints != NULL && src->waypoints_count > 0)
  {
    if((dest->waypoints = malloc(src->waypoints_count*sizeof(player_pose2d_t))) == NULL)
      return(0);
  }
  else
    dest->waypoints = NULL;
  size += sizeof(player_pose2d_t)*src->waypoints_count;
  memcpy(dest->waypoints,src->waypoints,sizeof(player_pose2d_t)*src->waypoints_count); 
  return(size);
}
void player_planner_waypoints_req_t_cleanup(const player_planner_waypoints_req_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->waypoints); 
}
player_planner_waypoints_req_t * player_planner_waypoints_req_t_clone(const player_planner_waypoints_req_t *msg)
{      
  player_planner_waypoints_req_t * clone = malloc(sizeof(player_planner_waypoints_req_t));
  if (clone)
    player_planner_waypoints_req_t_copy(clone,msg);
  return clone;
}
void player_planner_waypoints_req_t_free(player_planner_waypoints_req_t *msg)
{      
  player_planner_waypoints_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_planner_waypoints_req_t_sizeof(player_planner_waypoints_req_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_pose2d_t)*msg->waypoints_count; 
  return(size);
}

int xdr_player_planner_enable_req_t (XDR* xdrs, player_planner_enable_req_t * msg)
{   if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_planner_enable_req_pack(void* buf, size_t buflen, player_planner_enable_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_planner_enable_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_planner_enable_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_planner_enable_req_t_copy(player_planner_enable_req_t *dest, const player_planner_enable_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_planner_enable_req_t));
  return sizeof(player_planner_enable_req_t);
} 
void player_planner_enable_req_t_cleanup(const player_planner_enable_req_t *msg)
{
} 
player_planner_enable_req_t * player_planner_enable_req_t_clone(const player_planner_enable_req_t *msg)
{      
  player_planner_enable_req_t * clone = malloc(sizeof(player_planner_enable_req_t));
  if (clone)
    player_planner_enable_req_t_copy(clone,msg);
  return clone;
}
void player_planner_enable_req_t_free(player_planner_enable_req_t *msg)
{      
  player_planner_enable_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_planner_enable_req_t_sizeof(player_planner_enable_req_t *msg)
{
  return sizeof(player_planner_enable_req_t);
} 

int xdr_player_blinkenlight_data_t (XDR* xdrs, player_blinkenlight_data_t * msg)
{   if(xdr_u_char(xdrs,&msg->enable) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->period) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->dutycycle) != 1)
    return(0);
  if(xdr_player_color_t(xdrs,&msg->color) != 1)
    return(0);
  return(1);
}
int 
player_blinkenlight_data_pack(void* buf, size_t buflen, player_blinkenlight_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_blinkenlight_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_blinkenlight_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_blinkenlight_data_t_copy(player_blinkenlight_data_t *dest, const player_blinkenlight_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_blinkenlight_data_t));
  return sizeof(player_blinkenlight_data_t);
} 
void player_blinkenlight_data_t_cleanup(const player_blinkenlight_data_t *msg)
{
} 
player_blinkenlight_data_t * player_blinkenlight_data_t_clone(const player_blinkenlight_data_t *msg)
{      
  player_blinkenlight_data_t * clone = malloc(sizeof(player_blinkenlight_data_t));
  if (clone)
    player_blinkenlight_data_t_copy(clone,msg);
  return clone;
}
void player_blinkenlight_data_t_free(player_blinkenlight_data_t *msg)
{      
  player_blinkenlight_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_blinkenlight_data_t_sizeof(player_blinkenlight_data_t *msg)
{
  return sizeof(player_blinkenlight_data_t);
} 

int xdr_player_blinkenlight_cmd_t (XDR* xdrs, player_blinkenlight_cmd_t * msg)
{   if(xdr_u_short(xdrs,&msg->id) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->enable) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->period) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->dutycycle) != 1)
    return(0);
  if(xdr_player_color_t(xdrs,&msg->color) != 1)
    return(0);
  return(1);
}
int 
player_blinkenlight_cmd_pack(void* buf, size_t buflen, player_blinkenlight_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_blinkenlight_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_blinkenlight_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_blinkenlight_cmd_t_copy(player_blinkenlight_cmd_t *dest, const player_blinkenlight_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_blinkenlight_cmd_t));
  return sizeof(player_blinkenlight_cmd_t);
} 
void player_blinkenlight_cmd_t_cleanup(const player_blinkenlight_cmd_t *msg)
{
} 
player_blinkenlight_cmd_t * player_blinkenlight_cmd_t_clone(const player_blinkenlight_cmd_t *msg)
{      
  player_blinkenlight_cmd_t * clone = malloc(sizeof(player_blinkenlight_cmd_t));
  if (clone)
    player_blinkenlight_cmd_t_copy(clone,msg);
  return clone;
}
void player_blinkenlight_cmd_t_free(player_blinkenlight_cmd_t *msg)
{      
  player_blinkenlight_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_blinkenlight_cmd_t_sizeof(player_blinkenlight_cmd_t *msg)
{
  return sizeof(player_blinkenlight_cmd_t);
} 

int xdr_player_blinkenlight_cmd_power_t (XDR* xdrs, player_blinkenlight_cmd_power_t * msg)
{   if(xdr_u_short(xdrs,&msg->id) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->enable) != 1)
    return(0);
  return(1);
}
int 
player_blinkenlight_cmd_power_pack(void* buf, size_t buflen, player_blinkenlight_cmd_power_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_blinkenlight_cmd_power_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_blinkenlight_cmd_power_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_blinkenlight_cmd_power_t_copy(player_blinkenlight_cmd_power_t *dest, const player_blinkenlight_cmd_power_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_blinkenlight_cmd_power_t));
  return sizeof(player_blinkenlight_cmd_power_t);
} 
void player_blinkenlight_cmd_power_t_cleanup(const player_blinkenlight_cmd_power_t *msg)
{
} 
player_blinkenlight_cmd_power_t * player_blinkenlight_cmd_power_t_clone(const player_blinkenlight_cmd_power_t *msg)
{      
  player_blinkenlight_cmd_power_t * clone = malloc(sizeof(player_blinkenlight_cmd_power_t));
  if (clone)
    player_blinkenlight_cmd_power_t_copy(clone,msg);
  return clone;
}
void player_blinkenlight_cmd_power_t_free(player_blinkenlight_cmd_power_t *msg)
{      
  player_blinkenlight_cmd_power_t_cleanup(msg);
  free(msg);
}
unsigned int player_blinkenlight_cmd_power_t_sizeof(player_blinkenlight_cmd_power_t *msg)
{
  return sizeof(player_blinkenlight_cmd_power_t);
} 

int xdr_player_blinkenlight_cmd_color_t (XDR* xdrs, player_blinkenlight_cmd_color_t * msg)
{   if(xdr_u_short(xdrs,&msg->id) != 1)
    return(0);
  if(xdr_player_color_t(xdrs,&msg->color) != 1)
    return(0);
  return(1);
}
int 
player_blinkenlight_cmd_color_pack(void* buf, size_t buflen, player_blinkenlight_cmd_color_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_blinkenlight_cmd_color_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_blinkenlight_cmd_color_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_blinkenlight_cmd_color_t_copy(player_blinkenlight_cmd_color_t *dest, const player_blinkenlight_cmd_color_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_blinkenlight_cmd_color_t));
  return sizeof(player_blinkenlight_cmd_color_t);
} 
void player_blinkenlight_cmd_color_t_cleanup(const player_blinkenlight_cmd_color_t *msg)
{
} 
player_blinkenlight_cmd_color_t * player_blinkenlight_cmd_color_t_clone(const player_blinkenlight_cmd_color_t *msg)
{      
  player_blinkenlight_cmd_color_t * clone = malloc(sizeof(player_blinkenlight_cmd_color_t));
  if (clone)
    player_blinkenlight_cmd_color_t_copy(clone,msg);
  return clone;
}
void player_blinkenlight_cmd_color_t_free(player_blinkenlight_cmd_color_t *msg)
{      
  player_blinkenlight_cmd_color_t_cleanup(msg);
  free(msg);
}
unsigned int player_blinkenlight_cmd_color_t_sizeof(player_blinkenlight_cmd_color_t *msg)
{
  return sizeof(player_blinkenlight_cmd_color_t);
} 

int xdr_player_blinkenlight_cmd_flash_t (XDR* xdrs, player_blinkenlight_cmd_flash_t * msg)
{   if(xdr_u_short(xdrs,&msg->id) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->period) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->dutycycle) != 1)
    return(0);
  return(1);
}
int 
player_blinkenlight_cmd_flash_pack(void* buf, size_t buflen, player_blinkenlight_cmd_flash_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_blinkenlight_cmd_flash_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_blinkenlight_cmd_flash_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_blinkenlight_cmd_flash_t_copy(player_blinkenlight_cmd_flash_t *dest, const player_blinkenlight_cmd_flash_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_blinkenlight_cmd_flash_t));
  return sizeof(player_blinkenlight_cmd_flash_t);
} 
void player_blinkenlight_cmd_flash_t_cleanup(const player_blinkenlight_cmd_flash_t *msg)
{
} 
player_blinkenlight_cmd_flash_t * player_blinkenlight_cmd_flash_t_clone(const player_blinkenlight_cmd_flash_t *msg)
{      
  player_blinkenlight_cmd_flash_t * clone = malloc(sizeof(player_blinkenlight_cmd_flash_t));
  if (clone)
    player_blinkenlight_cmd_flash_t_copy(clone,msg);
  return clone;
}
void player_blinkenlight_cmd_flash_t_free(player_blinkenlight_cmd_flash_t *msg)
{      
  player_blinkenlight_cmd_flash_t_cleanup(msg);
  free(msg);
}
unsigned int player_blinkenlight_cmd_flash_t_sizeof(player_blinkenlight_cmd_flash_t *msg)
{
  return sizeof(player_blinkenlight_cmd_flash_t);
} 

int xdr_player_laser_data_t (XDR* xdrs, player_laser_data_t * msg)
{   if(xdr_float(xdrs,&msg->min_angle) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->max_angle) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->resolution) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->max_range) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->ranges_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->ranges = malloc(msg->ranges_count*sizeof(float))) == NULL)
      return(0);
  }
  {
    float* ranges_p = msg->ranges;
    if(xdr_array(xdrs, (char**)&ranges_p, &msg->ranges_count, msg->ranges_count, sizeof(float), (xdrproc_t)xdr_float) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->intensity_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->intensity = malloc(msg->intensity_count*sizeof(uint8_t))) == NULL)
      return(0);
  }
  {
    uint8_t* intensity_p = msg->intensity;
    if(xdr_bytes(xdrs, (char**)&intensity_p, &msg->intensity_count, msg->intensity_count) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->id) != 1)
    return(0);
  return(1);
}
int 
player_laser_data_pack(void* buf, size_t buflen, player_laser_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_laser_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_laser_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_laser_data_t_copy(player_laser_data_t *dest, const player_laser_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(float)*1;
  memcpy(&dest->min_angle,&src->min_angle,sizeof(float)*1); 
  size += sizeof(float)*1;
  memcpy(&dest->max_angle,&src->max_angle,sizeof(float)*1); 
  size += sizeof(float)*1;
  memcpy(&dest->resolution,&src->resolution,sizeof(float)*1); 
  size += sizeof(float)*1;
  memcpy(&dest->max_range,&src->max_range,sizeof(float)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->ranges_count,&src->ranges_count,sizeof(uint32_t)*1); 
  if(src->ranges != NULL && src->ranges_count > 0)
  {
    if((dest->ranges = malloc(src->ranges_count*sizeof(float))) == NULL)
      return(0);
  }
  else
    dest->ranges = NULL;
  size += sizeof(float)*src->ranges_count;
  memcpy(dest->ranges,src->ranges,sizeof(float)*src->ranges_count); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->intensity_count,&src->intensity_count,sizeof(uint32_t)*1); 
  if(src->intensity != NULL && src->intensity_count > 0)
  {
    if((dest->intensity = malloc(src->intensity_count*sizeof(uint8_t))) == NULL)
      return(0);
  }
  else
    dest->intensity = NULL;
  size += sizeof(uint8_t)*src->intensity_count;
  memcpy(dest->intensity,src->intensity,sizeof(uint8_t)*src->intensity_count); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->id,&src->id,sizeof(uint32_t)*1); 
  return(size);
}
void player_laser_data_t_cleanup(const player_laser_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->ranges); 
  free(msg->intensity); 
}
player_laser_data_t * player_laser_data_t_clone(const player_laser_data_t *msg)
{      
  player_laser_data_t * clone = malloc(sizeof(player_laser_data_t));
  if (clone)
    player_laser_data_t_copy(clone,msg);
  return clone;
}
void player_laser_data_t_free(player_laser_data_t *msg)
{      
  player_laser_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_laser_data_t_sizeof(player_laser_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(float)*1; 
  size += sizeof(float)*1; 
  size += sizeof(float)*1; 
  size += sizeof(float)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(float)*msg->ranges_count; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint8_t)*msg->intensity_count; 
  size += sizeof(uint32_t)*1; 
  return(size);
}

int xdr_player_laser_data_scanpose_t (XDR* xdrs, player_laser_data_scanpose_t * msg)
{   if(xdr_player_laser_data_t(xdrs,&msg->scan) != 1)
    return(0);
  if(xdr_player_pose2d_t(xdrs,&msg->pose) != 1)
    return(0);
  return(1);
}
int 
player_laser_data_scanpose_pack(void* buf, size_t buflen, player_laser_data_scanpose_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_laser_data_scanpose_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_laser_data_scanpose_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_laser_data_scanpose_t_copy(player_laser_data_scanpose_t *dest, const player_laser_data_scanpose_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  {size += player_laser_data_t_copy(&dest->scan, &src->scan);}
  size += sizeof(player_pose2d_t)*1;
  memcpy(&dest->pose,&src->pose,sizeof(player_pose2d_t)*1); 
  return(size);
}
void player_laser_data_scanpose_t_cleanup(const player_laser_data_scanpose_t *msg)
{      
  
  if(msg == NULL)
    return;
  player_laser_data_t_cleanup(&msg->scan); 
}
player_laser_data_scanpose_t * player_laser_data_scanpose_t_clone(const player_laser_data_scanpose_t *msg)
{      
  player_laser_data_scanpose_t * clone = malloc(sizeof(player_laser_data_scanpose_t));
  if (clone)
    player_laser_data_scanpose_t_copy(clone,msg);
  return clone;
}
void player_laser_data_scanpose_t_free(player_laser_data_scanpose_t *msg)
{      
  player_laser_data_scanpose_t_cleanup(msg);
  free(msg);
}
unsigned int player_laser_data_scanpose_t_sizeof(player_laser_data_scanpose_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  {size += player_laser_data_t_sizeof(&msg->scan);}
  size += sizeof(player_pose2d_t)*1; 
  return(size);
}

int xdr_player_laser_geom_t (XDR* xdrs, player_laser_geom_t * msg)
{   if(xdr_player_pose3d_t(xdrs,&msg->pose) != 1)
    return(0);
  if(xdr_player_bbox3d_t(xdrs,&msg->size) != 1)
    return(0);
  return(1);
}
int 
player_laser_geom_pack(void* buf, size_t buflen, player_laser_geom_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_laser_geom_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_laser_geom_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_laser_geom_t_copy(player_laser_geom_t *dest, const player_laser_geom_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_laser_geom_t));
  return sizeof(player_laser_geom_t);
} 
void player_laser_geom_t_cleanup(const player_laser_geom_t *msg)
{
} 
player_laser_geom_t * player_laser_geom_t_clone(const player_laser_geom_t *msg)
{      
  player_laser_geom_t * clone = malloc(sizeof(player_laser_geom_t));
  if (clone)
    player_laser_geom_t_copy(clone,msg);
  return clone;
}
void player_laser_geom_t_free(player_laser_geom_t *msg)
{      
  player_laser_geom_t_cleanup(msg);
  free(msg);
}
unsigned int player_laser_geom_t_sizeof(player_laser_geom_t *msg)
{
  return sizeof(player_laser_geom_t);
} 

int xdr_player_laser_config_t (XDR* xdrs, player_laser_config_t * msg)
{   if(xdr_float(xdrs,&msg->min_angle) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->max_angle) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->resolution) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->max_range) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->range_res) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->intensity) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->scanning_frequency) != 1)
    return(0);
  return(1);
}
int 
player_laser_config_pack(void* buf, size_t buflen, player_laser_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_laser_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_laser_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_laser_config_t_copy(player_laser_config_t *dest, const player_laser_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_laser_config_t));
  return sizeof(player_laser_config_t);
} 
void player_laser_config_t_cleanup(const player_laser_config_t *msg)
{
} 
player_laser_config_t * player_laser_config_t_clone(const player_laser_config_t *msg)
{      
  player_laser_config_t * clone = malloc(sizeof(player_laser_config_t));
  if (clone)
    player_laser_config_t_copy(clone,msg);
  return clone;
}
void player_laser_config_t_free(player_laser_config_t *msg)
{      
  player_laser_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_laser_config_t_sizeof(player_laser_config_t *msg)
{
  return sizeof(player_laser_config_t);
} 

int xdr_player_laser_power_config_t (XDR* xdrs, player_laser_power_config_t * msg)
{   if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_laser_power_config_pack(void* buf, size_t buflen, player_laser_power_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_laser_power_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_laser_power_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_laser_power_config_t_copy(player_laser_power_config_t *dest, const player_laser_power_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_laser_power_config_t));
  return sizeof(player_laser_power_config_t);
} 
void player_laser_power_config_t_cleanup(const player_laser_power_config_t *msg)
{
} 
player_laser_power_config_t * player_laser_power_config_t_clone(const player_laser_power_config_t *msg)
{      
  player_laser_power_config_t * clone = malloc(sizeof(player_laser_power_config_t));
  if (clone)
    player_laser_power_config_t_copy(clone,msg);
  return clone;
}
void player_laser_power_config_t_free(player_laser_power_config_t *msg)
{      
  player_laser_power_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_laser_power_config_t_sizeof(player_laser_power_config_t *msg)
{
  return sizeof(player_laser_power_config_t);
} 

int xdr_player_laser_get_id_config_t (XDR* xdrs, player_laser_get_id_config_t * msg)
{   if(xdr_u_int(xdrs,&msg->serial_number) != 1)
    return(0);
  return(1);
}
int 
player_laser_get_id_config_pack(void* buf, size_t buflen, player_laser_get_id_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_laser_get_id_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_laser_get_id_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_laser_get_id_config_t_copy(player_laser_get_id_config_t *dest, const player_laser_get_id_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_laser_get_id_config_t));
  return sizeof(player_laser_get_id_config_t);
} 
void player_laser_get_id_config_t_cleanup(const player_laser_get_id_config_t *msg)
{
} 
player_laser_get_id_config_t * player_laser_get_id_config_t_clone(const player_laser_get_id_config_t *msg)
{      
  player_laser_get_id_config_t * clone = malloc(sizeof(player_laser_get_id_config_t));
  if (clone)
    player_laser_get_id_config_t_copy(clone,msg);
  return clone;
}
void player_laser_get_id_config_t_free(player_laser_get_id_config_t *msg)
{      
  player_laser_get_id_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_laser_get_id_config_t_sizeof(player_laser_get_id_config_t *msg)
{
  return sizeof(player_laser_get_id_config_t);
} 

int xdr_player_laser_set_filter_config_t (XDR* xdrs, player_laser_set_filter_config_t * msg)
{   if(xdr_u_char(xdrs,&msg->filter_type) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->parameters_count) != 1)
    return(0);
  {
    float* parameters_p = msg->parameters;
    if(xdr_array(xdrs, (char**)&parameters_p, &msg->parameters_count, PLAYER_LASER_MAX_FILTER_PARAMS, sizeof(float), (xdrproc_t)xdr_float) != 1)
      return(0);
  }
  return(1);
}
int 
player_laser_set_filter_config_pack(void* buf, size_t buflen, player_laser_set_filter_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_laser_set_filter_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_laser_set_filter_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_laser_set_filter_config_t_copy(player_laser_set_filter_config_t *dest, const player_laser_set_filter_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_laser_set_filter_config_t));
  return sizeof(player_laser_set_filter_config_t);
} 
void player_laser_set_filter_config_t_cleanup(const player_laser_set_filter_config_t *msg)
{
} 
player_laser_set_filter_config_t * player_laser_set_filter_config_t_clone(const player_laser_set_filter_config_t *msg)
{      
  player_laser_set_filter_config_t * clone = malloc(sizeof(player_laser_set_filter_config_t));
  if (clone)
    player_laser_set_filter_config_t_copy(clone,msg);
  return clone;
}
void player_laser_set_filter_config_t_free(player_laser_set_filter_config_t *msg)
{      
  player_laser_set_filter_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_laser_set_filter_config_t_sizeof(player_laser_set_filter_config_t *msg)
{
  return sizeof(player_laser_set_filter_config_t);
} 

int xdr_player_limb_data_t (XDR* xdrs, player_limb_data_t * msg)
{   if(xdr_player_point_3d_t(xdrs,&msg->position) != 1)
    return(0);
  if(xdr_player_point_3d_t(xdrs,&msg->approach) != 1)
    return(0);
  if(xdr_player_point_3d_t(xdrs,&msg->orientation) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_limb_data_pack(void* buf, size_t buflen, player_limb_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_limb_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_limb_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_limb_data_t_copy(player_limb_data_t *dest, const player_limb_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_limb_data_t));
  return sizeof(player_limb_data_t);
} 
void player_limb_data_t_cleanup(const player_limb_data_t *msg)
{
} 
player_limb_data_t * player_limb_data_t_clone(const player_limb_data_t *msg)
{      
  player_limb_data_t * clone = malloc(sizeof(player_limb_data_t));
  if (clone)
    player_limb_data_t_copy(clone,msg);
  return clone;
}
void player_limb_data_t_free(player_limb_data_t *msg)
{      
  player_limb_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_limb_data_t_sizeof(player_limb_data_t *msg)
{
  return sizeof(player_limb_data_t);
} 

int xdr_player_limb_setpose_cmd_t (XDR* xdrs, player_limb_setpose_cmd_t * msg)
{   if(xdr_player_point_3d_t(xdrs,&msg->position) != 1)
    return(0);
  if(xdr_player_point_3d_t(xdrs,&msg->approach) != 1)
    return(0);
  if(xdr_player_point_3d_t(xdrs,&msg->orientation) != 1)
    return(0);
  return(1);
}
int 
player_limb_setpose_cmd_pack(void* buf, size_t buflen, player_limb_setpose_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_limb_setpose_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_limb_setpose_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_limb_setpose_cmd_t_copy(player_limb_setpose_cmd_t *dest, const player_limb_setpose_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_limb_setpose_cmd_t));
  return sizeof(player_limb_setpose_cmd_t);
} 
void player_limb_setpose_cmd_t_cleanup(const player_limb_setpose_cmd_t *msg)
{
} 
player_limb_setpose_cmd_t * player_limb_setpose_cmd_t_clone(const player_limb_setpose_cmd_t *msg)
{      
  player_limb_setpose_cmd_t * clone = malloc(sizeof(player_limb_setpose_cmd_t));
  if (clone)
    player_limb_setpose_cmd_t_copy(clone,msg);
  return clone;
}
void player_limb_setpose_cmd_t_free(player_limb_setpose_cmd_t *msg)
{      
  player_limb_setpose_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_limb_setpose_cmd_t_sizeof(player_limb_setpose_cmd_t *msg)
{
  return sizeof(player_limb_setpose_cmd_t);
} 

int xdr_player_limb_setposition_cmd_t (XDR* xdrs, player_limb_setposition_cmd_t * msg)
{   if(xdr_player_point_3d_t(xdrs,&msg->position) != 1)
    return(0);
  return(1);
}
int 
player_limb_setposition_cmd_pack(void* buf, size_t buflen, player_limb_setposition_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_limb_setposition_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_limb_setposition_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_limb_setposition_cmd_t_copy(player_limb_setposition_cmd_t *dest, const player_limb_setposition_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_limb_setposition_cmd_t));
  return sizeof(player_limb_setposition_cmd_t);
} 
void player_limb_setposition_cmd_t_cleanup(const player_limb_setposition_cmd_t *msg)
{
} 
player_limb_setposition_cmd_t * player_limb_setposition_cmd_t_clone(const player_limb_setposition_cmd_t *msg)
{      
  player_limb_setposition_cmd_t * clone = malloc(sizeof(player_limb_setposition_cmd_t));
  if (clone)
    player_limb_setposition_cmd_t_copy(clone,msg);
  return clone;
}
void player_limb_setposition_cmd_t_free(player_limb_setposition_cmd_t *msg)
{      
  player_limb_setposition_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_limb_setposition_cmd_t_sizeof(player_limb_setposition_cmd_t *msg)
{
  return sizeof(player_limb_setposition_cmd_t);
} 

int xdr_player_limb_vecmove_cmd_t (XDR* xdrs, player_limb_vecmove_cmd_t * msg)
{   if(xdr_player_point_3d_t(xdrs,&msg->direction) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->length) != 1)
    return(0);
  return(1);
}
int 
player_limb_vecmove_cmd_pack(void* buf, size_t buflen, player_limb_vecmove_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_limb_vecmove_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_limb_vecmove_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_limb_vecmove_cmd_t_copy(player_limb_vecmove_cmd_t *dest, const player_limb_vecmove_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_limb_vecmove_cmd_t));
  return sizeof(player_limb_vecmove_cmd_t);
} 
void player_limb_vecmove_cmd_t_cleanup(const player_limb_vecmove_cmd_t *msg)
{
} 
player_limb_vecmove_cmd_t * player_limb_vecmove_cmd_t_clone(const player_limb_vecmove_cmd_t *msg)
{      
  player_limb_vecmove_cmd_t * clone = malloc(sizeof(player_limb_vecmove_cmd_t));
  if (clone)
    player_limb_vecmove_cmd_t_copy(clone,msg);
  return clone;
}
void player_limb_vecmove_cmd_t_free(player_limb_vecmove_cmd_t *msg)
{      
  player_limb_vecmove_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_limb_vecmove_cmd_t_sizeof(player_limb_vecmove_cmd_t *msg)
{
  return sizeof(player_limb_vecmove_cmd_t);
} 

int xdr_player_limb_power_req_t (XDR* xdrs, player_limb_power_req_t * msg)
{   if(xdr_u_char(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_limb_power_req_pack(void* buf, size_t buflen, player_limb_power_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_limb_power_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_limb_power_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_limb_power_req_t_copy(player_limb_power_req_t *dest, const player_limb_power_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_limb_power_req_t));
  return sizeof(player_limb_power_req_t);
} 
void player_limb_power_req_t_cleanup(const player_limb_power_req_t *msg)
{
} 
player_limb_power_req_t * player_limb_power_req_t_clone(const player_limb_power_req_t *msg)
{      
  player_limb_power_req_t * clone = malloc(sizeof(player_limb_power_req_t));
  if (clone)
    player_limb_power_req_t_copy(clone,msg);
  return clone;
}
void player_limb_power_req_t_free(player_limb_power_req_t *msg)
{      
  player_limb_power_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_limb_power_req_t_sizeof(player_limb_power_req_t *msg)
{
  return sizeof(player_limb_power_req_t);
} 

int xdr_player_limb_brakes_req_t (XDR* xdrs, player_limb_brakes_req_t * msg)
{   if(xdr_u_char(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_limb_brakes_req_pack(void* buf, size_t buflen, player_limb_brakes_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_limb_brakes_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_limb_brakes_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_limb_brakes_req_t_copy(player_limb_brakes_req_t *dest, const player_limb_brakes_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_limb_brakes_req_t));
  return sizeof(player_limb_brakes_req_t);
} 
void player_limb_brakes_req_t_cleanup(const player_limb_brakes_req_t *msg)
{
} 
player_limb_brakes_req_t * player_limb_brakes_req_t_clone(const player_limb_brakes_req_t *msg)
{      
  player_limb_brakes_req_t * clone = malloc(sizeof(player_limb_brakes_req_t));
  if (clone)
    player_limb_brakes_req_t_copy(clone,msg);
  return clone;
}
void player_limb_brakes_req_t_free(player_limb_brakes_req_t *msg)
{      
  player_limb_brakes_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_limb_brakes_req_t_sizeof(player_limb_brakes_req_t *msg)
{
  return sizeof(player_limb_brakes_req_t);
} 

int xdr_player_limb_geom_req_t (XDR* xdrs, player_limb_geom_req_t * msg)
{   if(xdr_player_point_3d_t(xdrs,&msg->basePos) != 1)
    return(0);
  return(1);
}
int 
player_limb_geom_req_pack(void* buf, size_t buflen, player_limb_geom_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_limb_geom_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_limb_geom_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_limb_geom_req_t_copy(player_limb_geom_req_t *dest, const player_limb_geom_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_limb_geom_req_t));
  return sizeof(player_limb_geom_req_t);
} 
void player_limb_geom_req_t_cleanup(const player_limb_geom_req_t *msg)
{
} 
player_limb_geom_req_t * player_limb_geom_req_t_clone(const player_limb_geom_req_t *msg)
{      
  player_limb_geom_req_t * clone = malloc(sizeof(player_limb_geom_req_t));
  if (clone)
    player_limb_geom_req_t_copy(clone,msg);
  return clone;
}
void player_limb_geom_req_t_free(player_limb_geom_req_t *msg)
{      
  player_limb_geom_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_limb_geom_req_t_sizeof(player_limb_geom_req_t *msg)
{
  return sizeof(player_limb_geom_req_t);
} 

int xdr_player_limb_speed_req_t (XDR* xdrs, player_limb_speed_req_t * msg)
{   if(xdr_float(xdrs,&msg->speed) != 1)
    return(0);
  return(1);
}
int 
player_limb_speed_req_pack(void* buf, size_t buflen, player_limb_speed_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_limb_speed_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_limb_speed_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_limb_speed_req_t_copy(player_limb_speed_req_t *dest, const player_limb_speed_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_limb_speed_req_t));
  return sizeof(player_limb_speed_req_t);
} 
void player_limb_speed_req_t_cleanup(const player_limb_speed_req_t *msg)
{
} 
player_limb_speed_req_t * player_limb_speed_req_t_clone(const player_limb_speed_req_t *msg)
{      
  player_limb_speed_req_t * clone = malloc(sizeof(player_limb_speed_req_t));
  if (clone)
    player_limb_speed_req_t_copy(clone,msg);
  return clone;
}
void player_limb_speed_req_t_free(player_limb_speed_req_t *msg)
{      
  player_limb_speed_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_limb_speed_req_t_sizeof(player_limb_speed_req_t *msg)
{
  return sizeof(player_limb_speed_req_t);
} 

int xdr_player_actarray_actuator_t (XDR* xdrs, player_actarray_actuator_t * msg)
{   if(xdr_float(xdrs,&msg->position) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->speed) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->acceleration) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->current) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_actarray_actuator_pack(void* buf, size_t buflen, player_actarray_actuator_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_actuator_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_actuator_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_actuator_t_copy(player_actarray_actuator_t *dest, const player_actarray_actuator_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_actarray_actuator_t));
  return sizeof(player_actarray_actuator_t);
} 
void player_actarray_actuator_t_cleanup(const player_actarray_actuator_t *msg)
{
} 
player_actarray_actuator_t * player_actarray_actuator_t_clone(const player_actarray_actuator_t *msg)
{      
  player_actarray_actuator_t * clone = malloc(sizeof(player_actarray_actuator_t));
  if (clone)
    player_actarray_actuator_t_copy(clone,msg);
  return clone;
}
void player_actarray_actuator_t_free(player_actarray_actuator_t *msg)
{      
  player_actarray_actuator_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_actuator_t_sizeof(player_actarray_actuator_t *msg)
{
  return sizeof(player_actarray_actuator_t);
} 

int xdr_player_actarray_data_t (XDR* xdrs, player_actarray_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->actuators_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->actuators = malloc(msg->actuators_count*sizeof(player_actarray_actuator_t))) == NULL)
      return(0);
  }
  {
    player_actarray_actuator_t* actuators_p = msg->actuators;
    if(xdr_array(xdrs, (char**)&actuators_p, &msg->actuators_count, msg->actuators_count, sizeof(player_actarray_actuator_t), (xdrproc_t)xdr_player_actarray_actuator_t) != 1)
      return(0);
  }
  if(xdr_u_char(xdrs,&msg->motor_state) != 1)
    return(0);
  return(1);
}
int 
player_actarray_data_pack(void* buf, size_t buflen, player_actarray_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_data_t_copy(player_actarray_data_t *dest, const player_actarray_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->actuators_count,&src->actuators_count,sizeof(uint32_t)*1); 
  if(src->actuators != NULL && src->actuators_count > 0)
  {
    if((dest->actuators = malloc(src->actuators_count*sizeof(player_actarray_actuator_t))) == NULL)
      return(0);
  }
  else
    dest->actuators = NULL;
  size += sizeof(player_actarray_actuator_t)*src->actuators_count;
  memcpy(dest->actuators,src->actuators,sizeof(player_actarray_actuator_t)*src->actuators_count); 
  size += sizeof(uint8_t)*1;
  memcpy(&dest->motor_state,&src->motor_state,sizeof(uint8_t)*1); 
  return(size);
}
void player_actarray_data_t_cleanup(const player_actarray_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->actuators); 
}
player_actarray_data_t * player_actarray_data_t_clone(const player_actarray_data_t *msg)
{      
  player_actarray_data_t * clone = malloc(sizeof(player_actarray_data_t));
  if (clone)
    player_actarray_data_t_copy(clone,msg);
  return clone;
}
void player_actarray_data_t_free(player_actarray_data_t *msg)
{      
  player_actarray_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_data_t_sizeof(player_actarray_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_actarray_actuator_t)*msg->actuators_count; 
  size += sizeof(uint8_t)*1; 
  return(size);
}

int xdr_player_actarray_actuatorgeom_t (XDR* xdrs, player_actarray_actuatorgeom_t * msg)
{   if(xdr_u_char(xdrs,&msg->type) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->length) != 1)
    return(0);
  if(xdr_player_orientation_3d_t(xdrs,&msg->orientation) != 1)
    return(0);
  if(xdr_player_point_3d_t(xdrs,&msg->axis) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->min) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->centre) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->max) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->home) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->config_speed) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->hasbrakes) != 1)
    return(0);
  return(1);
}
int 
player_actarray_actuatorgeom_pack(void* buf, size_t buflen, player_actarray_actuatorgeom_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_actuatorgeom_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_actuatorgeom_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_actuatorgeom_t_copy(player_actarray_actuatorgeom_t *dest, const player_actarray_actuatorgeom_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_actarray_actuatorgeom_t));
  return sizeof(player_actarray_actuatorgeom_t);
} 
void player_actarray_actuatorgeom_t_cleanup(const player_actarray_actuatorgeom_t *msg)
{
} 
player_actarray_actuatorgeom_t * player_actarray_actuatorgeom_t_clone(const player_actarray_actuatorgeom_t *msg)
{      
  player_actarray_actuatorgeom_t * clone = malloc(sizeof(player_actarray_actuatorgeom_t));
  if (clone)
    player_actarray_actuatorgeom_t_copy(clone,msg);
  return clone;
}
void player_actarray_actuatorgeom_t_free(player_actarray_actuatorgeom_t *msg)
{      
  player_actarray_actuatorgeom_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_actuatorgeom_t_sizeof(player_actarray_actuatorgeom_t *msg)
{
  return sizeof(player_actarray_actuatorgeom_t);
} 

int xdr_player_actarray_geom_t (XDR* xdrs, player_actarray_geom_t * msg)
{   if(xdr_u_int(xdrs,&msg->actuators_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->actuators = malloc(msg->actuators_count*sizeof(player_actarray_actuatorgeom_t))) == NULL)
      return(0);
  }
  {
    player_actarray_actuatorgeom_t* actuators_p = msg->actuators;
    if(xdr_array(xdrs, (char**)&actuators_p, &msg->actuators_count, msg->actuators_count, sizeof(player_actarray_actuatorgeom_t), (xdrproc_t)xdr_player_actarray_actuatorgeom_t) != 1)
      return(0);
  }
  if(xdr_player_point_3d_t(xdrs,&msg->base_pos) != 1)
    return(0);
  if(xdr_player_orientation_3d_t(xdrs,&msg->base_orientation) != 1)
    return(0);
  return(1);
}
int 
player_actarray_geom_pack(void* buf, size_t buflen, player_actarray_geom_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_geom_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_geom_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_geom_t_copy(player_actarray_geom_t *dest, const player_actarray_geom_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->actuators_count,&src->actuators_count,sizeof(uint32_t)*1); 
  if(src->actuators != NULL && src->actuators_count > 0)
  {
    if((dest->actuators = malloc(src->actuators_count*sizeof(player_actarray_actuatorgeom_t))) == NULL)
      return(0);
  }
  else
    dest->actuators = NULL;
  size += sizeof(player_actarray_actuatorgeom_t)*src->actuators_count;
  memcpy(dest->actuators,src->actuators,sizeof(player_actarray_actuatorgeom_t)*src->actuators_count); 
  size += sizeof(player_point_3d_t)*1;
  memcpy(&dest->base_pos,&src->base_pos,sizeof(player_point_3d_t)*1); 
  size += sizeof(player_orientation_3d_t)*1;
  memcpy(&dest->base_orientation,&src->base_orientation,sizeof(player_orientation_3d_t)*1); 
  return(size);
}
void player_actarray_geom_t_cleanup(const player_actarray_geom_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->actuators); 
}
player_actarray_geom_t * player_actarray_geom_t_clone(const player_actarray_geom_t *msg)
{      
  player_actarray_geom_t * clone = malloc(sizeof(player_actarray_geom_t));
  if (clone)
    player_actarray_geom_t_copy(clone,msg);
  return clone;
}
void player_actarray_geom_t_free(player_actarray_geom_t *msg)
{      
  player_actarray_geom_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_geom_t_sizeof(player_actarray_geom_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_actarray_actuatorgeom_t)*msg->actuators_count; 
  size += sizeof(player_point_3d_t)*1; 
  size += sizeof(player_orientation_3d_t)*1; 
  return(size);
}

int xdr_player_actarray_position_cmd_t (XDR* xdrs, player_actarray_position_cmd_t * msg)
{   if(xdr_int(xdrs,&msg->joint) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->position) != 1)
    return(0);
  return(1);
}
int 
player_actarray_position_cmd_pack(void* buf, size_t buflen, player_actarray_position_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_position_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_position_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_position_cmd_t_copy(player_actarray_position_cmd_t *dest, const player_actarray_position_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_actarray_position_cmd_t));
  return sizeof(player_actarray_position_cmd_t);
} 
void player_actarray_position_cmd_t_cleanup(const player_actarray_position_cmd_t *msg)
{
} 
player_actarray_position_cmd_t * player_actarray_position_cmd_t_clone(const player_actarray_position_cmd_t *msg)
{      
  player_actarray_position_cmd_t * clone = malloc(sizeof(player_actarray_position_cmd_t));
  if (clone)
    player_actarray_position_cmd_t_copy(clone,msg);
  return clone;
}
void player_actarray_position_cmd_t_free(player_actarray_position_cmd_t *msg)
{      
  player_actarray_position_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_position_cmd_t_sizeof(player_actarray_position_cmd_t *msg)
{
  return sizeof(player_actarray_position_cmd_t);
} 

int xdr_player_actarray_multi_position_cmd_t (XDR* xdrs, player_actarray_multi_position_cmd_t * msg)
{   if(xdr_u_int(xdrs,&msg->positions_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->positions = malloc(msg->positions_count*sizeof(float))) == NULL)
      return(0);
  }
  {
    float* positions_p = msg->positions;
    if(xdr_array(xdrs, (char**)&positions_p, &msg->positions_count, msg->positions_count, sizeof(float), (xdrproc_t)xdr_float) != 1)
      return(0);
  }
  return(1);
}
int 
player_actarray_multi_position_cmd_pack(void* buf, size_t buflen, player_actarray_multi_position_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_multi_position_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_multi_position_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_multi_position_cmd_t_copy(player_actarray_multi_position_cmd_t *dest, const player_actarray_multi_position_cmd_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->positions_count,&src->positions_count,sizeof(uint32_t)*1); 
  if(src->positions != NULL && src->positions_count > 0)
  {
    if((dest->positions = malloc(src->positions_count*sizeof(float))) == NULL)
      return(0);
  }
  else
    dest->positions = NULL;
  size += sizeof(float)*src->positions_count;
  memcpy(dest->positions,src->positions,sizeof(float)*src->positions_count); 
  return(size);
}
void player_actarray_multi_position_cmd_t_cleanup(const player_actarray_multi_position_cmd_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->positions); 
}
player_actarray_multi_position_cmd_t * player_actarray_multi_position_cmd_t_clone(const player_actarray_multi_position_cmd_t *msg)
{      
  player_actarray_multi_position_cmd_t * clone = malloc(sizeof(player_actarray_multi_position_cmd_t));
  if (clone)
    player_actarray_multi_position_cmd_t_copy(clone,msg);
  return clone;
}
void player_actarray_multi_position_cmd_t_free(player_actarray_multi_position_cmd_t *msg)
{      
  player_actarray_multi_position_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_multi_position_cmd_t_sizeof(player_actarray_multi_position_cmd_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(float)*msg->positions_count; 
  return(size);
}

int xdr_player_actarray_speed_cmd_t (XDR* xdrs, player_actarray_speed_cmd_t * msg)
{   if(xdr_int(xdrs,&msg->joint) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->speed) != 1)
    return(0);
  return(1);
}
int 
player_actarray_speed_cmd_pack(void* buf, size_t buflen, player_actarray_speed_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_speed_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_speed_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_speed_cmd_t_copy(player_actarray_speed_cmd_t *dest, const player_actarray_speed_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_actarray_speed_cmd_t));
  return sizeof(player_actarray_speed_cmd_t);
} 
void player_actarray_speed_cmd_t_cleanup(const player_actarray_speed_cmd_t *msg)
{
} 
player_actarray_speed_cmd_t * player_actarray_speed_cmd_t_clone(const player_actarray_speed_cmd_t *msg)
{      
  player_actarray_speed_cmd_t * clone = malloc(sizeof(player_actarray_speed_cmd_t));
  if (clone)
    player_actarray_speed_cmd_t_copy(clone,msg);
  return clone;
}
void player_actarray_speed_cmd_t_free(player_actarray_speed_cmd_t *msg)
{      
  player_actarray_speed_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_speed_cmd_t_sizeof(player_actarray_speed_cmd_t *msg)
{
  return sizeof(player_actarray_speed_cmd_t);
} 

int xdr_player_actarray_multi_speed_cmd_t (XDR* xdrs, player_actarray_multi_speed_cmd_t * msg)
{   if(xdr_u_int(xdrs,&msg->speeds_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->speeds = malloc(msg->speeds_count*sizeof(float))) == NULL)
      return(0);
  }
  {
    float* speeds_p = msg->speeds;
    if(xdr_array(xdrs, (char**)&speeds_p, &msg->speeds_count, msg->speeds_count, sizeof(float), (xdrproc_t)xdr_float) != 1)
      return(0);
  }
  return(1);
}
int 
player_actarray_multi_speed_cmd_pack(void* buf, size_t buflen, player_actarray_multi_speed_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_multi_speed_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_multi_speed_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_multi_speed_cmd_t_copy(player_actarray_multi_speed_cmd_t *dest, const player_actarray_multi_speed_cmd_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->speeds_count,&src->speeds_count,sizeof(uint32_t)*1); 
  if(src->speeds != NULL && src->speeds_count > 0)
  {
    if((dest->speeds = malloc(src->speeds_count*sizeof(float))) == NULL)
      return(0);
  }
  else
    dest->speeds = NULL;
  size += sizeof(float)*src->speeds_count;
  memcpy(dest->speeds,src->speeds,sizeof(float)*src->speeds_count); 
  return(size);
}
void player_actarray_multi_speed_cmd_t_cleanup(const player_actarray_multi_speed_cmd_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->speeds); 
}
player_actarray_multi_speed_cmd_t * player_actarray_multi_speed_cmd_t_clone(const player_actarray_multi_speed_cmd_t *msg)
{      
  player_actarray_multi_speed_cmd_t * clone = malloc(sizeof(player_actarray_multi_speed_cmd_t));
  if (clone)
    player_actarray_multi_speed_cmd_t_copy(clone,msg);
  return clone;
}
void player_actarray_multi_speed_cmd_t_free(player_actarray_multi_speed_cmd_t *msg)
{      
  player_actarray_multi_speed_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_multi_speed_cmd_t_sizeof(player_actarray_multi_speed_cmd_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(float)*msg->speeds_count; 
  return(size);
}

int xdr_player_actarray_home_cmd_t (XDR* xdrs, player_actarray_home_cmd_t * msg)
{   if(xdr_int(xdrs,&msg->joint) != 1)
    return(0);
  return(1);
}
int 
player_actarray_home_cmd_pack(void* buf, size_t buflen, player_actarray_home_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_home_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_home_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_home_cmd_t_copy(player_actarray_home_cmd_t *dest, const player_actarray_home_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_actarray_home_cmd_t));
  return sizeof(player_actarray_home_cmd_t);
} 
void player_actarray_home_cmd_t_cleanup(const player_actarray_home_cmd_t *msg)
{
} 
player_actarray_home_cmd_t * player_actarray_home_cmd_t_clone(const player_actarray_home_cmd_t *msg)
{      
  player_actarray_home_cmd_t * clone = malloc(sizeof(player_actarray_home_cmd_t));
  if (clone)
    player_actarray_home_cmd_t_copy(clone,msg);
  return clone;
}
void player_actarray_home_cmd_t_free(player_actarray_home_cmd_t *msg)
{      
  player_actarray_home_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_home_cmd_t_sizeof(player_actarray_home_cmd_t *msg)
{
  return sizeof(player_actarray_home_cmd_t);
} 

int xdr_player_actarray_current_cmd_t (XDR* xdrs, player_actarray_current_cmd_t * msg)
{   if(xdr_int(xdrs,&msg->joint) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->current) != 1)
    return(0);
  return(1);
}
int 
player_actarray_current_cmd_pack(void* buf, size_t buflen, player_actarray_current_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_current_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_current_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_current_cmd_t_copy(player_actarray_current_cmd_t *dest, const player_actarray_current_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_actarray_current_cmd_t));
  return sizeof(player_actarray_current_cmd_t);
} 
void player_actarray_current_cmd_t_cleanup(const player_actarray_current_cmd_t *msg)
{
} 
player_actarray_current_cmd_t * player_actarray_current_cmd_t_clone(const player_actarray_current_cmd_t *msg)
{      
  player_actarray_current_cmd_t * clone = malloc(sizeof(player_actarray_current_cmd_t));
  if (clone)
    player_actarray_current_cmd_t_copy(clone,msg);
  return clone;
}
void player_actarray_current_cmd_t_free(player_actarray_current_cmd_t *msg)
{      
  player_actarray_current_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_current_cmd_t_sizeof(player_actarray_current_cmd_t *msg)
{
  return sizeof(player_actarray_current_cmd_t);
} 

int xdr_player_actarray_multi_current_cmd_t (XDR* xdrs, player_actarray_multi_current_cmd_t * msg)
{   if(xdr_u_int(xdrs,&msg->currents_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->currents = malloc(msg->currents_count*sizeof(float))) == NULL)
      return(0);
  }
  {
    float* currents_p = msg->currents;
    if(xdr_array(xdrs, (char**)&currents_p, &msg->currents_count, msg->currents_count, sizeof(float), (xdrproc_t)xdr_float) != 1)
      return(0);
  }
  return(1);
}
int 
player_actarray_multi_current_cmd_pack(void* buf, size_t buflen, player_actarray_multi_current_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_multi_current_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_multi_current_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_multi_current_cmd_t_copy(player_actarray_multi_current_cmd_t *dest, const player_actarray_multi_current_cmd_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->currents_count,&src->currents_count,sizeof(uint32_t)*1); 
  if(src->currents != NULL && src->currents_count > 0)
  {
    if((dest->currents = malloc(src->currents_count*sizeof(float))) == NULL)
      return(0);
  }
  else
    dest->currents = NULL;
  size += sizeof(float)*src->currents_count;
  memcpy(dest->currents,src->currents,sizeof(float)*src->currents_count); 
  return(size);
}
void player_actarray_multi_current_cmd_t_cleanup(const player_actarray_multi_current_cmd_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->currents); 
}
player_actarray_multi_current_cmd_t * player_actarray_multi_current_cmd_t_clone(const player_actarray_multi_current_cmd_t *msg)
{      
  player_actarray_multi_current_cmd_t * clone = malloc(sizeof(player_actarray_multi_current_cmd_t));
  if (clone)
    player_actarray_multi_current_cmd_t_copy(clone,msg);
  return clone;
}
void player_actarray_multi_current_cmd_t_free(player_actarray_multi_current_cmd_t *msg)
{      
  player_actarray_multi_current_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_multi_current_cmd_t_sizeof(player_actarray_multi_current_cmd_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(float)*msg->currents_count; 
  return(size);
}

int xdr_player_actarray_power_config_t (XDR* xdrs, player_actarray_power_config_t * msg)
{   if(xdr_u_char(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_actarray_power_config_pack(void* buf, size_t buflen, player_actarray_power_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_power_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_power_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_power_config_t_copy(player_actarray_power_config_t *dest, const player_actarray_power_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_actarray_power_config_t));
  return sizeof(player_actarray_power_config_t);
} 
void player_actarray_power_config_t_cleanup(const player_actarray_power_config_t *msg)
{
} 
player_actarray_power_config_t * player_actarray_power_config_t_clone(const player_actarray_power_config_t *msg)
{      
  player_actarray_power_config_t * clone = malloc(sizeof(player_actarray_power_config_t));
  if (clone)
    player_actarray_power_config_t_copy(clone,msg);
  return clone;
}
void player_actarray_power_config_t_free(player_actarray_power_config_t *msg)
{      
  player_actarray_power_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_power_config_t_sizeof(player_actarray_power_config_t *msg)
{
  return sizeof(player_actarray_power_config_t);
} 

int xdr_player_actarray_brakes_config_t (XDR* xdrs, player_actarray_brakes_config_t * msg)
{   if(xdr_u_char(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_actarray_brakes_config_pack(void* buf, size_t buflen, player_actarray_brakes_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_brakes_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_brakes_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_brakes_config_t_copy(player_actarray_brakes_config_t *dest, const player_actarray_brakes_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_actarray_brakes_config_t));
  return sizeof(player_actarray_brakes_config_t);
} 
void player_actarray_brakes_config_t_cleanup(const player_actarray_brakes_config_t *msg)
{
} 
player_actarray_brakes_config_t * player_actarray_brakes_config_t_clone(const player_actarray_brakes_config_t *msg)
{      
  player_actarray_brakes_config_t * clone = malloc(sizeof(player_actarray_brakes_config_t));
  if (clone)
    player_actarray_brakes_config_t_copy(clone,msg);
  return clone;
}
void player_actarray_brakes_config_t_free(player_actarray_brakes_config_t *msg)
{      
  player_actarray_brakes_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_brakes_config_t_sizeof(player_actarray_brakes_config_t *msg)
{
  return sizeof(player_actarray_brakes_config_t);
} 

int xdr_player_actarray_speed_config_t (XDR* xdrs, player_actarray_speed_config_t * msg)
{   if(xdr_int(xdrs,&msg->joint) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->speed) != 1)
    return(0);
  return(1);
}
int 
player_actarray_speed_config_pack(void* buf, size_t buflen, player_actarray_speed_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_speed_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_speed_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_speed_config_t_copy(player_actarray_speed_config_t *dest, const player_actarray_speed_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_actarray_speed_config_t));
  return sizeof(player_actarray_speed_config_t);
} 
void player_actarray_speed_config_t_cleanup(const player_actarray_speed_config_t *msg)
{
} 
player_actarray_speed_config_t * player_actarray_speed_config_t_clone(const player_actarray_speed_config_t *msg)
{      
  player_actarray_speed_config_t * clone = malloc(sizeof(player_actarray_speed_config_t));
  if (clone)
    player_actarray_speed_config_t_copy(clone,msg);
  return clone;
}
void player_actarray_speed_config_t_free(player_actarray_speed_config_t *msg)
{      
  player_actarray_speed_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_speed_config_t_sizeof(player_actarray_speed_config_t *msg)
{
  return sizeof(player_actarray_speed_config_t);
} 

int xdr_player_actarray_accel_config_t (XDR* xdrs, player_actarray_accel_config_t * msg)
{   if(xdr_int(xdrs,&msg->joint) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->accel) != 1)
    return(0);
  return(1);
}
int 
player_actarray_accel_config_pack(void* buf, size_t buflen, player_actarray_accel_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_actarray_accel_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_actarray_accel_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_actarray_accel_config_t_copy(player_actarray_accel_config_t *dest, const player_actarray_accel_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_actarray_accel_config_t));
  return sizeof(player_actarray_accel_config_t);
} 
void player_actarray_accel_config_t_cleanup(const player_actarray_accel_config_t *msg)
{
} 
player_actarray_accel_config_t * player_actarray_accel_config_t_clone(const player_actarray_accel_config_t *msg)
{      
  player_actarray_accel_config_t * clone = malloc(sizeof(player_actarray_accel_config_t));
  if (clone)
    player_actarray_accel_config_t_copy(clone,msg);
  return clone;
}
void player_actarray_accel_config_t_free(player_actarray_accel_config_t *msg)
{      
  player_actarray_accel_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_actarray_accel_config_t_sizeof(player_actarray_accel_config_t *msg)
{
  return sizeof(player_actarray_accel_config_t);
} 

int xdr_player_joystick_data_t (XDR* xdrs, player_joystick_data_t * msg)
{   if(xdr_vector(xdrs, (char*)&msg->pos, 3, sizeof(int32_t), (xdrproc_t)xdr_int) != 1)
    return(0);
  if(xdr_vector(xdrs, (char*)&msg->scale, 3, sizeof(int32_t), (xdrproc_t)xdr_int) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->buttons) != 1)
    return(0);
  return(1);
}
int 
player_joystick_data_pack(void* buf, size_t buflen, player_joystick_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_joystick_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_joystick_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_joystick_data_t_copy(player_joystick_data_t *dest, const player_joystick_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_joystick_data_t));
  return sizeof(player_joystick_data_t);
} 
void player_joystick_data_t_cleanup(const player_joystick_data_t *msg)
{
} 
player_joystick_data_t * player_joystick_data_t_clone(const player_joystick_data_t *msg)
{      
  player_joystick_data_t * clone = malloc(sizeof(player_joystick_data_t));
  if (clone)
    player_joystick_data_t_copy(clone,msg);
  return clone;
}
void player_joystick_data_t_free(player_joystick_data_t *msg)
{      
  player_joystick_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_joystick_data_t_sizeof(player_joystick_data_t *msg)
{
  return sizeof(player_joystick_data_t);
} 

int xdr_player_graphics3d_cmd_draw_t (XDR* xdrs, player_graphics3d_cmd_draw_t * msg)
{   if(xdr_u_int(xdrs,&msg->draw_mode) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->points_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->points = malloc(msg->points_count*sizeof(player_point_3d_t))) == NULL)
      return(0);
  }
  {
    player_point_3d_t* points_p = msg->points;
    if(xdr_array(xdrs, (char**)&points_p, &msg->points_count, msg->points_count, sizeof(player_point_3d_t), (xdrproc_t)xdr_player_point_3d_t) != 1)
      return(0);
  }
  if(xdr_player_color_t(xdrs,&msg->color) != 1)
    return(0);
  return(1);
}
int 
player_graphics3d_cmd_draw_pack(void* buf, size_t buflen, player_graphics3d_cmd_draw_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_graphics3d_cmd_draw_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_graphics3d_cmd_draw_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_graphics3d_cmd_draw_t_copy(player_graphics3d_cmd_draw_t *dest, const player_graphics3d_cmd_draw_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->draw_mode,&src->draw_mode,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->points_count,&src->points_count,sizeof(uint32_t)*1); 
  if(src->points != NULL && src->points_count > 0)
  {
    if((dest->points = malloc(src->points_count*sizeof(player_point_3d_t))) == NULL)
      return(0);
  }
  else
    dest->points = NULL;
  size += sizeof(player_point_3d_t)*src->points_count;
  memcpy(dest->points,src->points,sizeof(player_point_3d_t)*src->points_count); 
  size += sizeof(player_color_t)*1;
  memcpy(&dest->color,&src->color,sizeof(player_color_t)*1); 
  return(size);
}
void player_graphics3d_cmd_draw_t_cleanup(const player_graphics3d_cmd_draw_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->points); 
}
player_graphics3d_cmd_draw_t * player_graphics3d_cmd_draw_t_clone(const player_graphics3d_cmd_draw_t *msg)
{      
  player_graphics3d_cmd_draw_t * clone = malloc(sizeof(player_graphics3d_cmd_draw_t));
  if (clone)
    player_graphics3d_cmd_draw_t_copy(clone,msg);
  return clone;
}
void player_graphics3d_cmd_draw_t_free(player_graphics3d_cmd_draw_t *msg)
{      
  player_graphics3d_cmd_draw_t_cleanup(msg);
  free(msg);
}
unsigned int player_graphics3d_cmd_draw_t_sizeof(player_graphics3d_cmd_draw_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_point_3d_t)*msg->points_count; 
  size += sizeof(player_color_t)*1; 
  return(size);
}

int xdr_player_graphics3d_cmd_translate_t (XDR* xdrs, player_graphics3d_cmd_translate_t * msg)
{   if(xdr_double(xdrs,&msg->x) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->y) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->z) != 1)
    return(0);
  return(1);
}
int 
player_graphics3d_cmd_translate_pack(void* buf, size_t buflen, player_graphics3d_cmd_translate_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_graphics3d_cmd_translate_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_graphics3d_cmd_translate_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_graphics3d_cmd_translate_t_copy(player_graphics3d_cmd_translate_t *dest, const player_graphics3d_cmd_translate_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_graphics3d_cmd_translate_t));
  return sizeof(player_graphics3d_cmd_translate_t);
} 
void player_graphics3d_cmd_translate_t_cleanup(const player_graphics3d_cmd_translate_t *msg)
{
} 
player_graphics3d_cmd_translate_t * player_graphics3d_cmd_translate_t_clone(const player_graphics3d_cmd_translate_t *msg)
{      
  player_graphics3d_cmd_translate_t * clone = malloc(sizeof(player_graphics3d_cmd_translate_t));
  if (clone)
    player_graphics3d_cmd_translate_t_copy(clone,msg);
  return clone;
}
void player_graphics3d_cmd_translate_t_free(player_graphics3d_cmd_translate_t *msg)
{      
  player_graphics3d_cmd_translate_t_cleanup(msg);
  free(msg);
}
unsigned int player_graphics3d_cmd_translate_t_sizeof(player_graphics3d_cmd_translate_t *msg)
{
  return sizeof(player_graphics3d_cmd_translate_t);
} 

int xdr_player_graphics3d_cmd_rotate_t (XDR* xdrs, player_graphics3d_cmd_rotate_t * msg)
{   if(xdr_double(xdrs,&msg->a) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->x) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->y) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->z) != 1)
    return(0);
  return(1);
}
int 
player_graphics3d_cmd_rotate_pack(void* buf, size_t buflen, player_graphics3d_cmd_rotate_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_graphics3d_cmd_rotate_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_graphics3d_cmd_rotate_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_graphics3d_cmd_rotate_t_copy(player_graphics3d_cmd_rotate_t *dest, const player_graphics3d_cmd_rotate_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_graphics3d_cmd_rotate_t));
  return sizeof(player_graphics3d_cmd_rotate_t);
} 
void player_graphics3d_cmd_rotate_t_cleanup(const player_graphics3d_cmd_rotate_t *msg)
{
} 
player_graphics3d_cmd_rotate_t * player_graphics3d_cmd_rotate_t_clone(const player_graphics3d_cmd_rotate_t *msg)
{      
  player_graphics3d_cmd_rotate_t * clone = malloc(sizeof(player_graphics3d_cmd_rotate_t));
  if (clone)
    player_graphics3d_cmd_rotate_t_copy(clone,msg);
  return clone;
}
void player_graphics3d_cmd_rotate_t_free(player_graphics3d_cmd_rotate_t *msg)
{      
  player_graphics3d_cmd_rotate_t_cleanup(msg);
  free(msg);
}
unsigned int player_graphics3d_cmd_rotate_t_sizeof(player_graphics3d_cmd_rotate_t *msg)
{
  return sizeof(player_graphics3d_cmd_rotate_t);
} 

int xdr_player_device_devlist_t (XDR* xdrs, player_device_devlist_t * msg)
{   if(xdr_u_int(xdrs,&msg->devices_count) != 1)
    return(0);
  {
    player_devaddr_t* devices_p = msg->devices;
    if(xdr_array(xdrs, (char**)&devices_p, &msg->devices_count, PLAYER_MAX_DEVICES, sizeof(player_devaddr_t), (xdrproc_t)xdr_player_devaddr_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_device_devlist_pack(void* buf, size_t buflen, player_device_devlist_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_device_devlist_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_device_devlist_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_device_devlist_t_copy(player_device_devlist_t *dest, const player_device_devlist_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_device_devlist_t));
  return sizeof(player_device_devlist_t);
} 
void player_device_devlist_t_cleanup(const player_device_devlist_t *msg)
{
} 
player_device_devlist_t * player_device_devlist_t_clone(const player_device_devlist_t *msg)
{      
  player_device_devlist_t * clone = malloc(sizeof(player_device_devlist_t));
  if (clone)
    player_device_devlist_t_copy(clone,msg);
  return clone;
}
void player_device_devlist_t_free(player_device_devlist_t *msg)
{      
  player_device_devlist_t_cleanup(msg);
  free(msg);
}
unsigned int player_device_devlist_t_sizeof(player_device_devlist_t *msg)
{
  return sizeof(player_device_devlist_t);
} 

int xdr_player_device_driverinfo_t (XDR* xdrs, player_device_driverinfo_t * msg)
{   if(xdr_player_devaddr_t(xdrs,&msg->addr) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->driver_name_count) != 1)
    return(0);
  {
    char* driver_name_p = msg->driver_name;
    if(xdr_bytes(xdrs, (char**)&driver_name_p, &msg->driver_name_count, PLAYER_MAX_DRIVER_STRING_LEN) != 1)
      return(0);
  }
  return(1);
}
int 
player_device_driverinfo_pack(void* buf, size_t buflen, player_device_driverinfo_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_device_driverinfo_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_device_driverinfo_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_device_driverinfo_t_copy(player_device_driverinfo_t *dest, const player_device_driverinfo_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_device_driverinfo_t));
  return sizeof(player_device_driverinfo_t);
} 
void player_device_driverinfo_t_cleanup(const player_device_driverinfo_t *msg)
{
} 
player_device_driverinfo_t * player_device_driverinfo_t_clone(const player_device_driverinfo_t *msg)
{      
  player_device_driverinfo_t * clone = malloc(sizeof(player_device_driverinfo_t));
  if (clone)
    player_device_driverinfo_t_copy(clone,msg);
  return clone;
}
void player_device_driverinfo_t_free(player_device_driverinfo_t *msg)
{      
  player_device_driverinfo_t_cleanup(msg);
  free(msg);
}
unsigned int player_device_driverinfo_t_sizeof(player_device_driverinfo_t *msg)
{
  return sizeof(player_device_driverinfo_t);
} 

int xdr_player_device_req_t (XDR* xdrs, player_device_req_t * msg)
{   if(xdr_player_devaddr_t(xdrs,&msg->addr) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->access) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->driver_name_count) != 1)
    return(0);
  {
    char* driver_name_p = msg->driver_name;
    if(xdr_bytes(xdrs, (char**)&driver_name_p, &msg->driver_name_count, PLAYER_MAX_DRIVER_STRING_LEN) != 1)
      return(0);
  }
  return(1);
}
int 
player_device_req_pack(void* buf, size_t buflen, player_device_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_device_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_device_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_device_req_t_copy(player_device_req_t *dest, const player_device_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_device_req_t));
  return sizeof(player_device_req_t);
} 
void player_device_req_t_cleanup(const player_device_req_t *msg)
{
} 
player_device_req_t * player_device_req_t_clone(const player_device_req_t *msg)
{      
  player_device_req_t * clone = malloc(sizeof(player_device_req_t));
  if (clone)
    player_device_req_t_copy(clone,msg);
  return clone;
}
void player_device_req_t_free(player_device_req_t *msg)
{      
  player_device_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_device_req_t_sizeof(player_device_req_t *msg)
{
  return sizeof(player_device_req_t);
} 

int xdr_player_device_datamode_req_t (XDR* xdrs, player_device_datamode_req_t * msg)
{   if(xdr_u_char(xdrs,&msg->mode) != 1)
    return(0);
  return(1);
}
int 
player_device_datamode_req_pack(void* buf, size_t buflen, player_device_datamode_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_device_datamode_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_device_datamode_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_device_datamode_req_t_copy(player_device_datamode_req_t *dest, const player_device_datamode_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_device_datamode_req_t));
  return sizeof(player_device_datamode_req_t);
} 
void player_device_datamode_req_t_cleanup(const player_device_datamode_req_t *msg)
{
} 
player_device_datamode_req_t * player_device_datamode_req_t_clone(const player_device_datamode_req_t *msg)
{      
  player_device_datamode_req_t * clone = malloc(sizeof(player_device_datamode_req_t));
  if (clone)
    player_device_datamode_req_t_copy(clone,msg);
  return clone;
}
void player_device_datamode_req_t_free(player_device_datamode_req_t *msg)
{      
  player_device_datamode_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_device_datamode_req_t_sizeof(player_device_datamode_req_t *msg)
{
  return sizeof(player_device_datamode_req_t);
} 

int xdr_player_device_auth_req_t (XDR* xdrs, player_device_auth_req_t * msg)
{   if(xdr_u_int(xdrs,&msg->auth_key_count) != 1)
    return(0);
  {
    uint8_t* auth_key_p = msg->auth_key;
    if(xdr_bytes(xdrs, (char**)&auth_key_p, &msg->auth_key_count, PLAYER_KEYLEN) != 1)
      return(0);
  }
  return(1);
}
int 
player_device_auth_req_pack(void* buf, size_t buflen, player_device_auth_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_device_auth_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_device_auth_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_device_auth_req_t_copy(player_device_auth_req_t *dest, const player_device_auth_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_device_auth_req_t));
  return sizeof(player_device_auth_req_t);
} 
void player_device_auth_req_t_cleanup(const player_device_auth_req_t *msg)
{
} 
player_device_auth_req_t * player_device_auth_req_t_clone(const player_device_auth_req_t *msg)
{      
  player_device_auth_req_t * clone = malloc(sizeof(player_device_auth_req_t));
  if (clone)
    player_device_auth_req_t_copy(clone,msg);
  return clone;
}
void player_device_auth_req_t_free(player_device_auth_req_t *msg)
{      
  player_device_auth_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_device_auth_req_t_sizeof(player_device_auth_req_t *msg)
{
  return sizeof(player_device_auth_req_t);
} 

int xdr_player_device_nameservice_req_t (XDR* xdrs, player_device_nameservice_req_t * msg)
{   if(xdr_u_int(xdrs,&msg->name_count) != 1)
    return(0);
  {
    uint8_t* name_p = msg->name;
    if(xdr_bytes(xdrs, (char**)&name_p, &msg->name_count, PLAYER_MAX_DRIVER_STRING_LEN) != 1)
      return(0);
  }
  if(xdr_u_short(xdrs,&msg->port) != 1)
    return(0);
  return(1);
}
int 
player_device_nameservice_req_pack(void* buf, size_t buflen, player_device_nameservice_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_device_nameservice_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_device_nameservice_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_device_nameservice_req_t_copy(player_device_nameservice_req_t *dest, const player_device_nameservice_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_device_nameservice_req_t));
  return sizeof(player_device_nameservice_req_t);
} 
void player_device_nameservice_req_t_cleanup(const player_device_nameservice_req_t *msg)
{
} 
player_device_nameservice_req_t * player_device_nameservice_req_t_clone(const player_device_nameservice_req_t *msg)
{      
  player_device_nameservice_req_t * clone = malloc(sizeof(player_device_nameservice_req_t));
  if (clone)
    player_device_nameservice_req_t_copy(clone,msg);
  return clone;
}
void player_device_nameservice_req_t_free(player_device_nameservice_req_t *msg)
{      
  player_device_nameservice_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_device_nameservice_req_t_sizeof(player_device_nameservice_req_t *msg)
{
  return sizeof(player_device_nameservice_req_t);
} 

int xdr_player_add_replace_rule_req_t (XDR* xdrs, player_add_replace_rule_req_t * msg)
{   if(xdr_int(xdrs,&msg->interf) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->index) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->type) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->subtype) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->replace) != 1)
    return(0);
  return(1);
}
int 
player_add_replace_rule_req_pack(void* buf, size_t buflen, player_add_replace_rule_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_add_replace_rule_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_add_replace_rule_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_add_replace_rule_req_t_copy(player_add_replace_rule_req_t *dest, const player_add_replace_rule_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_add_replace_rule_req_t));
  return sizeof(player_add_replace_rule_req_t);
} 
void player_add_replace_rule_req_t_cleanup(const player_add_replace_rule_req_t *msg)
{
} 
player_add_replace_rule_req_t * player_add_replace_rule_req_t_clone(const player_add_replace_rule_req_t *msg)
{      
  player_add_replace_rule_req_t * clone = malloc(sizeof(player_add_replace_rule_req_t));
  if (clone)
    player_add_replace_rule_req_t_copy(clone,msg);
  return clone;
}
void player_add_replace_rule_req_t_free(player_add_replace_rule_req_t *msg)
{      
  player_add_replace_rule_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_add_replace_rule_req_t_sizeof(player_add_replace_rule_req_t *msg)
{
  return sizeof(player_add_replace_rule_req_t);
} 

int xdr_player_vectormap_feature_data_t (XDR* xdrs, player_vectormap_feature_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->name_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->name = malloc(msg->name_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* name_p = msg->name;
    if(xdr_bytes(xdrs, (char**)&name_p, &msg->name_count, msg->name_count) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->wkb_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->wkb = malloc(msg->wkb_count*sizeof(uint8_t))) == NULL)
      return(0);
  }
  {
    uint8_t* wkb_p = msg->wkb;
    if(xdr_bytes(xdrs, (char**)&wkb_p, &msg->wkb_count, msg->wkb_count) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->attrib_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->attrib = malloc(msg->attrib_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* attrib_p = msg->attrib;
    if(xdr_bytes(xdrs, (char**)&attrib_p, &msg->attrib_count, msg->attrib_count) != 1)
      return(0);
  }
  return(1);
}
int 
player_vectormap_feature_data_pack(void* buf, size_t buflen, player_vectormap_feature_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_vectormap_feature_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_vectormap_feature_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_vectormap_feature_data_t_copy(player_vectormap_feature_data_t *dest, const player_vectormap_feature_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->name_count,&src->name_count,sizeof(uint32_t)*1); 
  if(src->name != NULL && src->name_count > 0)
  {
    if((dest->name = malloc(src->name_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->name = NULL;
  size += sizeof(char)*src->name_count;
  memcpy(dest->name,src->name,sizeof(char)*src->name_count); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->wkb_count,&src->wkb_count,sizeof(uint32_t)*1); 
  if(src->wkb != NULL && src->wkb_count > 0)
  {
    if((dest->wkb = malloc(src->wkb_count*sizeof(uint8_t))) == NULL)
      return(0);
  }
  else
    dest->wkb = NULL;
  size += sizeof(uint8_t)*src->wkb_count;
  memcpy(dest->wkb,src->wkb,sizeof(uint8_t)*src->wkb_count); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->attrib_count,&src->attrib_count,sizeof(uint32_t)*1); 
  if(src->attrib != NULL && src->attrib_count > 0)
  {
    if((dest->attrib = malloc(src->attrib_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->attrib = NULL;
  size += sizeof(char)*src->attrib_count;
  memcpy(dest->attrib,src->attrib,sizeof(char)*src->attrib_count); 
  return(size);
}
void player_vectormap_feature_data_t_cleanup(const player_vectormap_feature_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->name); 
  free(msg->wkb); 
  free(msg->attrib); 
}
player_vectormap_feature_data_t * player_vectormap_feature_data_t_clone(const player_vectormap_feature_data_t *msg)
{      
  player_vectormap_feature_data_t * clone = malloc(sizeof(player_vectormap_feature_data_t));
  if (clone)
    player_vectormap_feature_data_t_copy(clone,msg);
  return clone;
}
void player_vectormap_feature_data_t_free(player_vectormap_feature_data_t *msg)
{      
  player_vectormap_feature_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_vectormap_feature_data_t_sizeof(player_vectormap_feature_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->name_count; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint8_t)*msg->wkb_count; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->attrib_count; 
  return(size);
}

int xdr_player_vectormap_layer_info_t (XDR* xdrs, player_vectormap_layer_info_t * msg)
{   if(xdr_u_int(xdrs,&msg->name_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->name = malloc(msg->name_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* name_p = msg->name;
    if(xdr_bytes(xdrs, (char**)&name_p, &msg->name_count, msg->name_count) != 1)
      return(0);
  }
  if(xdr_player_extent2d_t(xdrs,&msg->extent) != 1)
    return(0);
  return(1);
}
int 
player_vectormap_layer_info_pack(void* buf, size_t buflen, player_vectormap_layer_info_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_vectormap_layer_info_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_vectormap_layer_info_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_vectormap_layer_info_t_copy(player_vectormap_layer_info_t *dest, const player_vectormap_layer_info_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->name_count,&src->name_count,sizeof(uint32_t)*1); 
  if(src->name != NULL && src->name_count > 0)
  {
    if((dest->name = malloc(src->name_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->name = NULL;
  size += sizeof(char)*src->name_count;
  memcpy(dest->name,src->name,sizeof(char)*src->name_count); 
  size += sizeof(player_extent2d_t)*1;
  memcpy(&dest->extent,&src->extent,sizeof(player_extent2d_t)*1); 
  return(size);
}
void player_vectormap_layer_info_t_cleanup(const player_vectormap_layer_info_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->name); 
}
player_vectormap_layer_info_t * player_vectormap_layer_info_t_clone(const player_vectormap_layer_info_t *msg)
{      
  player_vectormap_layer_info_t * clone = malloc(sizeof(player_vectormap_layer_info_t));
  if (clone)
    player_vectormap_layer_info_t_copy(clone,msg);
  return clone;
}
void player_vectormap_layer_info_t_free(player_vectormap_layer_info_t *msg)
{      
  player_vectormap_layer_info_t_cleanup(msg);
  free(msg);
}
unsigned int player_vectormap_layer_info_t_sizeof(player_vectormap_layer_info_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->name_count; 
  size += sizeof(player_extent2d_t)*1; 
  return(size);
}

int xdr_player_vectormap_layer_data_t (XDR* xdrs, player_vectormap_layer_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->name_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->name = malloc(msg->name_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* name_p = msg->name;
    if(xdr_bytes(xdrs, (char**)&name_p, &msg->name_count, msg->name_count) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->features_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->features = malloc(msg->features_count*sizeof(player_vectormap_feature_data_t))) == NULL)
      return(0);
  }
  {
    player_vectormap_feature_data_t* features_p = msg->features;
    if(xdr_array(xdrs, (char**)&features_p, &msg->features_count, msg->features_count, sizeof(player_vectormap_feature_data_t), (xdrproc_t)xdr_player_vectormap_feature_data_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_vectormap_layer_data_pack(void* buf, size_t buflen, player_vectormap_layer_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_vectormap_layer_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_vectormap_layer_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_vectormap_layer_data_t_copy(player_vectormap_layer_data_t *dest, const player_vectormap_layer_data_t *src)
{      
  unsigned ii;
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->name_count,&src->name_count,sizeof(uint32_t)*1); 
  if(src->name != NULL && src->name_count > 0)
  {
    if((dest->name = malloc(src->name_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->name = NULL;
  size += sizeof(char)*src->name_count;
  memcpy(dest->name,src->name,sizeof(char)*src->name_count); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->features_count,&src->features_count,sizeof(uint32_t)*1); 
  if(src->features != NULL && src->features_count > 0)
  {
    if((dest->features = malloc(src->features_count*sizeof(player_vectormap_feature_data_t))) == NULL)
      return(0);
  }
  else
    dest->features = NULL;
  for(ii = 0; ii < src->features_count; ii++)
  {size += player_vectormap_feature_data_t_copy(&dest->features[ii], &src->features[ii]);}
  return(size);
}
void player_vectormap_layer_data_t_cleanup(const player_vectormap_layer_data_t *msg)
{      
  unsigned ii;
  if(msg == NULL)
    return;
  free(msg->name); 
  for(ii = 0; ii < msg->features_count; ii++)
  {
    player_vectormap_feature_data_t_cleanup(&msg->features[ii]);
  }
  free(msg->features); 
}
player_vectormap_layer_data_t * player_vectormap_layer_data_t_clone(const player_vectormap_layer_data_t *msg)
{      
  player_vectormap_layer_data_t * clone = malloc(sizeof(player_vectormap_layer_data_t));
  if (clone)
    player_vectormap_layer_data_t_copy(clone,msg);
  return clone;
}
void player_vectormap_layer_data_t_free(player_vectormap_layer_data_t *msg)
{      
  player_vectormap_layer_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_vectormap_layer_data_t_sizeof(player_vectormap_layer_data_t *msg)
{
  unsigned ii;
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->name_count; 
  size += sizeof(uint32_t)*1; 
  for(ii = 0; ii < msg->features_count; ii++)
  {size += player_vectormap_feature_data_t_sizeof(&msg->features[ii]);}
  return(size);
}

int xdr_player_vectormap_info_t (XDR* xdrs, player_vectormap_info_t * msg)
{   if(xdr_u_int(xdrs,&msg->srid) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->layers_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->layers = malloc(msg->layers_count*sizeof(player_vectormap_layer_info_t))) == NULL)
      return(0);
  }
  {
    player_vectormap_layer_info_t* layers_p = msg->layers;
    if(xdr_array(xdrs, (char**)&layers_p, &msg->layers_count, msg->layers_count, sizeof(player_vectormap_layer_info_t), (xdrproc_t)xdr_player_vectormap_layer_info_t) != 1)
      return(0);
  }
  if(xdr_player_extent2d_t(xdrs,&msg->extent) != 1)
    return(0);
  return(1);
}
int 
player_vectormap_info_pack(void* buf, size_t buflen, player_vectormap_info_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_vectormap_info_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_vectormap_info_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_vectormap_info_t_copy(player_vectormap_info_t *dest, const player_vectormap_info_t *src)
{      
  unsigned ii;
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->srid,&src->srid,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->layers_count,&src->layers_count,sizeof(uint32_t)*1); 
  if(src->layers != NULL && src->layers_count > 0)
  {
    if((dest->layers = malloc(src->layers_count*sizeof(player_vectormap_layer_info_t))) == NULL)
      return(0);
  }
  else
    dest->layers = NULL;
  for(ii = 0; ii < src->layers_count; ii++)
  {size += player_vectormap_layer_info_t_copy(&dest->layers[ii], &src->layers[ii]);}
  size += sizeof(player_extent2d_t)*1;
  memcpy(&dest->extent,&src->extent,sizeof(player_extent2d_t)*1); 
  return(size);
}
void player_vectormap_info_t_cleanup(const player_vectormap_info_t *msg)
{      
  unsigned ii;
  if(msg == NULL)
    return;
  for(ii = 0; ii < msg->layers_count; ii++)
  {
    player_vectormap_layer_info_t_cleanup(&msg->layers[ii]);
  }
  free(msg->layers); 
}
player_vectormap_info_t * player_vectormap_info_t_clone(const player_vectormap_info_t *msg)
{      
  player_vectormap_info_t * clone = malloc(sizeof(player_vectormap_info_t));
  if (clone)
    player_vectormap_info_t_copy(clone,msg);
  return clone;
}
void player_vectormap_info_t_free(player_vectormap_info_t *msg)
{      
  player_vectormap_info_t_cleanup(msg);
  free(msg);
}
unsigned int player_vectormap_info_t_sizeof(player_vectormap_info_t *msg)
{
  unsigned ii;
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  for(ii = 0; ii < msg->layers_count; ii++)
  {size += player_vectormap_layer_info_t_sizeof(&msg->layers[ii]);}
  size += sizeof(player_extent2d_t)*1; 
  return(size);
}

int xdr_player_ir_data_t (XDR* xdrs, player_ir_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->voltages_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->voltages = malloc(msg->voltages_count*sizeof(float))) == NULL)
      return(0);
  }
  {
    float* voltages_p = msg->voltages;
    if(xdr_array(xdrs, (char**)&voltages_p, &msg->voltages_count, msg->voltages_count, sizeof(float), (xdrproc_t)xdr_float) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->ranges_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->ranges = malloc(msg->ranges_count*sizeof(float))) == NULL)
      return(0);
  }
  {
    float* ranges_p = msg->ranges;
    if(xdr_array(xdrs, (char**)&ranges_p, &msg->ranges_count, msg->ranges_count, sizeof(float), (xdrproc_t)xdr_float) != 1)
      return(0);
  }
  return(1);
}
int 
player_ir_data_pack(void* buf, size_t buflen, player_ir_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ir_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ir_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ir_data_t_copy(player_ir_data_t *dest, const player_ir_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->voltages_count,&src->voltages_count,sizeof(uint32_t)*1); 
  if(src->voltages != NULL && src->voltages_count > 0)
  {
    if((dest->voltages = malloc(src->voltages_count*sizeof(float))) == NULL)
      return(0);
  }
  else
    dest->voltages = NULL;
  size += sizeof(float)*src->voltages_count;
  memcpy(dest->voltages,src->voltages,sizeof(float)*src->voltages_count); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->ranges_count,&src->ranges_count,sizeof(uint32_t)*1); 
  if(src->ranges != NULL && src->ranges_count > 0)
  {
    if((dest->ranges = malloc(src->ranges_count*sizeof(float))) == NULL)
      return(0);
  }
  else
    dest->ranges = NULL;
  size += sizeof(float)*src->ranges_count;
  memcpy(dest->ranges,src->ranges,sizeof(float)*src->ranges_count); 
  return(size);
}
void player_ir_data_t_cleanup(const player_ir_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->voltages); 
  free(msg->ranges); 
}
player_ir_data_t * player_ir_data_t_clone(const player_ir_data_t *msg)
{      
  player_ir_data_t * clone = malloc(sizeof(player_ir_data_t));
  if (clone)
    player_ir_data_t_copy(clone,msg);
  return clone;
}
void player_ir_data_t_free(player_ir_data_t *msg)
{      
  player_ir_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_ir_data_t_sizeof(player_ir_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(float)*msg->voltages_count; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(float)*msg->ranges_count; 
  return(size);
}

int xdr_player_ir_pose_t (XDR* xdrs, player_ir_pose_t * msg)
{   if(xdr_u_int(xdrs,&msg->poses_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->poses = malloc(msg->poses_count*sizeof(player_pose3d_t))) == NULL)
      return(0);
  }
  {
    player_pose3d_t* poses_p = msg->poses;
    if(xdr_array(xdrs, (char**)&poses_p, &msg->poses_count, msg->poses_count, sizeof(player_pose3d_t), (xdrproc_t)xdr_player_pose3d_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_ir_pose_pack(void* buf, size_t buflen, player_ir_pose_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ir_pose_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ir_pose_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ir_pose_t_copy(player_ir_pose_t *dest, const player_ir_pose_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->poses_count,&src->poses_count,sizeof(uint32_t)*1); 
  if(src->poses != NULL && src->poses_count > 0)
  {
    if((dest->poses = malloc(src->poses_count*sizeof(player_pose3d_t))) == NULL)
      return(0);
  }
  else
    dest->poses = NULL;
  size += sizeof(player_pose3d_t)*src->poses_count;
  memcpy(dest->poses,src->poses,sizeof(player_pose3d_t)*src->poses_count); 
  return(size);
}
void player_ir_pose_t_cleanup(const player_ir_pose_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->poses); 
}
player_ir_pose_t * player_ir_pose_t_clone(const player_ir_pose_t *msg)
{      
  player_ir_pose_t * clone = malloc(sizeof(player_ir_pose_t));
  if (clone)
    player_ir_pose_t_copy(clone,msg);
  return clone;
}
void player_ir_pose_t_free(player_ir_pose_t *msg)
{      
  player_ir_pose_t_cleanup(msg);
  free(msg);
}
unsigned int player_ir_pose_t_sizeof(player_ir_pose_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_pose3d_t)*msg->poses_count; 
  return(size);
}

int xdr_player_ir_power_req_t (XDR* xdrs, player_ir_power_req_t * msg)
{   if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_ir_power_req_pack(void* buf, size_t buflen, player_ir_power_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ir_power_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ir_power_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ir_power_req_t_copy(player_ir_power_req_t *dest, const player_ir_power_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_ir_power_req_t));
  return sizeof(player_ir_power_req_t);
} 
void player_ir_power_req_t_cleanup(const player_ir_power_req_t *msg)
{
} 
player_ir_power_req_t * player_ir_power_req_t_clone(const player_ir_power_req_t *msg)
{      
  player_ir_power_req_t * clone = malloc(sizeof(player_ir_power_req_t));
  if (clone)
    player_ir_power_req_t_copy(clone,msg);
  return clone;
}
void player_ir_power_req_t_free(player_ir_power_req_t *msg)
{      
  player_ir_power_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_ir_power_req_t_sizeof(player_ir_power_req_t *msg)
{
  return sizeof(player_ir_power_req_t);
} 

int xdr_player_pointcloud3d_element_t (XDR* xdrs, player_pointcloud3d_element_t * msg)
{   if(xdr_player_point_3d_t(xdrs,&msg->point) != 1)
    return(0);
  if(xdr_player_color_t(xdrs,&msg->color) != 1)
    return(0);
  return(1);
}
int 
player_pointcloud3d_element_pack(void* buf, size_t buflen, player_pointcloud3d_element_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_pointcloud3d_element_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_pointcloud3d_element_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_pointcloud3d_element_t_copy(player_pointcloud3d_element_t *dest, const player_pointcloud3d_element_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_pointcloud3d_element_t));
  return sizeof(player_pointcloud3d_element_t);
} 
void player_pointcloud3d_element_t_cleanup(const player_pointcloud3d_element_t *msg)
{
} 
player_pointcloud3d_element_t * player_pointcloud3d_element_t_clone(const player_pointcloud3d_element_t *msg)
{      
  player_pointcloud3d_element_t * clone = malloc(sizeof(player_pointcloud3d_element_t));
  if (clone)
    player_pointcloud3d_element_t_copy(clone,msg);
  return clone;
}
void player_pointcloud3d_element_t_free(player_pointcloud3d_element_t *msg)
{      
  player_pointcloud3d_element_t_cleanup(msg);
  free(msg);
}
unsigned int player_pointcloud3d_element_t_sizeof(player_pointcloud3d_element_t *msg)
{
  return sizeof(player_pointcloud3d_element_t);
} 

int xdr_player_pointcloud3d_data_t (XDR* xdrs, player_pointcloud3d_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->points_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->points = malloc(msg->points_count*sizeof(player_pointcloud3d_element_t))) == NULL)
      return(0);
  }
  {
    player_pointcloud3d_element_t* points_p = msg->points;
    if(xdr_array(xdrs, (char**)&points_p, &msg->points_count, msg->points_count, sizeof(player_pointcloud3d_element_t), (xdrproc_t)xdr_player_pointcloud3d_element_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_pointcloud3d_data_pack(void* buf, size_t buflen, player_pointcloud3d_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_pointcloud3d_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_pointcloud3d_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_pointcloud3d_data_t_copy(player_pointcloud3d_data_t *dest, const player_pointcloud3d_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->points_count,&src->points_count,sizeof(uint32_t)*1); 
  if(src->points != NULL && src->points_count > 0)
  {
    if((dest->points = malloc(src->points_count*sizeof(player_pointcloud3d_element_t))) == NULL)
      return(0);
  }
  else
    dest->points = NULL;
  size += sizeof(player_pointcloud3d_element_t)*src->points_count;
  memcpy(dest->points,src->points,sizeof(player_pointcloud3d_element_t)*src->points_count); 
  return(size);
}
void player_pointcloud3d_data_t_cleanup(const player_pointcloud3d_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->points); 
}
player_pointcloud3d_data_t * player_pointcloud3d_data_t_clone(const player_pointcloud3d_data_t *msg)
{      
  player_pointcloud3d_data_t * clone = malloc(sizeof(player_pointcloud3d_data_t));
  if (clone)
    player_pointcloud3d_data_t_copy(clone,msg);
  return clone;
}
void player_pointcloud3d_data_t_free(player_pointcloud3d_data_t *msg)
{      
  player_pointcloud3d_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_pointcloud3d_data_t_sizeof(player_pointcloud3d_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_pointcloud3d_element_t)*msg->points_count; 
  return(size);
}

int xdr_player_speech_recognition_data_t (XDR* xdrs, player_speech_recognition_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->text_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->text = malloc(msg->text_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* text_p = msg->text;
    if(xdr_bytes(xdrs, (char**)&text_p, &msg->text_count, msg->text_count) != 1)
      return(0);
  }
  return(1);
}
int 
player_speech_recognition_data_pack(void* buf, size_t buflen, player_speech_recognition_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_speech_recognition_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_speech_recognition_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_speech_recognition_data_t_copy(player_speech_recognition_data_t *dest, const player_speech_recognition_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->text_count,&src->text_count,sizeof(uint32_t)*1); 
  if(src->text != NULL && src->text_count > 0)
  {
    if((dest->text = malloc(src->text_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->text = NULL;
  size += sizeof(char)*src->text_count;
  memcpy(dest->text,src->text,sizeof(char)*src->text_count); 
  return(size);
}
void player_speech_recognition_data_t_cleanup(const player_speech_recognition_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->text); 
}
player_speech_recognition_data_t * player_speech_recognition_data_t_clone(const player_speech_recognition_data_t *msg)
{      
  player_speech_recognition_data_t * clone = malloc(sizeof(player_speech_recognition_data_t));
  if (clone)
    player_speech_recognition_data_t_copy(clone,msg);
  return clone;
}
void player_speech_recognition_data_t_free(player_speech_recognition_data_t *msg)
{      
  player_speech_recognition_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_speech_recognition_data_t_sizeof(player_speech_recognition_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->text_count; 
  return(size);
}

int xdr_player_ranger_geom_t (XDR* xdrs, player_ranger_geom_t * msg)
{   if(xdr_player_pose3d_t(xdrs,&msg->pose) != 1)
    return(0);
  if(xdr_player_bbox3d_t(xdrs,&msg->size) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->sensor_poses_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->sensor_poses = malloc(msg->sensor_poses_count*sizeof(player_pose3d_t))) == NULL)
      return(0);
  }
  {
    player_pose3d_t* sensor_poses_p = msg->sensor_poses;
    if(xdr_array(xdrs, (char**)&sensor_poses_p, &msg->sensor_poses_count, msg->sensor_poses_count, sizeof(player_pose3d_t), (xdrproc_t)xdr_player_pose3d_t) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->sensor_sizes_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->sensor_sizes = malloc(msg->sensor_sizes_count*sizeof(player_bbox3d_t))) == NULL)
      return(0);
  }
  {
    player_bbox3d_t* sensor_sizes_p = msg->sensor_sizes;
    if(xdr_array(xdrs, (char**)&sensor_sizes_p, &msg->sensor_sizes_count, msg->sensor_sizes_count, sizeof(player_bbox3d_t), (xdrproc_t)xdr_player_bbox3d_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_ranger_geom_pack(void* buf, size_t buflen, player_ranger_geom_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ranger_geom_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ranger_geom_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ranger_geom_t_copy(player_ranger_geom_t *dest, const player_ranger_geom_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(player_pose3d_t)*1;
  memcpy(&dest->pose,&src->pose,sizeof(player_pose3d_t)*1); 
  size += sizeof(player_bbox3d_t)*1;
  memcpy(&dest->size,&src->size,sizeof(player_bbox3d_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->sensor_poses_count,&src->sensor_poses_count,sizeof(uint32_t)*1); 
  if(src->sensor_poses != NULL && src->sensor_poses_count > 0)
  {
    if((dest->sensor_poses = malloc(src->sensor_poses_count*sizeof(player_pose3d_t))) == NULL)
      return(0);
  }
  else
    dest->sensor_poses = NULL;
  size += sizeof(player_pose3d_t)*src->sensor_poses_count;
  memcpy(dest->sensor_poses,src->sensor_poses,sizeof(player_pose3d_t)*src->sensor_poses_count); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->sensor_sizes_count,&src->sensor_sizes_count,sizeof(uint32_t)*1); 
  if(src->sensor_sizes != NULL && src->sensor_sizes_count > 0)
  {
    if((dest->sensor_sizes = malloc(src->sensor_sizes_count*sizeof(player_bbox3d_t))) == NULL)
      return(0);
  }
  else
    dest->sensor_sizes = NULL;
  size += sizeof(player_bbox3d_t)*src->sensor_sizes_count;
  memcpy(dest->sensor_sizes,src->sensor_sizes,sizeof(player_bbox3d_t)*src->sensor_sizes_count); 
  return(size);
}
void player_ranger_geom_t_cleanup(const player_ranger_geom_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->sensor_poses); 
  free(msg->sensor_sizes); 
}
player_ranger_geom_t * player_ranger_geom_t_clone(const player_ranger_geom_t *msg)
{      
  player_ranger_geom_t * clone = malloc(sizeof(player_ranger_geom_t));
  if (clone)
    player_ranger_geom_t_copy(clone,msg);
  return clone;
}
void player_ranger_geom_t_free(player_ranger_geom_t *msg)
{      
  player_ranger_geom_t_cleanup(msg);
  free(msg);
}
unsigned int player_ranger_geom_t_sizeof(player_ranger_geom_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(player_pose3d_t)*1; 
  size += sizeof(player_bbox3d_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_pose3d_t)*msg->sensor_poses_count; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_bbox3d_t)*msg->sensor_sizes_count; 
  return(size);
}

int xdr_player_ranger_data_range_t (XDR* xdrs, player_ranger_data_range_t * msg)
{   if(xdr_u_int(xdrs,&msg->ranges_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->ranges = malloc(msg->ranges_count*sizeof(double))) == NULL)
      return(0);
  }
  {
    double* ranges_p = msg->ranges;
    if(xdr_array(xdrs, (char**)&ranges_p, &msg->ranges_count, msg->ranges_count, sizeof(double), (xdrproc_t)xdr_double) != 1)
      return(0);
  }
  return(1);
}
int 
player_ranger_data_range_pack(void* buf, size_t buflen, player_ranger_data_range_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ranger_data_range_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ranger_data_range_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ranger_data_range_t_copy(player_ranger_data_range_t *dest, const player_ranger_data_range_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->ranges_count,&src->ranges_count,sizeof(uint32_t)*1); 
  if(src->ranges != NULL && src->ranges_count > 0)
  {
    if((dest->ranges = malloc(src->ranges_count*sizeof(double))) == NULL)
      return(0);
  }
  else
    dest->ranges = NULL;
  size += sizeof(double)*src->ranges_count;
  memcpy(dest->ranges,src->ranges,sizeof(double)*src->ranges_count); 
  return(size);
}
void player_ranger_data_range_t_cleanup(const player_ranger_data_range_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->ranges); 
}
player_ranger_data_range_t * player_ranger_data_range_t_clone(const player_ranger_data_range_t *msg)
{      
  player_ranger_data_range_t * clone = malloc(sizeof(player_ranger_data_range_t));
  if (clone)
    player_ranger_data_range_t_copy(clone,msg);
  return clone;
}
void player_ranger_data_range_t_free(player_ranger_data_range_t *msg)
{      
  player_ranger_data_range_t_cleanup(msg);
  free(msg);
}
unsigned int player_ranger_data_range_t_sizeof(player_ranger_data_range_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(double)*msg->ranges_count; 
  return(size);
}

int xdr_player_ranger_data_rangepose_t (XDR* xdrs, player_ranger_data_rangepose_t * msg)
{   if(xdr_player_ranger_data_range_t(xdrs,&msg->data) != 1)
    return(0);
  if(xdr_player_ranger_geom_t(xdrs,&msg->geom) != 1)
    return(0);
  return(1);
}
int 
player_ranger_data_rangepose_pack(void* buf, size_t buflen, player_ranger_data_rangepose_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ranger_data_rangepose_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ranger_data_rangepose_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ranger_data_rangepose_t_copy(player_ranger_data_rangepose_t *dest, const player_ranger_data_rangepose_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  {size += player_ranger_data_range_t_copy(&dest->data, &src->data);}
  {size += player_ranger_geom_t_copy(&dest->geom, &src->geom);}
  return(size);
}
void player_ranger_data_rangepose_t_cleanup(const player_ranger_data_rangepose_t *msg)
{      
  
  if(msg == NULL)
    return;
  player_ranger_data_range_t_cleanup(&msg->data); 
  player_ranger_geom_t_cleanup(&msg->geom); 
}
player_ranger_data_rangepose_t * player_ranger_data_rangepose_t_clone(const player_ranger_data_rangepose_t *msg)
{      
  player_ranger_data_rangepose_t * clone = malloc(sizeof(player_ranger_data_rangepose_t));
  if (clone)
    player_ranger_data_rangepose_t_copy(clone,msg);
  return clone;
}
void player_ranger_data_rangepose_t_free(player_ranger_data_rangepose_t *msg)
{      
  player_ranger_data_rangepose_t_cleanup(msg);
  free(msg);
}
unsigned int player_ranger_data_rangepose_t_sizeof(player_ranger_data_rangepose_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  {size += player_ranger_data_range_t_sizeof(&msg->data);}
  {size += player_ranger_geom_t_sizeof(&msg->geom);}
  return(size);
}

int xdr_player_ranger_data_intns_t (XDR* xdrs, player_ranger_data_intns_t * msg)
{   if(xdr_u_int(xdrs,&msg->intensities_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->intensities = malloc(msg->intensities_count*sizeof(double))) == NULL)
      return(0);
  }
  {
    double* intensities_p = msg->intensities;
    if(xdr_array(xdrs, (char**)&intensities_p, &msg->intensities_count, msg->intensities_count, sizeof(double), (xdrproc_t)xdr_double) != 1)
      return(0);
  }
  return(1);
}
int 
player_ranger_data_intns_pack(void* buf, size_t buflen, player_ranger_data_intns_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ranger_data_intns_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ranger_data_intns_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ranger_data_intns_t_copy(player_ranger_data_intns_t *dest, const player_ranger_data_intns_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->intensities_count,&src->intensities_count,sizeof(uint32_t)*1); 
  if(src->intensities != NULL && src->intensities_count > 0)
  {
    if((dest->intensities = malloc(src->intensities_count*sizeof(double))) == NULL)
      return(0);
  }
  else
    dest->intensities = NULL;
  size += sizeof(double)*src->intensities_count;
  memcpy(dest->intensities,src->intensities,sizeof(double)*src->intensities_count); 
  return(size);
}
void player_ranger_data_intns_t_cleanup(const player_ranger_data_intns_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->intensities); 
}
player_ranger_data_intns_t * player_ranger_data_intns_t_clone(const player_ranger_data_intns_t *msg)
{      
  player_ranger_data_intns_t * clone = malloc(sizeof(player_ranger_data_intns_t));
  if (clone)
    player_ranger_data_intns_t_copy(clone,msg);
  return clone;
}
void player_ranger_data_intns_t_free(player_ranger_data_intns_t *msg)
{      
  player_ranger_data_intns_t_cleanup(msg);
  free(msg);
}
unsigned int player_ranger_data_intns_t_sizeof(player_ranger_data_intns_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(double)*msg->intensities_count; 
  return(size);
}

int xdr_player_ranger_data_intnspose_t (XDR* xdrs, player_ranger_data_intnspose_t * msg)
{   if(xdr_player_ranger_data_intns_t(xdrs,&msg->data) != 1)
    return(0);
  if(xdr_player_ranger_geom_t(xdrs,&msg->geom) != 1)
    return(0);
  return(1);
}
int 
player_ranger_data_intnspose_pack(void* buf, size_t buflen, player_ranger_data_intnspose_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ranger_data_intnspose_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ranger_data_intnspose_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ranger_data_intnspose_t_copy(player_ranger_data_intnspose_t *dest, const player_ranger_data_intnspose_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  {size += player_ranger_data_intns_t_copy(&dest->data, &src->data);}
  {size += player_ranger_geom_t_copy(&dest->geom, &src->geom);}
  return(size);
}
void player_ranger_data_intnspose_t_cleanup(const player_ranger_data_intnspose_t *msg)
{      
  
  if(msg == NULL)
    return;
  player_ranger_data_intns_t_cleanup(&msg->data); 
  player_ranger_geom_t_cleanup(&msg->geom); 
}
player_ranger_data_intnspose_t * player_ranger_data_intnspose_t_clone(const player_ranger_data_intnspose_t *msg)
{      
  player_ranger_data_intnspose_t * clone = malloc(sizeof(player_ranger_data_intnspose_t));
  if (clone)
    player_ranger_data_intnspose_t_copy(clone,msg);
  return clone;
}
void player_ranger_data_intnspose_t_free(player_ranger_data_intnspose_t *msg)
{      
  player_ranger_data_intnspose_t_cleanup(msg);
  free(msg);
}
unsigned int player_ranger_data_intnspose_t_sizeof(player_ranger_data_intnspose_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  {size += player_ranger_data_intns_t_sizeof(&msg->data);}
  {size += player_ranger_geom_t_sizeof(&msg->geom);}
  return(size);
}

int xdr_player_ranger_power_config_t (XDR* xdrs, player_ranger_power_config_t * msg)
{   if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_ranger_power_config_pack(void* buf, size_t buflen, player_ranger_power_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ranger_power_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ranger_power_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ranger_power_config_t_copy(player_ranger_power_config_t *dest, const player_ranger_power_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_ranger_power_config_t));
  return sizeof(player_ranger_power_config_t);
} 
void player_ranger_power_config_t_cleanup(const player_ranger_power_config_t *msg)
{
} 
player_ranger_power_config_t * player_ranger_power_config_t_clone(const player_ranger_power_config_t *msg)
{      
  player_ranger_power_config_t * clone = malloc(sizeof(player_ranger_power_config_t));
  if (clone)
    player_ranger_power_config_t_copy(clone,msg);
  return clone;
}
void player_ranger_power_config_t_free(player_ranger_power_config_t *msg)
{      
  player_ranger_power_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_ranger_power_config_t_sizeof(player_ranger_power_config_t *msg)
{
  return sizeof(player_ranger_power_config_t);
} 

int xdr_player_ranger_intns_config_t (XDR* xdrs, player_ranger_intns_config_t * msg)
{   if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_ranger_intns_config_pack(void* buf, size_t buflen, player_ranger_intns_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ranger_intns_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ranger_intns_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ranger_intns_config_t_copy(player_ranger_intns_config_t *dest, const player_ranger_intns_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_ranger_intns_config_t));
  return sizeof(player_ranger_intns_config_t);
} 
void player_ranger_intns_config_t_cleanup(const player_ranger_intns_config_t *msg)
{
} 
player_ranger_intns_config_t * player_ranger_intns_config_t_clone(const player_ranger_intns_config_t *msg)
{      
  player_ranger_intns_config_t * clone = malloc(sizeof(player_ranger_intns_config_t));
  if (clone)
    player_ranger_intns_config_t_copy(clone,msg);
  return clone;
}
void player_ranger_intns_config_t_free(player_ranger_intns_config_t *msg)
{      
  player_ranger_intns_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_ranger_intns_config_t_sizeof(player_ranger_intns_config_t *msg)
{
  return sizeof(player_ranger_intns_config_t);
} 

int xdr_player_ranger_config_t (XDR* xdrs, player_ranger_config_t * msg)
{   if(xdr_double(xdrs,&msg->min_angle) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->max_angle) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->resolution) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->max_range) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->range_res) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->frequency) != 1)
    return(0);
  return(1);
}
int 
player_ranger_config_pack(void* buf, size_t buflen, player_ranger_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ranger_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ranger_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ranger_config_t_copy(player_ranger_config_t *dest, const player_ranger_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_ranger_config_t));
  return sizeof(player_ranger_config_t);
} 
void player_ranger_config_t_cleanup(const player_ranger_config_t *msg)
{
} 
player_ranger_config_t * player_ranger_config_t_clone(const player_ranger_config_t *msg)
{      
  player_ranger_config_t * clone = malloc(sizeof(player_ranger_config_t));
  if (clone)
    player_ranger_config_t_copy(clone,msg);
  return clone;
}
void player_ranger_config_t_free(player_ranger_config_t *msg)
{      
  player_ranger_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_ranger_config_t_sizeof(player_ranger_config_t *msg)
{
  return sizeof(player_ranger_config_t);
} 

int xdr_player_bumper_data_t (XDR* xdrs, player_bumper_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->bumpers_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->bumpers = malloc(msg->bumpers_count*sizeof(uint8_t))) == NULL)
      return(0);
  }
  {
    uint8_t* bumpers_p = msg->bumpers;
    if(xdr_bytes(xdrs, (char**)&bumpers_p, &msg->bumpers_count, msg->bumpers_count) != 1)
      return(0);
  }
  return(1);
}
int 
player_bumper_data_pack(void* buf, size_t buflen, player_bumper_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_bumper_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_bumper_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_bumper_data_t_copy(player_bumper_data_t *dest, const player_bumper_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->bumpers_count,&src->bumpers_count,sizeof(uint32_t)*1); 
  if(src->bumpers != NULL && src->bumpers_count > 0)
  {
    if((dest->bumpers = malloc(src->bumpers_count*sizeof(uint8_t))) == NULL)
      return(0);
  }
  else
    dest->bumpers = NULL;
  size += sizeof(uint8_t)*src->bumpers_count;
  memcpy(dest->bumpers,src->bumpers,sizeof(uint8_t)*src->bumpers_count); 
  return(size);
}
void player_bumper_data_t_cleanup(const player_bumper_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->bumpers); 
}
player_bumper_data_t * player_bumper_data_t_clone(const player_bumper_data_t *msg)
{      
  player_bumper_data_t * clone = malloc(sizeof(player_bumper_data_t));
  if (clone)
    player_bumper_data_t_copy(clone,msg);
  return clone;
}
void player_bumper_data_t_free(player_bumper_data_t *msg)
{      
  player_bumper_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_bumper_data_t_sizeof(player_bumper_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint8_t)*msg->bumpers_count; 
  return(size);
}

int xdr_player_bumper_define_t (XDR* xdrs, player_bumper_define_t * msg)
{   if(xdr_player_pose3d_t(xdrs,&msg->pose) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->length) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->radius) != 1)
    return(0);
  return(1);
}
int 
player_bumper_define_pack(void* buf, size_t buflen, player_bumper_define_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_bumper_define_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_bumper_define_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_bumper_define_t_copy(player_bumper_define_t *dest, const player_bumper_define_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_bumper_define_t));
  return sizeof(player_bumper_define_t);
} 
void player_bumper_define_t_cleanup(const player_bumper_define_t *msg)
{
} 
player_bumper_define_t * player_bumper_define_t_clone(const player_bumper_define_t *msg)
{      
  player_bumper_define_t * clone = malloc(sizeof(player_bumper_define_t));
  if (clone)
    player_bumper_define_t_copy(clone,msg);
  return clone;
}
void player_bumper_define_t_free(player_bumper_define_t *msg)
{      
  player_bumper_define_t_cleanup(msg);
  free(msg);
}
unsigned int player_bumper_define_t_sizeof(player_bumper_define_t *msg)
{
  return sizeof(player_bumper_define_t);
} 

int xdr_player_bumper_geom_t (XDR* xdrs, player_bumper_geom_t * msg)
{   if(xdr_u_int(xdrs,&msg->bumper_def_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->bumper_def = malloc(msg->bumper_def_count*sizeof(player_bumper_define_t))) == NULL)
      return(0);
  }
  {
    player_bumper_define_t* bumper_def_p = msg->bumper_def;
    if(xdr_array(xdrs, (char**)&bumper_def_p, &msg->bumper_def_count, msg->bumper_def_count, sizeof(player_bumper_define_t), (xdrproc_t)xdr_player_bumper_define_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_bumper_geom_pack(void* buf, size_t buflen, player_bumper_geom_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_bumper_geom_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_bumper_geom_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_bumper_geom_t_copy(player_bumper_geom_t *dest, const player_bumper_geom_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->bumper_def_count,&src->bumper_def_count,sizeof(uint32_t)*1); 
  if(src->bumper_def != NULL && src->bumper_def_count > 0)
  {
    if((dest->bumper_def = malloc(src->bumper_def_count*sizeof(player_bumper_define_t))) == NULL)
      return(0);
  }
  else
    dest->bumper_def = NULL;
  size += sizeof(player_bumper_define_t)*src->bumper_def_count;
  memcpy(dest->bumper_def,src->bumper_def,sizeof(player_bumper_define_t)*src->bumper_def_count); 
  return(size);
}
void player_bumper_geom_t_cleanup(const player_bumper_geom_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->bumper_def); 
}
player_bumper_geom_t * player_bumper_geom_t_clone(const player_bumper_geom_t *msg)
{      
  player_bumper_geom_t * clone = malloc(sizeof(player_bumper_geom_t));
  if (clone)
    player_bumper_geom_t_copy(clone,msg);
  return clone;
}
void player_bumper_geom_t_free(player_bumper_geom_t *msg)
{      
  player_bumper_geom_t_cleanup(msg);
  free(msg);
}
unsigned int player_bumper_geom_t_sizeof(player_bumper_geom_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_bumper_define_t)*msg->bumper_def_count; 
  return(size);
}

int xdr_player_power_data_t (XDR* xdrs, player_power_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->valid) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->volts) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->percent) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->joules) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->watts) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->charging) != 1)
    return(0);
  return(1);
}
int 
player_power_data_pack(void* buf, size_t buflen, player_power_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_power_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_power_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_power_data_t_copy(player_power_data_t *dest, const player_power_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_power_data_t));
  return sizeof(player_power_data_t);
} 
void player_power_data_t_cleanup(const player_power_data_t *msg)
{
} 
player_power_data_t * player_power_data_t_clone(const player_power_data_t *msg)
{      
  player_power_data_t * clone = malloc(sizeof(player_power_data_t));
  if (clone)
    player_power_data_t_copy(clone,msg);
  return clone;
}
void player_power_data_t_free(player_power_data_t *msg)
{      
  player_power_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_power_data_t_sizeof(player_power_data_t *msg)
{
  return sizeof(player_power_data_t);
} 

int xdr_player_power_chargepolicy_config_t (XDR* xdrs, player_power_chargepolicy_config_t * msg)
{   if(xdr_u_char(xdrs,&msg->enable_input) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->enable_output) != 1)
    return(0);
  return(1);
}
int 
player_power_chargepolicy_config_pack(void* buf, size_t buflen, player_power_chargepolicy_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_power_chargepolicy_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_power_chargepolicy_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_power_chargepolicy_config_t_copy(player_power_chargepolicy_config_t *dest, const player_power_chargepolicy_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_power_chargepolicy_config_t));
  return sizeof(player_power_chargepolicy_config_t);
} 
void player_power_chargepolicy_config_t_cleanup(const player_power_chargepolicy_config_t *msg)
{
} 
player_power_chargepolicy_config_t * player_power_chargepolicy_config_t_clone(const player_power_chargepolicy_config_t *msg)
{      
  player_power_chargepolicy_config_t * clone = malloc(sizeof(player_power_chargepolicy_config_t));
  if (clone)
    player_power_chargepolicy_config_t_copy(clone,msg);
  return clone;
}
void player_power_chargepolicy_config_t_free(player_power_chargepolicy_config_t *msg)
{      
  player_power_chargepolicy_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_power_chargepolicy_config_t_sizeof(player_power_chargepolicy_config_t *msg)
{
  return sizeof(player_power_chargepolicy_config_t);
} 

int xdr_player_blobfinder_blob_t (XDR* xdrs, player_blobfinder_blob_t * msg)
{   if(xdr_u_int(xdrs,&msg->id) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->color) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->area) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->x) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->y) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->left) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->right) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->top) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->bottom) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->range) != 1)
    return(0);
  return(1);
}
int 
player_blobfinder_blob_pack(void* buf, size_t buflen, player_blobfinder_blob_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_blobfinder_blob_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_blobfinder_blob_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_blobfinder_blob_t_copy(player_blobfinder_blob_t *dest, const player_blobfinder_blob_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_blobfinder_blob_t));
  return sizeof(player_blobfinder_blob_t);
} 
void player_blobfinder_blob_t_cleanup(const player_blobfinder_blob_t *msg)
{
} 
player_blobfinder_blob_t * player_blobfinder_blob_t_clone(const player_blobfinder_blob_t *msg)
{      
  player_blobfinder_blob_t * clone = malloc(sizeof(player_blobfinder_blob_t));
  if (clone)
    player_blobfinder_blob_t_copy(clone,msg);
  return clone;
}
void player_blobfinder_blob_t_free(player_blobfinder_blob_t *msg)
{      
  player_blobfinder_blob_t_cleanup(msg);
  free(msg);
}
unsigned int player_blobfinder_blob_t_sizeof(player_blobfinder_blob_t *msg)
{
  return sizeof(player_blobfinder_blob_t);
} 

int xdr_player_blobfinder_data_t (XDR* xdrs, player_blobfinder_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->width) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->height) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->blobs_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->blobs = malloc(msg->blobs_count*sizeof(player_blobfinder_blob_t))) == NULL)
      return(0);
  }
  {
    player_blobfinder_blob_t* blobs_p = msg->blobs;
    if(xdr_array(xdrs, (char**)&blobs_p, &msg->blobs_count, msg->blobs_count, sizeof(player_blobfinder_blob_t), (xdrproc_t)xdr_player_blobfinder_blob_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_blobfinder_data_pack(void* buf, size_t buflen, player_blobfinder_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_blobfinder_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_blobfinder_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_blobfinder_data_t_copy(player_blobfinder_data_t *dest, const player_blobfinder_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->width,&src->width,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->height,&src->height,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->blobs_count,&src->blobs_count,sizeof(uint32_t)*1); 
  if(src->blobs != NULL && src->blobs_count > 0)
  {
    if((dest->blobs = malloc(src->blobs_count*sizeof(player_blobfinder_blob_t))) == NULL)
      return(0);
  }
  else
    dest->blobs = NULL;
  size += sizeof(player_blobfinder_blob_t)*src->blobs_count;
  memcpy(dest->blobs,src->blobs,sizeof(player_blobfinder_blob_t)*src->blobs_count); 
  return(size);
}
void player_blobfinder_data_t_cleanup(const player_blobfinder_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->blobs); 
}
player_blobfinder_data_t * player_blobfinder_data_t_clone(const player_blobfinder_data_t *msg)
{      
  player_blobfinder_data_t * clone = malloc(sizeof(player_blobfinder_data_t));
  if (clone)
    player_blobfinder_data_t_copy(clone,msg);
  return clone;
}
void player_blobfinder_data_t_free(player_blobfinder_data_t *msg)
{      
  player_blobfinder_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_blobfinder_data_t_sizeof(player_blobfinder_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_blobfinder_blob_t)*msg->blobs_count; 
  return(size);
}

int xdr_player_blobfinder_color_config_t (XDR* xdrs, player_blobfinder_color_config_t * msg)
{   if(xdr_u_int(xdrs,&msg->channel) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->rmin) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->rmax) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->gmin) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->gmax) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->bmin) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->bmax) != 1)
    return(0);
  return(1);
}
int 
player_blobfinder_color_config_pack(void* buf, size_t buflen, player_blobfinder_color_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_blobfinder_color_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_blobfinder_color_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_blobfinder_color_config_t_copy(player_blobfinder_color_config_t *dest, const player_blobfinder_color_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_blobfinder_color_config_t));
  return sizeof(player_blobfinder_color_config_t);
} 
void player_blobfinder_color_config_t_cleanup(const player_blobfinder_color_config_t *msg)
{
} 
player_blobfinder_color_config_t * player_blobfinder_color_config_t_clone(const player_blobfinder_color_config_t *msg)
{      
  player_blobfinder_color_config_t * clone = malloc(sizeof(player_blobfinder_color_config_t));
  if (clone)
    player_blobfinder_color_config_t_copy(clone,msg);
  return clone;
}
void player_blobfinder_color_config_t_free(player_blobfinder_color_config_t *msg)
{      
  player_blobfinder_color_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_blobfinder_color_config_t_sizeof(player_blobfinder_color_config_t *msg)
{
  return sizeof(player_blobfinder_color_config_t);
} 

int xdr_player_blobfinder_imager_config_t (XDR* xdrs, player_blobfinder_imager_config_t * msg)
{   if(xdr_int(xdrs,&msg->brightness) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->contrast) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->colormode) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->autogain) != 1)
    return(0);
  return(1);
}
int 
player_blobfinder_imager_config_pack(void* buf, size_t buflen, player_blobfinder_imager_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_blobfinder_imager_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_blobfinder_imager_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_blobfinder_imager_config_t_copy(player_blobfinder_imager_config_t *dest, const player_blobfinder_imager_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_blobfinder_imager_config_t));
  return sizeof(player_blobfinder_imager_config_t);
} 
void player_blobfinder_imager_config_t_cleanup(const player_blobfinder_imager_config_t *msg)
{
} 
player_blobfinder_imager_config_t * player_blobfinder_imager_config_t_clone(const player_blobfinder_imager_config_t *msg)
{      
  player_blobfinder_imager_config_t * clone = malloc(sizeof(player_blobfinder_imager_config_t));
  if (clone)
    player_blobfinder_imager_config_t_copy(clone,msg);
  return clone;
}
void player_blobfinder_imager_config_t_free(player_blobfinder_imager_config_t *msg)
{      
  player_blobfinder_imager_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_blobfinder_imager_config_t_sizeof(player_blobfinder_imager_config_t *msg)
{
  return sizeof(player_blobfinder_imager_config_t);
} 

int xdr_player_imu_data_state_t (XDR* xdrs, player_imu_data_state_t * msg)
{   if(xdr_player_pose3d_t(xdrs,&msg->pose) != 1)
    return(0);
  return(1);
}
int 
player_imu_data_state_pack(void* buf, size_t buflen, player_imu_data_state_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_imu_data_state_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_imu_data_state_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_imu_data_state_t_copy(player_imu_data_state_t *dest, const player_imu_data_state_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_imu_data_state_t));
  return sizeof(player_imu_data_state_t);
} 
void player_imu_data_state_t_cleanup(const player_imu_data_state_t *msg)
{
} 
player_imu_data_state_t * player_imu_data_state_t_clone(const player_imu_data_state_t *msg)
{      
  player_imu_data_state_t * clone = malloc(sizeof(player_imu_data_state_t));
  if (clone)
    player_imu_data_state_t_copy(clone,msg);
  return clone;
}
void player_imu_data_state_t_free(player_imu_data_state_t *msg)
{      
  player_imu_data_state_t_cleanup(msg);
  free(msg);
}
unsigned int player_imu_data_state_t_sizeof(player_imu_data_state_t *msg)
{
  return sizeof(player_imu_data_state_t);
} 

int xdr_player_imu_data_calib_t (XDR* xdrs, player_imu_data_calib_t * msg)
{   if(xdr_float(xdrs,&msg->accel_x) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->accel_y) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->accel_z) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->gyro_x) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->gyro_y) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->gyro_z) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->magn_x) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->magn_y) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->magn_z) != 1)
    return(0);
  return(1);
}
int 
player_imu_data_calib_pack(void* buf, size_t buflen, player_imu_data_calib_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_imu_data_calib_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_imu_data_calib_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_imu_data_calib_t_copy(player_imu_data_calib_t *dest, const player_imu_data_calib_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_imu_data_calib_t));
  return sizeof(player_imu_data_calib_t);
} 
void player_imu_data_calib_t_cleanup(const player_imu_data_calib_t *msg)
{
} 
player_imu_data_calib_t * player_imu_data_calib_t_clone(const player_imu_data_calib_t *msg)
{      
  player_imu_data_calib_t * clone = malloc(sizeof(player_imu_data_calib_t));
  if (clone)
    player_imu_data_calib_t_copy(clone,msg);
  return clone;
}
void player_imu_data_calib_t_free(player_imu_data_calib_t *msg)
{      
  player_imu_data_calib_t_cleanup(msg);
  free(msg);
}
unsigned int player_imu_data_calib_t_sizeof(player_imu_data_calib_t *msg)
{
  return sizeof(player_imu_data_calib_t);
} 

int xdr_player_imu_data_quat_t (XDR* xdrs, player_imu_data_quat_t * msg)
{   if(xdr_player_imu_data_calib_t(xdrs,&msg->calib_data) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->q0) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->q1) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->q2) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->q3) != 1)
    return(0);
  return(1);
}
int 
player_imu_data_quat_pack(void* buf, size_t buflen, player_imu_data_quat_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_imu_data_quat_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_imu_data_quat_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_imu_data_quat_t_copy(player_imu_data_quat_t *dest, const player_imu_data_quat_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_imu_data_quat_t));
  return sizeof(player_imu_data_quat_t);
} 
void player_imu_data_quat_t_cleanup(const player_imu_data_quat_t *msg)
{
} 
player_imu_data_quat_t * player_imu_data_quat_t_clone(const player_imu_data_quat_t *msg)
{      
  player_imu_data_quat_t * clone = malloc(sizeof(player_imu_data_quat_t));
  if (clone)
    player_imu_data_quat_t_copy(clone,msg);
  return clone;
}
void player_imu_data_quat_t_free(player_imu_data_quat_t *msg)
{      
  player_imu_data_quat_t_cleanup(msg);
  free(msg);
}
unsigned int player_imu_data_quat_t_sizeof(player_imu_data_quat_t *msg)
{
  return sizeof(player_imu_data_quat_t);
} 

int xdr_player_imu_data_euler_t (XDR* xdrs, player_imu_data_euler_t * msg)
{   if(xdr_player_imu_data_calib_t(xdrs,&msg->calib_data) != 1)
    return(0);
  if(xdr_player_orientation_3d_t(xdrs,&msg->orientation) != 1)
    return(0);
  return(1);
}
int 
player_imu_data_euler_pack(void* buf, size_t buflen, player_imu_data_euler_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_imu_data_euler_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_imu_data_euler_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_imu_data_euler_t_copy(player_imu_data_euler_t *dest, const player_imu_data_euler_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_imu_data_euler_t));
  return sizeof(player_imu_data_euler_t);
} 
void player_imu_data_euler_t_cleanup(const player_imu_data_euler_t *msg)
{
} 
player_imu_data_euler_t * player_imu_data_euler_t_clone(const player_imu_data_euler_t *msg)
{      
  player_imu_data_euler_t * clone = malloc(sizeof(player_imu_data_euler_t));
  if (clone)
    player_imu_data_euler_t_copy(clone,msg);
  return clone;
}
void player_imu_data_euler_t_free(player_imu_data_euler_t *msg)
{      
  player_imu_data_euler_t_cleanup(msg);
  free(msg);
}
unsigned int player_imu_data_euler_t_sizeof(player_imu_data_euler_t *msg)
{
  return sizeof(player_imu_data_euler_t);
} 

int xdr_player_imu_datatype_config_t (XDR* xdrs, player_imu_datatype_config_t * msg)
{   if(xdr_u_char(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_imu_datatype_config_pack(void* buf, size_t buflen, player_imu_datatype_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_imu_datatype_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_imu_datatype_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_imu_datatype_config_t_copy(player_imu_datatype_config_t *dest, const player_imu_datatype_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_imu_datatype_config_t));
  return sizeof(player_imu_datatype_config_t);
} 
void player_imu_datatype_config_t_cleanup(const player_imu_datatype_config_t *msg)
{
} 
player_imu_datatype_config_t * player_imu_datatype_config_t_clone(const player_imu_datatype_config_t *msg)
{      
  player_imu_datatype_config_t * clone = malloc(sizeof(player_imu_datatype_config_t));
  if (clone)
    player_imu_datatype_config_t_copy(clone,msg);
  return clone;
}
void player_imu_datatype_config_t_free(player_imu_datatype_config_t *msg)
{      
  player_imu_datatype_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_imu_datatype_config_t_sizeof(player_imu_datatype_config_t *msg)
{
  return sizeof(player_imu_datatype_config_t);
} 

int xdr_player_imu_reset_orientation_config_t (XDR* xdrs, player_imu_reset_orientation_config_t * msg)
{   if(xdr_u_int(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_imu_reset_orientation_config_pack(void* buf, size_t buflen, player_imu_reset_orientation_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_imu_reset_orientation_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_imu_reset_orientation_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_imu_reset_orientation_config_t_copy(player_imu_reset_orientation_config_t *dest, const player_imu_reset_orientation_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_imu_reset_orientation_config_t));
  return sizeof(player_imu_reset_orientation_config_t);
} 
void player_imu_reset_orientation_config_t_cleanup(const player_imu_reset_orientation_config_t *msg)
{
} 
player_imu_reset_orientation_config_t * player_imu_reset_orientation_config_t_clone(const player_imu_reset_orientation_config_t *msg)
{      
  player_imu_reset_orientation_config_t * clone = malloc(sizeof(player_imu_reset_orientation_config_t));
  if (clone)
    player_imu_reset_orientation_config_t_copy(clone,msg);
  return clone;
}
void player_imu_reset_orientation_config_t_free(player_imu_reset_orientation_config_t *msg)
{      
  player_imu_reset_orientation_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_imu_reset_orientation_config_t_sizeof(player_imu_reset_orientation_config_t *msg)
{
  return sizeof(player_imu_reset_orientation_config_t);
} 

int xdr_player_graphics2d_cmd_points_t (XDR* xdrs, player_graphics2d_cmd_points_t * msg)
{   if(xdr_u_int(xdrs,&msg->points_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->points = malloc(msg->points_count*sizeof(player_point_2d_t))) == NULL)
      return(0);
  }
  {
    player_point_2d_t* points_p = msg->points;
    if(xdr_array(xdrs, (char**)&points_p, &msg->points_count, msg->points_count, sizeof(player_point_2d_t), (xdrproc_t)xdr_player_point_2d_t) != 1)
      return(0);
  }
  if(xdr_player_color_t(xdrs,&msg->color) != 1)
    return(0);
  return(1);
}
int 
player_graphics2d_cmd_points_pack(void* buf, size_t buflen, player_graphics2d_cmd_points_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_graphics2d_cmd_points_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_graphics2d_cmd_points_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_graphics2d_cmd_points_t_copy(player_graphics2d_cmd_points_t *dest, const player_graphics2d_cmd_points_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->points_count,&src->points_count,sizeof(uint32_t)*1); 
  if(src->points != NULL && src->points_count > 0)
  {
    if((dest->points = malloc(src->points_count*sizeof(player_point_2d_t))) == NULL)
      return(0);
  }
  else
    dest->points = NULL;
  size += sizeof(player_point_2d_t)*src->points_count;
  memcpy(dest->points,src->points,sizeof(player_point_2d_t)*src->points_count); 
  size += sizeof(player_color_t)*1;
  memcpy(&dest->color,&src->color,sizeof(player_color_t)*1); 
  return(size);
}
void player_graphics2d_cmd_points_t_cleanup(const player_graphics2d_cmd_points_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->points); 
}
player_graphics2d_cmd_points_t * player_graphics2d_cmd_points_t_clone(const player_graphics2d_cmd_points_t *msg)
{      
  player_graphics2d_cmd_points_t * clone = malloc(sizeof(player_graphics2d_cmd_points_t));
  if (clone)
    player_graphics2d_cmd_points_t_copy(clone,msg);
  return clone;
}
void player_graphics2d_cmd_points_t_free(player_graphics2d_cmd_points_t *msg)
{      
  player_graphics2d_cmd_points_t_cleanup(msg);
  free(msg);
}
unsigned int player_graphics2d_cmd_points_t_sizeof(player_graphics2d_cmd_points_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_point_2d_t)*msg->points_count; 
  size += sizeof(player_color_t)*1; 
  return(size);
}

int xdr_player_graphics2d_cmd_polyline_t (XDR* xdrs, player_graphics2d_cmd_polyline_t * msg)
{   if(xdr_u_int(xdrs,&msg->points_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->points = malloc(msg->points_count*sizeof(player_point_2d_t))) == NULL)
      return(0);
  }
  {
    player_point_2d_t* points_p = msg->points;
    if(xdr_array(xdrs, (char**)&points_p, &msg->points_count, msg->points_count, sizeof(player_point_2d_t), (xdrproc_t)xdr_player_point_2d_t) != 1)
      return(0);
  }
  if(xdr_player_color_t(xdrs,&msg->color) != 1)
    return(0);
  return(1);
}
int 
player_graphics2d_cmd_polyline_pack(void* buf, size_t buflen, player_graphics2d_cmd_polyline_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_graphics2d_cmd_polyline_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_graphics2d_cmd_polyline_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_graphics2d_cmd_polyline_t_copy(player_graphics2d_cmd_polyline_t *dest, const player_graphics2d_cmd_polyline_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->points_count,&src->points_count,sizeof(uint32_t)*1); 
  if(src->points != NULL && src->points_count > 0)
  {
    if((dest->points = malloc(src->points_count*sizeof(player_point_2d_t))) == NULL)
      return(0);
  }
  else
    dest->points = NULL;
  size += sizeof(player_point_2d_t)*src->points_count;
  memcpy(dest->points,src->points,sizeof(player_point_2d_t)*src->points_count); 
  size += sizeof(player_color_t)*1;
  memcpy(&dest->color,&src->color,sizeof(player_color_t)*1); 
  return(size);
}
void player_graphics2d_cmd_polyline_t_cleanup(const player_graphics2d_cmd_polyline_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->points); 
}
player_graphics2d_cmd_polyline_t * player_graphics2d_cmd_polyline_t_clone(const player_graphics2d_cmd_polyline_t *msg)
{      
  player_graphics2d_cmd_polyline_t * clone = malloc(sizeof(player_graphics2d_cmd_polyline_t));
  if (clone)
    player_graphics2d_cmd_polyline_t_copy(clone,msg);
  return clone;
}
void player_graphics2d_cmd_polyline_t_free(player_graphics2d_cmd_polyline_t *msg)
{      
  player_graphics2d_cmd_polyline_t_cleanup(msg);
  free(msg);
}
unsigned int player_graphics2d_cmd_polyline_t_sizeof(player_graphics2d_cmd_polyline_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_point_2d_t)*msg->points_count; 
  size += sizeof(player_color_t)*1; 
  return(size);
}

int xdr_player_graphics2d_cmd_polygon_t (XDR* xdrs, player_graphics2d_cmd_polygon_t * msg)
{   if(xdr_u_int(xdrs,&msg->points_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->points = malloc(msg->points_count*sizeof(player_point_2d_t))) == NULL)
      return(0);
  }
  {
    player_point_2d_t* points_p = msg->points;
    if(xdr_array(xdrs, (char**)&points_p, &msg->points_count, msg->points_count, sizeof(player_point_2d_t), (xdrproc_t)xdr_player_point_2d_t) != 1)
      return(0);
  }
  if(xdr_player_color_t(xdrs,&msg->color) != 1)
    return(0);
  if(xdr_player_color_t(xdrs,&msg->fill_color) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->filled) != 1)
    return(0);
  return(1);
}
int 
player_graphics2d_cmd_polygon_pack(void* buf, size_t buflen, player_graphics2d_cmd_polygon_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_graphics2d_cmd_polygon_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_graphics2d_cmd_polygon_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_graphics2d_cmd_polygon_t_copy(player_graphics2d_cmd_polygon_t *dest, const player_graphics2d_cmd_polygon_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->points_count,&src->points_count,sizeof(uint32_t)*1); 
  if(src->points != NULL && src->points_count > 0)
  {
    if((dest->points = malloc(src->points_count*sizeof(player_point_2d_t))) == NULL)
      return(0);
  }
  else
    dest->points = NULL;
  size += sizeof(player_point_2d_t)*src->points_count;
  memcpy(dest->points,src->points,sizeof(player_point_2d_t)*src->points_count); 
  size += sizeof(player_color_t)*1;
  memcpy(&dest->color,&src->color,sizeof(player_color_t)*1); 
  size += sizeof(player_color_t)*1;
  memcpy(&dest->fill_color,&src->fill_color,sizeof(player_color_t)*1); 
  size += sizeof(uint8_t)*1;
  memcpy(&dest->filled,&src->filled,sizeof(uint8_t)*1); 
  return(size);
}
void player_graphics2d_cmd_polygon_t_cleanup(const player_graphics2d_cmd_polygon_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->points); 
}
player_graphics2d_cmd_polygon_t * player_graphics2d_cmd_polygon_t_clone(const player_graphics2d_cmd_polygon_t *msg)
{      
  player_graphics2d_cmd_polygon_t * clone = malloc(sizeof(player_graphics2d_cmd_polygon_t));
  if (clone)
    player_graphics2d_cmd_polygon_t_copy(clone,msg);
  return clone;
}
void player_graphics2d_cmd_polygon_t_free(player_graphics2d_cmd_polygon_t *msg)
{      
  player_graphics2d_cmd_polygon_t_cleanup(msg);
  free(msg);
}
unsigned int player_graphics2d_cmd_polygon_t_sizeof(player_graphics2d_cmd_polygon_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_point_2d_t)*msg->points_count; 
  size += sizeof(player_color_t)*1; 
  size += sizeof(player_color_t)*1; 
  size += sizeof(uint8_t)*1; 
  return(size);
}

int xdr_player_speech_cmd_t (XDR* xdrs, player_speech_cmd_t * msg)
{   if(xdr_u_int(xdrs,&msg->string_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->string = malloc(msg->string_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* string_p = msg->string;
    if(xdr_bytes(xdrs, (char**)&string_p, &msg->string_count, msg->string_count) != 1)
      return(0);
  }
  return(1);
}
int 
player_speech_cmd_pack(void* buf, size_t buflen, player_speech_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_speech_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_speech_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_speech_cmd_t_copy(player_speech_cmd_t *dest, const player_speech_cmd_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->string_count,&src->string_count,sizeof(uint32_t)*1); 
  if(src->string != NULL && src->string_count > 0)
  {
    if((dest->string = malloc(src->string_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->string = NULL;
  size += sizeof(char)*src->string_count;
  memcpy(dest->string,src->string,sizeof(char)*src->string_count); 
  return(size);
}
void player_speech_cmd_t_cleanup(const player_speech_cmd_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->string); 
}
player_speech_cmd_t * player_speech_cmd_t_clone(const player_speech_cmd_t *msg)
{      
  player_speech_cmd_t * clone = malloc(sizeof(player_speech_cmd_t));
  if (clone)
    player_speech_cmd_t_copy(clone,msg);
  return clone;
}
void player_speech_cmd_t_free(player_speech_cmd_t *msg)
{      
  player_speech_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_speech_cmd_t_sizeof(player_speech_cmd_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->string_count; 
  return(size);
}

int xdr_player_position3d_data_t (XDR* xdrs, player_position3d_data_t * msg)
{   if(xdr_player_pose3d_t(xdrs,&msg->pos) != 1)
    return(0);
  if(xdr_player_pose3d_t(xdrs,&msg->vel) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->stall) != 1)
    return(0);
  return(1);
}
int 
player_position3d_data_pack(void* buf, size_t buflen, player_position3d_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position3d_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position3d_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position3d_data_t_copy(player_position3d_data_t *dest, const player_position3d_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position3d_data_t));
  return sizeof(player_position3d_data_t);
} 
void player_position3d_data_t_cleanup(const player_position3d_data_t *msg)
{
} 
player_position3d_data_t * player_position3d_data_t_clone(const player_position3d_data_t *msg)
{      
  player_position3d_data_t * clone = malloc(sizeof(player_position3d_data_t));
  if (clone)
    player_position3d_data_t_copy(clone,msg);
  return clone;
}
void player_position3d_data_t_free(player_position3d_data_t *msg)
{      
  player_position3d_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_position3d_data_t_sizeof(player_position3d_data_t *msg)
{
  return sizeof(player_position3d_data_t);
} 

int xdr_player_position3d_cmd_pos_t (XDR* xdrs, player_position3d_cmd_pos_t * msg)
{   if(xdr_player_pose3d_t(xdrs,&msg->pos) != 1)
    return(0);
  if(xdr_player_pose3d_t(xdrs,&msg->vel) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_position3d_cmd_pos_pack(void* buf, size_t buflen, player_position3d_cmd_pos_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position3d_cmd_pos_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position3d_cmd_pos_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position3d_cmd_pos_t_copy(player_position3d_cmd_pos_t *dest, const player_position3d_cmd_pos_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position3d_cmd_pos_t));
  return sizeof(player_position3d_cmd_pos_t);
} 
void player_position3d_cmd_pos_t_cleanup(const player_position3d_cmd_pos_t *msg)
{
} 
player_position3d_cmd_pos_t * player_position3d_cmd_pos_t_clone(const player_position3d_cmd_pos_t *msg)
{      
  player_position3d_cmd_pos_t * clone = malloc(sizeof(player_position3d_cmd_pos_t));
  if (clone)
    player_position3d_cmd_pos_t_copy(clone,msg);
  return clone;
}
void player_position3d_cmd_pos_t_free(player_position3d_cmd_pos_t *msg)
{      
  player_position3d_cmd_pos_t_cleanup(msg);
  free(msg);
}
unsigned int player_position3d_cmd_pos_t_sizeof(player_position3d_cmd_pos_t *msg)
{
  return sizeof(player_position3d_cmd_pos_t);
} 

int xdr_player_position3d_cmd_vel_t (XDR* xdrs, player_position3d_cmd_vel_t * msg)
{   if(xdr_player_pose3d_t(xdrs,&msg->vel) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_position3d_cmd_vel_pack(void* buf, size_t buflen, player_position3d_cmd_vel_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position3d_cmd_vel_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position3d_cmd_vel_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position3d_cmd_vel_t_copy(player_position3d_cmd_vel_t *dest, const player_position3d_cmd_vel_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position3d_cmd_vel_t));
  return sizeof(player_position3d_cmd_vel_t);
} 
void player_position3d_cmd_vel_t_cleanup(const player_position3d_cmd_vel_t *msg)
{
} 
player_position3d_cmd_vel_t * player_position3d_cmd_vel_t_clone(const player_position3d_cmd_vel_t *msg)
{      
  player_position3d_cmd_vel_t * clone = malloc(sizeof(player_position3d_cmd_vel_t));
  if (clone)
    player_position3d_cmd_vel_t_copy(clone,msg);
  return clone;
}
void player_position3d_cmd_vel_t_free(player_position3d_cmd_vel_t *msg)
{      
  player_position3d_cmd_vel_t_cleanup(msg);
  free(msg);
}
unsigned int player_position3d_cmd_vel_t_sizeof(player_position3d_cmd_vel_t *msg)
{
  return sizeof(player_position3d_cmd_vel_t);
} 

int xdr_player_position3d_geom_t (XDR* xdrs, player_position3d_geom_t * msg)
{   if(xdr_player_pose3d_t(xdrs,&msg->pose) != 1)
    return(0);
  if(xdr_player_bbox3d_t(xdrs,&msg->size) != 1)
    return(0);
  return(1);
}
int 
player_position3d_geom_pack(void* buf, size_t buflen, player_position3d_geom_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position3d_geom_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position3d_geom_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position3d_geom_t_copy(player_position3d_geom_t *dest, const player_position3d_geom_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position3d_geom_t));
  return sizeof(player_position3d_geom_t);
} 
void player_position3d_geom_t_cleanup(const player_position3d_geom_t *msg)
{
} 
player_position3d_geom_t * player_position3d_geom_t_clone(const player_position3d_geom_t *msg)
{      
  player_position3d_geom_t * clone = malloc(sizeof(player_position3d_geom_t));
  if (clone)
    player_position3d_geom_t_copy(clone,msg);
  return clone;
}
void player_position3d_geom_t_free(player_position3d_geom_t *msg)
{      
  player_position3d_geom_t_cleanup(msg);
  free(msg);
}
unsigned int player_position3d_geom_t_sizeof(player_position3d_geom_t *msg)
{
  return sizeof(player_position3d_geom_t);
} 

int xdr_player_position3d_power_config_t (XDR* xdrs, player_position3d_power_config_t * msg)
{   if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_position3d_power_config_pack(void* buf, size_t buflen, player_position3d_power_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position3d_power_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position3d_power_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position3d_power_config_t_copy(player_position3d_power_config_t *dest, const player_position3d_power_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position3d_power_config_t));
  return sizeof(player_position3d_power_config_t);
} 
void player_position3d_power_config_t_cleanup(const player_position3d_power_config_t *msg)
{
} 
player_position3d_power_config_t * player_position3d_power_config_t_clone(const player_position3d_power_config_t *msg)
{      
  player_position3d_power_config_t * clone = malloc(sizeof(player_position3d_power_config_t));
  if (clone)
    player_position3d_power_config_t_copy(clone,msg);
  return clone;
}
void player_position3d_power_config_t_free(player_position3d_power_config_t *msg)
{      
  player_position3d_power_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_position3d_power_config_t_sizeof(player_position3d_power_config_t *msg)
{
  return sizeof(player_position3d_power_config_t);
} 

int xdr_player_position3d_position_mode_req_t (XDR* xdrs, player_position3d_position_mode_req_t * msg)
{   if(xdr_u_int(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_position3d_position_mode_req_pack(void* buf, size_t buflen, player_position3d_position_mode_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position3d_position_mode_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position3d_position_mode_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position3d_position_mode_req_t_copy(player_position3d_position_mode_req_t *dest, const player_position3d_position_mode_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position3d_position_mode_req_t));
  return sizeof(player_position3d_position_mode_req_t);
} 
void player_position3d_position_mode_req_t_cleanup(const player_position3d_position_mode_req_t *msg)
{
} 
player_position3d_position_mode_req_t * player_position3d_position_mode_req_t_clone(const player_position3d_position_mode_req_t *msg)
{      
  player_position3d_position_mode_req_t * clone = malloc(sizeof(player_position3d_position_mode_req_t));
  if (clone)
    player_position3d_position_mode_req_t_copy(clone,msg);
  return clone;
}
void player_position3d_position_mode_req_t_free(player_position3d_position_mode_req_t *msg)
{      
  player_position3d_position_mode_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position3d_position_mode_req_t_sizeof(player_position3d_position_mode_req_t *msg)
{
  return sizeof(player_position3d_position_mode_req_t);
} 

int xdr_player_position3d_velocity_mode_config_t (XDR* xdrs, player_position3d_velocity_mode_config_t * msg)
{   if(xdr_u_int(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_position3d_velocity_mode_config_pack(void* buf, size_t buflen, player_position3d_velocity_mode_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position3d_velocity_mode_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position3d_velocity_mode_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position3d_velocity_mode_config_t_copy(player_position3d_velocity_mode_config_t *dest, const player_position3d_velocity_mode_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position3d_velocity_mode_config_t));
  return sizeof(player_position3d_velocity_mode_config_t);
} 
void player_position3d_velocity_mode_config_t_cleanup(const player_position3d_velocity_mode_config_t *msg)
{
} 
player_position3d_velocity_mode_config_t * player_position3d_velocity_mode_config_t_clone(const player_position3d_velocity_mode_config_t *msg)
{      
  player_position3d_velocity_mode_config_t * clone = malloc(sizeof(player_position3d_velocity_mode_config_t));
  if (clone)
    player_position3d_velocity_mode_config_t_copy(clone,msg);
  return clone;
}
void player_position3d_velocity_mode_config_t_free(player_position3d_velocity_mode_config_t *msg)
{      
  player_position3d_velocity_mode_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_position3d_velocity_mode_config_t_sizeof(player_position3d_velocity_mode_config_t *msg)
{
  return sizeof(player_position3d_velocity_mode_config_t);
} 

int xdr_player_position3d_set_odom_req_t (XDR* xdrs, player_position3d_set_odom_req_t * msg)
{   if(xdr_player_pose3d_t(xdrs,&msg->pos) != 1)
    return(0);
  return(1);
}
int 
player_position3d_set_odom_req_pack(void* buf, size_t buflen, player_position3d_set_odom_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position3d_set_odom_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position3d_set_odom_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position3d_set_odom_req_t_copy(player_position3d_set_odom_req_t *dest, const player_position3d_set_odom_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position3d_set_odom_req_t));
  return sizeof(player_position3d_set_odom_req_t);
} 
void player_position3d_set_odom_req_t_cleanup(const player_position3d_set_odom_req_t *msg)
{
} 
player_position3d_set_odom_req_t * player_position3d_set_odom_req_t_clone(const player_position3d_set_odom_req_t *msg)
{      
  player_position3d_set_odom_req_t * clone = malloc(sizeof(player_position3d_set_odom_req_t));
  if (clone)
    player_position3d_set_odom_req_t_copy(clone,msg);
  return clone;
}
void player_position3d_set_odom_req_t_free(player_position3d_set_odom_req_t *msg)
{      
  player_position3d_set_odom_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position3d_set_odom_req_t_sizeof(player_position3d_set_odom_req_t *msg)
{
  return sizeof(player_position3d_set_odom_req_t);
} 

int xdr_player_position3d_reset_odom_config_t (XDR* xdrs, player_position3d_reset_odom_config_t * msg)
{   return(1);
}
int 
player_position3d_reset_odom_config_pack(void* buf, size_t buflen, player_position3d_reset_odom_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position3d_reset_odom_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position3d_reset_odom_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position3d_reset_odom_config_t_copy(player_position3d_reset_odom_config_t *dest, const player_position3d_reset_odom_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position3d_reset_odom_config_t));
  return sizeof(player_position3d_reset_odom_config_t);
} 
void player_position3d_reset_odom_config_t_cleanup(const player_position3d_reset_odom_config_t *msg)
{
} 
player_position3d_reset_odom_config_t * player_position3d_reset_odom_config_t_clone(const player_position3d_reset_odom_config_t *msg)
{      
  player_position3d_reset_odom_config_t * clone = malloc(sizeof(player_position3d_reset_odom_config_t));
  if (clone)
    player_position3d_reset_odom_config_t_copy(clone,msg);
  return clone;
}
void player_position3d_reset_odom_config_t_free(player_position3d_reset_odom_config_t *msg)
{      
  player_position3d_reset_odom_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_position3d_reset_odom_config_t_sizeof(player_position3d_reset_odom_config_t *msg)
{
  return sizeof(player_position3d_reset_odom_config_t);
} 

int xdr_player_position3d_speed_pid_req_t (XDR* xdrs, player_position3d_speed_pid_req_t * msg)
{   if(xdr_float(xdrs,&msg->kp) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->ki) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->kd) != 1)
    return(0);
  return(1);
}
int 
player_position3d_speed_pid_req_pack(void* buf, size_t buflen, player_position3d_speed_pid_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position3d_speed_pid_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position3d_speed_pid_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position3d_speed_pid_req_t_copy(player_position3d_speed_pid_req_t *dest, const player_position3d_speed_pid_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position3d_speed_pid_req_t));
  return sizeof(player_position3d_speed_pid_req_t);
} 
void player_position3d_speed_pid_req_t_cleanup(const player_position3d_speed_pid_req_t *msg)
{
} 
player_position3d_speed_pid_req_t * player_position3d_speed_pid_req_t_clone(const player_position3d_speed_pid_req_t *msg)
{      
  player_position3d_speed_pid_req_t * clone = malloc(sizeof(player_position3d_speed_pid_req_t));
  if (clone)
    player_position3d_speed_pid_req_t_copy(clone,msg);
  return clone;
}
void player_position3d_speed_pid_req_t_free(player_position3d_speed_pid_req_t *msg)
{      
  player_position3d_speed_pid_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position3d_speed_pid_req_t_sizeof(player_position3d_speed_pid_req_t *msg)
{
  return sizeof(player_position3d_speed_pid_req_t);
} 

int xdr_player_position3d_position_pid_req_t (XDR* xdrs, player_position3d_position_pid_req_t * msg)
{   if(xdr_float(xdrs,&msg->kp) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->ki) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->kd) != 1)
    return(0);
  return(1);
}
int 
player_position3d_position_pid_req_pack(void* buf, size_t buflen, player_position3d_position_pid_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position3d_position_pid_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position3d_position_pid_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position3d_position_pid_req_t_copy(player_position3d_position_pid_req_t *dest, const player_position3d_position_pid_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position3d_position_pid_req_t));
  return sizeof(player_position3d_position_pid_req_t);
} 
void player_position3d_position_pid_req_t_cleanup(const player_position3d_position_pid_req_t *msg)
{
} 
player_position3d_position_pid_req_t * player_position3d_position_pid_req_t_clone(const player_position3d_position_pid_req_t *msg)
{      
  player_position3d_position_pid_req_t * clone = malloc(sizeof(player_position3d_position_pid_req_t));
  if (clone)
    player_position3d_position_pid_req_t_copy(clone,msg);
  return clone;
}
void player_position3d_position_pid_req_t_free(player_position3d_position_pid_req_t *msg)
{      
  player_position3d_position_pid_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position3d_position_pid_req_t_sizeof(player_position3d_position_pid_req_t *msg)
{
  return sizeof(player_position3d_position_pid_req_t);
} 

int xdr_player_position3d_speed_prof_req_t (XDR* xdrs, player_position3d_speed_prof_req_t * msg)
{   if(xdr_float(xdrs,&msg->speed) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->acc) != 1)
    return(0);
  return(1);
}
int 
player_position3d_speed_prof_req_pack(void* buf, size_t buflen, player_position3d_speed_prof_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position3d_speed_prof_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position3d_speed_prof_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position3d_speed_prof_req_t_copy(player_position3d_speed_prof_req_t *dest, const player_position3d_speed_prof_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position3d_speed_prof_req_t));
  return sizeof(player_position3d_speed_prof_req_t);
} 
void player_position3d_speed_prof_req_t_cleanup(const player_position3d_speed_prof_req_t *msg)
{
} 
player_position3d_speed_prof_req_t * player_position3d_speed_prof_req_t_clone(const player_position3d_speed_prof_req_t *msg)
{      
  player_position3d_speed_prof_req_t * clone = malloc(sizeof(player_position3d_speed_prof_req_t));
  if (clone)
    player_position3d_speed_prof_req_t_copy(clone,msg);
  return clone;
}
void player_position3d_speed_prof_req_t_free(player_position3d_speed_prof_req_t *msg)
{      
  player_position3d_speed_prof_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position3d_speed_prof_req_t_sizeof(player_position3d_speed_prof_req_t *msg)
{
  return sizeof(player_position3d_speed_prof_req_t);
} 

int xdr_player_sonar_data_t (XDR* xdrs, player_sonar_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->ranges_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->ranges = malloc(msg->ranges_count*sizeof(float))) == NULL)
      return(0);
  }
  {
    float* ranges_p = msg->ranges;
    if(xdr_array(xdrs, (char**)&ranges_p, &msg->ranges_count, msg->ranges_count, sizeof(float), (xdrproc_t)xdr_float) != 1)
      return(0);
  }
  return(1);
}
int 
player_sonar_data_pack(void* buf, size_t buflen, player_sonar_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_sonar_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_sonar_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_sonar_data_t_copy(player_sonar_data_t *dest, const player_sonar_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->ranges_count,&src->ranges_count,sizeof(uint32_t)*1); 
  if(src->ranges != NULL && src->ranges_count > 0)
  {
    if((dest->ranges = malloc(src->ranges_count*sizeof(float))) == NULL)
      return(0);
  }
  else
    dest->ranges = NULL;
  size += sizeof(float)*src->ranges_count;
  memcpy(dest->ranges,src->ranges,sizeof(float)*src->ranges_count); 
  return(size);
}
void player_sonar_data_t_cleanup(const player_sonar_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->ranges); 
}
player_sonar_data_t * player_sonar_data_t_clone(const player_sonar_data_t *msg)
{      
  player_sonar_data_t * clone = malloc(sizeof(player_sonar_data_t));
  if (clone)
    player_sonar_data_t_copy(clone,msg);
  return clone;
}
void player_sonar_data_t_free(player_sonar_data_t *msg)
{      
  player_sonar_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_sonar_data_t_sizeof(player_sonar_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(float)*msg->ranges_count; 
  return(size);
}

int xdr_player_sonar_geom_t (XDR* xdrs, player_sonar_geom_t * msg)
{   if(xdr_u_int(xdrs,&msg->poses_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->poses = malloc(msg->poses_count*sizeof(player_pose3d_t))) == NULL)
      return(0);
  }
  {
    player_pose3d_t* poses_p = msg->poses;
    if(xdr_array(xdrs, (char**)&poses_p, &msg->poses_count, msg->poses_count, sizeof(player_pose3d_t), (xdrproc_t)xdr_player_pose3d_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_sonar_geom_pack(void* buf, size_t buflen, player_sonar_geom_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_sonar_geom_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_sonar_geom_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_sonar_geom_t_copy(player_sonar_geom_t *dest, const player_sonar_geom_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->poses_count,&src->poses_count,sizeof(uint32_t)*1); 
  if(src->poses != NULL && src->poses_count > 0)
  {
    if((dest->poses = malloc(src->poses_count*sizeof(player_pose3d_t))) == NULL)
      return(0);
  }
  else
    dest->poses = NULL;
  size += sizeof(player_pose3d_t)*src->poses_count;
  memcpy(dest->poses,src->poses,sizeof(player_pose3d_t)*src->poses_count); 
  return(size);
}
void player_sonar_geom_t_cleanup(const player_sonar_geom_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->poses); 
}
player_sonar_geom_t * player_sonar_geom_t_clone(const player_sonar_geom_t *msg)
{      
  player_sonar_geom_t * clone = malloc(sizeof(player_sonar_geom_t));
  if (clone)
    player_sonar_geom_t_copy(clone,msg);
  return clone;
}
void player_sonar_geom_t_free(player_sonar_geom_t *msg)
{      
  player_sonar_geom_t_cleanup(msg);
  free(msg);
}
unsigned int player_sonar_geom_t_sizeof(player_sonar_geom_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_pose3d_t)*msg->poses_count; 
  return(size);
}

int xdr_player_sonar_power_config_t (XDR* xdrs, player_sonar_power_config_t * msg)
{   if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_sonar_power_config_pack(void* buf, size_t buflen, player_sonar_power_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_sonar_power_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_sonar_power_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_sonar_power_config_t_copy(player_sonar_power_config_t *dest, const player_sonar_power_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_sonar_power_config_t));
  return sizeof(player_sonar_power_config_t);
} 
void player_sonar_power_config_t_cleanup(const player_sonar_power_config_t *msg)
{
} 
player_sonar_power_config_t * player_sonar_power_config_t_clone(const player_sonar_power_config_t *msg)
{      
  player_sonar_power_config_t * clone = malloc(sizeof(player_sonar_power_config_t));
  if (clone)
    player_sonar_power_config_t_copy(clone,msg);
  return clone;
}
void player_sonar_power_config_t_free(player_sonar_power_config_t *msg)
{      
  player_sonar_power_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_sonar_power_config_t_sizeof(player_sonar_power_config_t *msg)
{
  return sizeof(player_sonar_power_config_t);
} 

int xdr_player_health_cpu_t (XDR* xdrs, player_health_cpu_t * msg)
{   if(xdr_float(xdrs,&msg->idle) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->system) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->user) != 1)
    return(0);
  return(1);
}
int 
player_health_cpu_pack(void* buf, size_t buflen, player_health_cpu_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_health_cpu_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_health_cpu_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_health_cpu_t_copy(player_health_cpu_t *dest, const player_health_cpu_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_health_cpu_t));
  return sizeof(player_health_cpu_t);
} 
void player_health_cpu_t_cleanup(const player_health_cpu_t *msg)
{
} 
player_health_cpu_t * player_health_cpu_t_clone(const player_health_cpu_t *msg)
{      
  player_health_cpu_t * clone = malloc(sizeof(player_health_cpu_t));
  if (clone)
    player_health_cpu_t_copy(clone,msg);
  return clone;
}
void player_health_cpu_t_free(player_health_cpu_t *msg)
{      
  player_health_cpu_t_cleanup(msg);
  free(msg);
}
unsigned int player_health_cpu_t_sizeof(player_health_cpu_t *msg)
{
  return sizeof(player_health_cpu_t);
} 

int xdr_player_health_memory_t (XDR* xdrs, player_health_memory_t * msg)
{   if(xdr_longlong_t(xdrs,&msg->total) != 1)
    return(0);
  if(xdr_longlong_t(xdrs,&msg->used) != 1)
    return(0);
  if(xdr_longlong_t(xdrs,&msg->free) != 1)
    return(0);
  return(1);
}
int 
player_health_memory_pack(void* buf, size_t buflen, player_health_memory_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_health_memory_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_health_memory_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_health_memory_t_copy(player_health_memory_t *dest, const player_health_memory_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_health_memory_t));
  return sizeof(player_health_memory_t);
} 
void player_health_memory_t_cleanup(const player_health_memory_t *msg)
{
} 
player_health_memory_t * player_health_memory_t_clone(const player_health_memory_t *msg)
{      
  player_health_memory_t * clone = malloc(sizeof(player_health_memory_t));
  if (clone)
    player_health_memory_t_copy(clone,msg);
  return clone;
}
void player_health_memory_t_free(player_health_memory_t *msg)
{      
  player_health_memory_t_cleanup(msg);
  free(msg);
}
unsigned int player_health_memory_t_sizeof(player_health_memory_t *msg)
{
  return sizeof(player_health_memory_t);
} 

int xdr_player_health_data_t (XDR* xdrs, player_health_data_t * msg)
{   if(xdr_player_health_cpu_t(xdrs,&msg->cpu_usage) != 1)
    return(0);
  if(xdr_player_health_memory_t(xdrs,&msg->mem) != 1)
    return(0);
  if(xdr_player_health_memory_t(xdrs,&msg->swap) != 1)
    return(0);
  return(1);
}
int 
player_health_data_pack(void* buf, size_t buflen, player_health_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_health_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_health_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_health_data_t_copy(player_health_data_t *dest, const player_health_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_health_data_t));
  return sizeof(player_health_data_t);
} 
void player_health_data_t_cleanup(const player_health_data_t *msg)
{
} 
player_health_data_t * player_health_data_t_clone(const player_health_data_t *msg)
{      
  player_health_data_t * clone = malloc(sizeof(player_health_data_t));
  if (clone)
    player_health_data_t_copy(clone,msg);
  return clone;
}
void player_health_data_t_free(player_health_data_t *msg)
{      
  player_health_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_health_data_t_sizeof(player_health_data_t *msg)
{
  return sizeof(player_health_data_t);
} 

int xdr_player_audio_wav_t (XDR* xdrs, player_audio_wav_t * msg)
{   if(xdr_u_int(xdrs,&msg->data_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->data = malloc(msg->data_count*sizeof(uint8_t))) == NULL)
      return(0);
  }
  {
    uint8_t* data_p = msg->data;
    if(xdr_bytes(xdrs, (char**)&data_p, &msg->data_count, msg->data_count) != 1)
      return(0);
  }
  if(xdr_u_int(xdrs,&msg->format) != 1)
    return(0);
  return(1);
}
int 
player_audio_wav_pack(void* buf, size_t buflen, player_audio_wav_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_audio_wav_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_audio_wav_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_audio_wav_t_copy(player_audio_wav_t *dest, const player_audio_wav_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->data_count,&src->data_count,sizeof(uint32_t)*1); 
  if(src->data != NULL && src->data_count > 0)
  {
    if((dest->data = malloc(src->data_count*sizeof(uint8_t))) == NULL)
      return(0);
  }
  else
    dest->data = NULL;
  size += sizeof(uint8_t)*src->data_count;
  memcpy(dest->data,src->data,sizeof(uint8_t)*src->data_count); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->format,&src->format,sizeof(uint32_t)*1); 
  return(size);
}
void player_audio_wav_t_cleanup(const player_audio_wav_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->data); 
}
player_audio_wav_t * player_audio_wav_t_clone(const player_audio_wav_t *msg)
{      
  player_audio_wav_t * clone = malloc(sizeof(player_audio_wav_t));
  if (clone)
    player_audio_wav_t_copy(clone,msg);
  return clone;
}
void player_audio_wav_t_free(player_audio_wav_t *msg)
{      
  player_audio_wav_t_cleanup(msg);
  free(msg);
}
unsigned int player_audio_wav_t_sizeof(player_audio_wav_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint8_t)*msg->data_count; 
  size += sizeof(uint32_t)*1; 
  return(size);
}

int xdr_player_audio_seq_item_t (XDR* xdrs, player_audio_seq_item_t * msg)
{   if(xdr_float(xdrs,&msg->freq) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->duration) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->amplitude) != 1)
    return(0);
  if(xdr_player_bool_t(xdrs,&msg->link) != 1)
    return(0);
  return(1);
}
int 
player_audio_seq_item_pack(void* buf, size_t buflen, player_audio_seq_item_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_audio_seq_item_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_audio_seq_item_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_audio_seq_item_t_copy(player_audio_seq_item_t *dest, const player_audio_seq_item_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_audio_seq_item_t));
  return sizeof(player_audio_seq_item_t);
} 
void player_audio_seq_item_t_cleanup(const player_audio_seq_item_t *msg)
{
} 
player_audio_seq_item_t * player_audio_seq_item_t_clone(const player_audio_seq_item_t *msg)
{      
  player_audio_seq_item_t * clone = malloc(sizeof(player_audio_seq_item_t));
  if (clone)
    player_audio_seq_item_t_copy(clone,msg);
  return clone;
}
void player_audio_seq_item_t_free(player_audio_seq_item_t *msg)
{      
  player_audio_seq_item_t_cleanup(msg);
  free(msg);
}
unsigned int player_audio_seq_item_t_sizeof(player_audio_seq_item_t *msg)
{
  return sizeof(player_audio_seq_item_t);
} 

int xdr_player_audio_seq_t (XDR* xdrs, player_audio_seq_t * msg)
{   if(xdr_u_int(xdrs,&msg->tones_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->tones = malloc(msg->tones_count*sizeof(player_audio_seq_item_t))) == NULL)
      return(0);
  }
  {
    player_audio_seq_item_t* tones_p = msg->tones;
    if(xdr_array(xdrs, (char**)&tones_p, &msg->tones_count, msg->tones_count, sizeof(player_audio_seq_item_t), (xdrproc_t)xdr_player_audio_seq_item_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_audio_seq_pack(void* buf, size_t buflen, player_audio_seq_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_audio_seq_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_audio_seq_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_audio_seq_t_copy(player_audio_seq_t *dest, const player_audio_seq_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->tones_count,&src->tones_count,sizeof(uint32_t)*1); 
  if(src->tones != NULL && src->tones_count > 0)
  {
    if((dest->tones = malloc(src->tones_count*sizeof(player_audio_seq_item_t))) == NULL)
      return(0);
  }
  else
    dest->tones = NULL;
  size += sizeof(player_audio_seq_item_t)*src->tones_count;
  memcpy(dest->tones,src->tones,sizeof(player_audio_seq_item_t)*src->tones_count); 
  return(size);
}
void player_audio_seq_t_cleanup(const player_audio_seq_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->tones); 
}
player_audio_seq_t * player_audio_seq_t_clone(const player_audio_seq_t *msg)
{      
  player_audio_seq_t * clone = malloc(sizeof(player_audio_seq_t));
  if (clone)
    player_audio_seq_t_copy(clone,msg);
  return clone;
}
void player_audio_seq_t_free(player_audio_seq_t *msg)
{      
  player_audio_seq_t_cleanup(msg);
  free(msg);
}
unsigned int player_audio_seq_t_sizeof(player_audio_seq_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_audio_seq_item_t)*msg->tones_count; 
  return(size);
}

int xdr_player_audio_mixer_channel_t (XDR* xdrs, player_audio_mixer_channel_t * msg)
{   if(xdr_float(xdrs,&msg->amplitude) != 1)
    return(0);
  if(xdr_player_bool_t(xdrs,&msg->active) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->index) != 1)
    return(0);
  return(1);
}
int 
player_audio_mixer_channel_pack(void* buf, size_t buflen, player_audio_mixer_channel_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_audio_mixer_channel_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_audio_mixer_channel_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_audio_mixer_channel_t_copy(player_audio_mixer_channel_t *dest, const player_audio_mixer_channel_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_audio_mixer_channel_t));
  return sizeof(player_audio_mixer_channel_t);
} 
void player_audio_mixer_channel_t_cleanup(const player_audio_mixer_channel_t *msg)
{
} 
player_audio_mixer_channel_t * player_audio_mixer_channel_t_clone(const player_audio_mixer_channel_t *msg)
{      
  player_audio_mixer_channel_t * clone = malloc(sizeof(player_audio_mixer_channel_t));
  if (clone)
    player_audio_mixer_channel_t_copy(clone,msg);
  return clone;
}
void player_audio_mixer_channel_t_free(player_audio_mixer_channel_t *msg)
{      
  player_audio_mixer_channel_t_cleanup(msg);
  free(msg);
}
unsigned int player_audio_mixer_channel_t_sizeof(player_audio_mixer_channel_t *msg)
{
  return sizeof(player_audio_mixer_channel_t);
} 

int xdr_player_audio_mixer_channel_list_t (XDR* xdrs, player_audio_mixer_channel_list_t * msg)
{   if(xdr_u_int(xdrs,&msg->channels_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->channels = malloc(msg->channels_count*sizeof(player_audio_mixer_channel_t))) == NULL)
      return(0);
  }
  {
    player_audio_mixer_channel_t* channels_p = msg->channels;
    if(xdr_array(xdrs, (char**)&channels_p, &msg->channels_count, msg->channels_count, sizeof(player_audio_mixer_channel_t), (xdrproc_t)xdr_player_audio_mixer_channel_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_audio_mixer_channel_list_pack(void* buf, size_t buflen, player_audio_mixer_channel_list_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_audio_mixer_channel_list_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_audio_mixer_channel_list_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_audio_mixer_channel_list_t_copy(player_audio_mixer_channel_list_t *dest, const player_audio_mixer_channel_list_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->channels_count,&src->channels_count,sizeof(uint32_t)*1); 
  if(src->channels != NULL && src->channels_count > 0)
  {
    if((dest->channels = malloc(src->channels_count*sizeof(player_audio_mixer_channel_t))) == NULL)
      return(0);
  }
  else
    dest->channels = NULL;
  size += sizeof(player_audio_mixer_channel_t)*src->channels_count;
  memcpy(dest->channels,src->channels,sizeof(player_audio_mixer_channel_t)*src->channels_count); 
  return(size);
}
void player_audio_mixer_channel_list_t_cleanup(const player_audio_mixer_channel_list_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->channels); 
}
player_audio_mixer_channel_list_t * player_audio_mixer_channel_list_t_clone(const player_audio_mixer_channel_list_t *msg)
{      
  player_audio_mixer_channel_list_t * clone = malloc(sizeof(player_audio_mixer_channel_list_t));
  if (clone)
    player_audio_mixer_channel_list_t_copy(clone,msg);
  return clone;
}
void player_audio_mixer_channel_list_t_free(player_audio_mixer_channel_list_t *msg)
{      
  player_audio_mixer_channel_list_t_cleanup(msg);
  free(msg);
}
unsigned int player_audio_mixer_channel_list_t_sizeof(player_audio_mixer_channel_list_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_audio_mixer_channel_t)*msg->channels_count; 
  return(size);
}

int xdr_player_audio_mixer_channel_detail_t (XDR* xdrs, player_audio_mixer_channel_detail_t * msg)
{   if(xdr_u_int(xdrs,&msg->name_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->name = malloc(msg->name_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* name_p = msg->name;
    if(xdr_bytes(xdrs, (char**)&name_p, &msg->name_count, msg->name_count) != 1)
      return(0);
  }
  if(xdr_u_char(xdrs,&msg->caps) != 1)
    return(0);
  return(1);
}
int 
player_audio_mixer_channel_detail_pack(void* buf, size_t buflen, player_audio_mixer_channel_detail_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_audio_mixer_channel_detail_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_audio_mixer_channel_detail_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_audio_mixer_channel_detail_t_copy(player_audio_mixer_channel_detail_t *dest, const player_audio_mixer_channel_detail_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->name_count,&src->name_count,sizeof(uint32_t)*1); 
  if(src->name != NULL && src->name_count > 0)
  {
    if((dest->name = malloc(src->name_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->name = NULL;
  size += sizeof(char)*src->name_count;
  memcpy(dest->name,src->name,sizeof(char)*src->name_count); 
  size += sizeof(uint8_t)*1;
  memcpy(&dest->caps,&src->caps,sizeof(uint8_t)*1); 
  return(size);
}
void player_audio_mixer_channel_detail_t_cleanup(const player_audio_mixer_channel_detail_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->name); 
}
player_audio_mixer_channel_detail_t * player_audio_mixer_channel_detail_t_clone(const player_audio_mixer_channel_detail_t *msg)
{      
  player_audio_mixer_channel_detail_t * clone = malloc(sizeof(player_audio_mixer_channel_detail_t));
  if (clone)
    player_audio_mixer_channel_detail_t_copy(clone,msg);
  return clone;
}
void player_audio_mixer_channel_detail_t_free(player_audio_mixer_channel_detail_t *msg)
{      
  player_audio_mixer_channel_detail_t_cleanup(msg);
  free(msg);
}
unsigned int player_audio_mixer_channel_detail_t_sizeof(player_audio_mixer_channel_detail_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->name_count; 
  size += sizeof(uint8_t)*1; 
  return(size);
}

int xdr_player_audio_mixer_channel_list_detail_t (XDR* xdrs, player_audio_mixer_channel_list_detail_t * msg)
{   if(xdr_u_int(xdrs,&msg->details_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->details = malloc(msg->details_count*sizeof(player_audio_mixer_channel_detail_t))) == NULL)
      return(0);
  }
  {
    player_audio_mixer_channel_detail_t* details_p = msg->details;
    if(xdr_array(xdrs, (char**)&details_p, &msg->details_count, msg->details_count, sizeof(player_audio_mixer_channel_detail_t), (xdrproc_t)xdr_player_audio_mixer_channel_detail_t) != 1)
      return(0);
  }
  if(xdr_int(xdrs,&msg->default_output) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->default_input) != 1)
    return(0);
  return(1);
}
int 
player_audio_mixer_channel_list_detail_pack(void* buf, size_t buflen, player_audio_mixer_channel_list_detail_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_audio_mixer_channel_list_detail_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_audio_mixer_channel_list_detail_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_audio_mixer_channel_list_detail_t_copy(player_audio_mixer_channel_list_detail_t *dest, const player_audio_mixer_channel_list_detail_t *src)
{      
  unsigned ii;
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->details_count,&src->details_count,sizeof(uint32_t)*1); 
  if(src->details != NULL && src->details_count > 0)
  {
    if((dest->details = malloc(src->details_count*sizeof(player_audio_mixer_channel_detail_t))) == NULL)
      return(0);
  }
  else
    dest->details = NULL;
  for(ii = 0; ii < src->details_count; ii++)
  {size += player_audio_mixer_channel_detail_t_copy(&dest->details[ii], &src->details[ii]);}
  size += sizeof(int32_t)*1;
  memcpy(&dest->default_output,&src->default_output,sizeof(int32_t)*1); 
  size += sizeof(int32_t)*1;
  memcpy(&dest->default_input,&src->default_input,sizeof(int32_t)*1); 
  return(size);
}
void player_audio_mixer_channel_list_detail_t_cleanup(const player_audio_mixer_channel_list_detail_t *msg)
{      
  unsigned ii;
  if(msg == NULL)
    return;
  for(ii = 0; ii < msg->details_count; ii++)
  {
    player_audio_mixer_channel_detail_t_cleanup(&msg->details[ii]);
  }
  free(msg->details); 
}
player_audio_mixer_channel_list_detail_t * player_audio_mixer_channel_list_detail_t_clone(const player_audio_mixer_channel_list_detail_t *msg)
{      
  player_audio_mixer_channel_list_detail_t * clone = malloc(sizeof(player_audio_mixer_channel_list_detail_t));
  if (clone)
    player_audio_mixer_channel_list_detail_t_copy(clone,msg);
  return clone;
}
void player_audio_mixer_channel_list_detail_t_free(player_audio_mixer_channel_list_detail_t *msg)
{      
  player_audio_mixer_channel_list_detail_t_cleanup(msg);
  free(msg);
}
unsigned int player_audio_mixer_channel_list_detail_t_sizeof(player_audio_mixer_channel_list_detail_t *msg)
{
  unsigned ii;
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  for(ii = 0; ii < msg->details_count; ii++)
  {size += player_audio_mixer_channel_detail_t_sizeof(&msg->details[ii]);}
  size += sizeof(int32_t)*1; 
  size += sizeof(int32_t)*1; 
  return(size);
}

int xdr_player_audio_sample_t (XDR* xdrs, player_audio_sample_t * msg)
{   if(xdr_player_audio_wav_t(xdrs,&msg->sample) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->index) != 1)
    return(0);
  return(1);
}
int 
player_audio_sample_pack(void* buf, size_t buflen, player_audio_sample_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_audio_sample_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_audio_sample_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_audio_sample_t_copy(player_audio_sample_t *dest, const player_audio_sample_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  {size += player_audio_wav_t_copy(&dest->sample, &src->sample);}
  size += sizeof(int32_t)*1;
  memcpy(&dest->index,&src->index,sizeof(int32_t)*1); 
  return(size);
}
void player_audio_sample_t_cleanup(const player_audio_sample_t *msg)
{      
  
  if(msg == NULL)
    return;
  player_audio_wav_t_cleanup(&msg->sample); 
}
player_audio_sample_t * player_audio_sample_t_clone(const player_audio_sample_t *msg)
{      
  player_audio_sample_t * clone = malloc(sizeof(player_audio_sample_t));
  if (clone)
    player_audio_sample_t_copy(clone,msg);
  return clone;
}
void player_audio_sample_t_free(player_audio_sample_t *msg)
{      
  player_audio_sample_t_cleanup(msg);
  free(msg);
}
unsigned int player_audio_sample_t_sizeof(player_audio_sample_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  {size += player_audio_wav_t_sizeof(&msg->sample);}
  size += sizeof(int32_t)*1; 
  return(size);
}

int xdr_player_audio_sample_item_t (XDR* xdrs, player_audio_sample_item_t * msg)
{   if(xdr_int(xdrs,&msg->index) != 1)
    return(0);
  return(1);
}
int 
player_audio_sample_item_pack(void* buf, size_t buflen, player_audio_sample_item_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_audio_sample_item_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_audio_sample_item_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_audio_sample_item_t_copy(player_audio_sample_item_t *dest, const player_audio_sample_item_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_audio_sample_item_t));
  return sizeof(player_audio_sample_item_t);
} 
void player_audio_sample_item_t_cleanup(const player_audio_sample_item_t *msg)
{
} 
player_audio_sample_item_t * player_audio_sample_item_t_clone(const player_audio_sample_item_t *msg)
{      
  player_audio_sample_item_t * clone = malloc(sizeof(player_audio_sample_item_t));
  if (clone)
    player_audio_sample_item_t_copy(clone,msg);
  return clone;
}
void player_audio_sample_item_t_free(player_audio_sample_item_t *msg)
{      
  player_audio_sample_item_t_cleanup(msg);
  free(msg);
}
unsigned int player_audio_sample_item_t_sizeof(player_audio_sample_item_t *msg)
{
  return sizeof(player_audio_sample_item_t);
} 

int xdr_player_audio_sample_rec_req_t (XDR* xdrs, player_audio_sample_rec_req_t * msg)
{   if(xdr_int(xdrs,&msg->index) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->length) != 1)
    return(0);
  return(1);
}
int 
player_audio_sample_rec_req_pack(void* buf, size_t buflen, player_audio_sample_rec_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_audio_sample_rec_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_audio_sample_rec_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_audio_sample_rec_req_t_copy(player_audio_sample_rec_req_t *dest, const player_audio_sample_rec_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_audio_sample_rec_req_t));
  return sizeof(player_audio_sample_rec_req_t);
} 
void player_audio_sample_rec_req_t_cleanup(const player_audio_sample_rec_req_t *msg)
{
} 
player_audio_sample_rec_req_t * player_audio_sample_rec_req_t_clone(const player_audio_sample_rec_req_t *msg)
{      
  player_audio_sample_rec_req_t * clone = malloc(sizeof(player_audio_sample_rec_req_t));
  if (clone)
    player_audio_sample_rec_req_t_copy(clone,msg);
  return clone;
}
void player_audio_sample_rec_req_t_free(player_audio_sample_rec_req_t *msg)
{      
  player_audio_sample_rec_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_audio_sample_rec_req_t_sizeof(player_audio_sample_rec_req_t *msg)
{
  return sizeof(player_audio_sample_rec_req_t);
} 

int xdr_player_audio_state_t (XDR* xdrs, player_audio_state_t * msg)
{   if(xdr_u_int(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_audio_state_pack(void* buf, size_t buflen, player_audio_state_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_audio_state_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_audio_state_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_audio_state_t_copy(player_audio_state_t *dest, const player_audio_state_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_audio_state_t));
  return sizeof(player_audio_state_t);
} 
void player_audio_state_t_cleanup(const player_audio_state_t *msg)
{
} 
player_audio_state_t * player_audio_state_t_clone(const player_audio_state_t *msg)
{      
  player_audio_state_t * clone = malloc(sizeof(player_audio_state_t));
  if (clone)
    player_audio_state_t_copy(clone,msg);
  return clone;
}
void player_audio_state_t_free(player_audio_state_t *msg)
{      
  player_audio_state_t_cleanup(msg);
  free(msg);
}
unsigned int player_audio_state_t_sizeof(player_audio_state_t *msg)
{
  return sizeof(player_audio_state_t);
} 

int xdr_player_opaque_data_t (XDR* xdrs, player_opaque_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->data_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->data = malloc(msg->data_count*sizeof(uint8_t))) == NULL)
      return(0);
  }
  {
    uint8_t* data_p = msg->data;
    if(xdr_bytes(xdrs, (char**)&data_p, &msg->data_count, msg->data_count) != 1)
      return(0);
  }
  return(1);
}
int 
player_opaque_data_pack(void* buf, size_t buflen, player_opaque_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_opaque_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_opaque_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_opaque_data_t_copy(player_opaque_data_t *dest, const player_opaque_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->data_count,&src->data_count,sizeof(uint32_t)*1); 
  if(src->data != NULL && src->data_count > 0)
  {
    if((dest->data = malloc(src->data_count*sizeof(uint8_t))) == NULL)
      return(0);
  }
  else
    dest->data = NULL;
  size += sizeof(uint8_t)*src->data_count;
  memcpy(dest->data,src->data,sizeof(uint8_t)*src->data_count); 
  return(size);
}
void player_opaque_data_t_cleanup(const player_opaque_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->data); 
}
player_opaque_data_t * player_opaque_data_t_clone(const player_opaque_data_t *msg)
{      
  player_opaque_data_t * clone = malloc(sizeof(player_opaque_data_t));
  if (clone)
    player_opaque_data_t_copy(clone,msg);
  return clone;
}
void player_opaque_data_t_free(player_opaque_data_t *msg)
{      
  player_opaque_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_opaque_data_t_sizeof(player_opaque_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint8_t)*msg->data_count; 
  return(size);
}

int xdr_player_fiducial_item_t (XDR* xdrs, player_fiducial_item_t * msg)
{   if(xdr_int(xdrs,&msg->id) != 1)
    return(0);
  if(xdr_player_pose3d_t(xdrs,&msg->pose) != 1)
    return(0);
  if(xdr_player_pose3d_t(xdrs,&msg->upose) != 1)
    return(0);
  return(1);
}
int 
player_fiducial_item_pack(void* buf, size_t buflen, player_fiducial_item_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_fiducial_item_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_fiducial_item_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_fiducial_item_t_copy(player_fiducial_item_t *dest, const player_fiducial_item_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_fiducial_item_t));
  return sizeof(player_fiducial_item_t);
} 
void player_fiducial_item_t_cleanup(const player_fiducial_item_t *msg)
{
} 
player_fiducial_item_t * player_fiducial_item_t_clone(const player_fiducial_item_t *msg)
{      
  player_fiducial_item_t * clone = malloc(sizeof(player_fiducial_item_t));
  if (clone)
    player_fiducial_item_t_copy(clone,msg);
  return clone;
}
void player_fiducial_item_t_free(player_fiducial_item_t *msg)
{      
  player_fiducial_item_t_cleanup(msg);
  free(msg);
}
unsigned int player_fiducial_item_t_sizeof(player_fiducial_item_t *msg)
{
  return sizeof(player_fiducial_item_t);
} 

int xdr_player_fiducial_data_t (XDR* xdrs, player_fiducial_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->fiducials_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->fiducials = malloc(msg->fiducials_count*sizeof(player_fiducial_item_t))) == NULL)
      return(0);
  }
  {
    player_fiducial_item_t* fiducials_p = msg->fiducials;
    if(xdr_array(xdrs, (char**)&fiducials_p, &msg->fiducials_count, msg->fiducials_count, sizeof(player_fiducial_item_t), (xdrproc_t)xdr_player_fiducial_item_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_fiducial_data_pack(void* buf, size_t buflen, player_fiducial_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_fiducial_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_fiducial_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_fiducial_data_t_copy(player_fiducial_data_t *dest, const player_fiducial_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->fiducials_count,&src->fiducials_count,sizeof(uint32_t)*1); 
  if(src->fiducials != NULL && src->fiducials_count > 0)
  {
    if((dest->fiducials = malloc(src->fiducials_count*sizeof(player_fiducial_item_t))) == NULL)
      return(0);
  }
  else
    dest->fiducials = NULL;
  size += sizeof(player_fiducial_item_t)*src->fiducials_count;
  memcpy(dest->fiducials,src->fiducials,sizeof(player_fiducial_item_t)*src->fiducials_count); 
  return(size);
}
void player_fiducial_data_t_cleanup(const player_fiducial_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->fiducials); 
}
player_fiducial_data_t * player_fiducial_data_t_clone(const player_fiducial_data_t *msg)
{      
  player_fiducial_data_t * clone = malloc(sizeof(player_fiducial_data_t));
  if (clone)
    player_fiducial_data_t_copy(clone,msg);
  return clone;
}
void player_fiducial_data_t_free(player_fiducial_data_t *msg)
{      
  player_fiducial_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_fiducial_data_t_sizeof(player_fiducial_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_fiducial_item_t)*msg->fiducials_count; 
  return(size);
}

int xdr_player_fiducial_geom_t (XDR* xdrs, player_fiducial_geom_t * msg)
{   if(xdr_player_pose3d_t(xdrs,&msg->pose) != 1)
    return(0);
  if(xdr_player_bbox3d_t(xdrs,&msg->size) != 1)
    return(0);
  if(xdr_player_bbox2d_t(xdrs,&msg->fiducial_size) != 1)
    return(0);
  return(1);
}
int 
player_fiducial_geom_pack(void* buf, size_t buflen, player_fiducial_geom_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_fiducial_geom_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_fiducial_geom_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_fiducial_geom_t_copy(player_fiducial_geom_t *dest, const player_fiducial_geom_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_fiducial_geom_t));
  return sizeof(player_fiducial_geom_t);
} 
void player_fiducial_geom_t_cleanup(const player_fiducial_geom_t *msg)
{
} 
player_fiducial_geom_t * player_fiducial_geom_t_clone(const player_fiducial_geom_t *msg)
{      
  player_fiducial_geom_t * clone = malloc(sizeof(player_fiducial_geom_t));
  if (clone)
    player_fiducial_geom_t_copy(clone,msg);
  return clone;
}
void player_fiducial_geom_t_free(player_fiducial_geom_t *msg)
{      
  player_fiducial_geom_t_cleanup(msg);
  free(msg);
}
unsigned int player_fiducial_geom_t_sizeof(player_fiducial_geom_t *msg)
{
  return sizeof(player_fiducial_geom_t);
} 

int xdr_player_fiducial_fov_t (XDR* xdrs, player_fiducial_fov_t * msg)
{   if(xdr_float(xdrs,&msg->min_range) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->max_range) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->view_angle) != 1)
    return(0);
  return(1);
}
int 
player_fiducial_fov_pack(void* buf, size_t buflen, player_fiducial_fov_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_fiducial_fov_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_fiducial_fov_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_fiducial_fov_t_copy(player_fiducial_fov_t *dest, const player_fiducial_fov_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_fiducial_fov_t));
  return sizeof(player_fiducial_fov_t);
} 
void player_fiducial_fov_t_cleanup(const player_fiducial_fov_t *msg)
{
} 
player_fiducial_fov_t * player_fiducial_fov_t_clone(const player_fiducial_fov_t *msg)
{      
  player_fiducial_fov_t * clone = malloc(sizeof(player_fiducial_fov_t));
  if (clone)
    player_fiducial_fov_t_copy(clone,msg);
  return clone;
}
void player_fiducial_fov_t_free(player_fiducial_fov_t *msg)
{      
  player_fiducial_fov_t_cleanup(msg);
  free(msg);
}
unsigned int player_fiducial_fov_t_sizeof(player_fiducial_fov_t *msg)
{
  return sizeof(player_fiducial_fov_t);
} 

int xdr_player_fiducial_id_t (XDR* xdrs, player_fiducial_id_t * msg)
{   if(xdr_u_int(xdrs,&msg->id) != 1)
    return(0);
  return(1);
}
int 
player_fiducial_id_pack(void* buf, size_t buflen, player_fiducial_id_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_fiducial_id_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_fiducial_id_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_fiducial_id_t_copy(player_fiducial_id_t *dest, const player_fiducial_id_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_fiducial_id_t));
  return sizeof(player_fiducial_id_t);
} 
void player_fiducial_id_t_cleanup(const player_fiducial_id_t *msg)
{
} 
player_fiducial_id_t * player_fiducial_id_t_clone(const player_fiducial_id_t *msg)
{      
  player_fiducial_id_t * clone = malloc(sizeof(player_fiducial_id_t));
  if (clone)
    player_fiducial_id_t_copy(clone,msg);
  return clone;
}
void player_fiducial_id_t_free(player_fiducial_id_t *msg)
{      
  player_fiducial_id_t_cleanup(msg);
  free(msg);
}
unsigned int player_fiducial_id_t_sizeof(player_fiducial_id_t *msg)
{
  return sizeof(player_fiducial_id_t);
} 

int xdr_player_victim_fiducial_item_t (XDR* xdrs, player_victim_fiducial_item_t * msg)
{   if(xdr_opaque(xdrs, (char*)&msg->id, PLAYER_FIDUCIAL_MAX_ID_LENGTH) != 1)
    return(0);
  if(xdr_opaque(xdrs, (char*)&msg->status, PLAYER_FIDUCIAL_MAX_STATUS_LENGTH) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->timestamp) != 1)
    return(0);
  if(xdr_player_pose3d_t(xdrs,&msg->pose) != 1)
    return(0);
  if(xdr_player_pose3d_t(xdrs,&msg->upose) != 1)
    return(0);
  return(1);
}
int 
player_victim_fiducial_item_pack(void* buf, size_t buflen, player_victim_fiducial_item_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_victim_fiducial_item_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_victim_fiducial_item_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_victim_fiducial_item_t_copy(player_victim_fiducial_item_t *dest, const player_victim_fiducial_item_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_victim_fiducial_item_t));
  return sizeof(player_victim_fiducial_item_t);
} 
void player_victim_fiducial_item_t_cleanup(const player_victim_fiducial_item_t *msg)
{
} 
player_victim_fiducial_item_t * player_victim_fiducial_item_t_clone(const player_victim_fiducial_item_t *msg)
{      
  player_victim_fiducial_item_t * clone = malloc(sizeof(player_victim_fiducial_item_t));
  if (clone)
    player_victim_fiducial_item_t_copy(clone,msg);
  return clone;
}
void player_victim_fiducial_item_t_free(player_victim_fiducial_item_t *msg)
{      
  player_victim_fiducial_item_t_cleanup(msg);
  free(msg);
}
unsigned int player_victim_fiducial_item_t_sizeof(player_victim_fiducial_item_t *msg)
{
  return sizeof(player_victim_fiducial_item_t);
} 

int xdr_player_victim_fiducial_data_t (XDR* xdrs, player_victim_fiducial_data_t * msg)
{   if(xdr_u_short(xdrs,&msg->fiducials_count) != 1)
    return(0);
  {
    player_victim_fiducial_item_t* fiducials_p = msg->fiducials;
    if(xdr_array(xdrs, (char**)&fiducials_p, &msg->fiducials_count, PLAYER_FIDUCIAL_MAX_SAMPLES, sizeof(player_victim_fiducial_item_t), (xdrproc_t)xdr_player_victim_fiducial_item_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_victim_fiducial_data_pack(void* buf, size_t buflen, player_victim_fiducial_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_victim_fiducial_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_victim_fiducial_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_victim_fiducial_data_t_copy(player_victim_fiducial_data_t *dest, const player_victim_fiducial_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_victim_fiducial_data_t));
  return sizeof(player_victim_fiducial_data_t);
} 
void player_victim_fiducial_data_t_cleanup(const player_victim_fiducial_data_t *msg)
{
} 
player_victim_fiducial_data_t * player_victim_fiducial_data_t_clone(const player_victim_fiducial_data_t *msg)
{      
  player_victim_fiducial_data_t * clone = malloc(sizeof(player_victim_fiducial_data_t));
  if (clone)
    player_victim_fiducial_data_t_copy(clone,msg);
  return clone;
}
void player_victim_fiducial_data_t_free(player_victim_fiducial_data_t *msg)
{      
  player_victim_fiducial_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_victim_fiducial_data_t_sizeof(player_victim_fiducial_data_t *msg)
{
  return sizeof(player_victim_fiducial_data_t);
} 

int xdr_player_log_set_write_state_t (XDR* xdrs, player_log_set_write_state_t * msg)
{   if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_log_set_write_state_pack(void* buf, size_t buflen, player_log_set_write_state_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_log_set_write_state_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_log_set_write_state_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_log_set_write_state_t_copy(player_log_set_write_state_t *dest, const player_log_set_write_state_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_log_set_write_state_t));
  return sizeof(player_log_set_write_state_t);
} 
void player_log_set_write_state_t_cleanup(const player_log_set_write_state_t *msg)
{
} 
player_log_set_write_state_t * player_log_set_write_state_t_clone(const player_log_set_write_state_t *msg)
{      
  player_log_set_write_state_t * clone = malloc(sizeof(player_log_set_write_state_t));
  if (clone)
    player_log_set_write_state_t_copy(clone,msg);
  return clone;
}
void player_log_set_write_state_t_free(player_log_set_write_state_t *msg)
{      
  player_log_set_write_state_t_cleanup(msg);
  free(msg);
}
unsigned int player_log_set_write_state_t_sizeof(player_log_set_write_state_t *msg)
{
  return sizeof(player_log_set_write_state_t);
} 

int xdr_player_log_set_read_state_t (XDR* xdrs, player_log_set_read_state_t * msg)
{   if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_log_set_read_state_pack(void* buf, size_t buflen, player_log_set_read_state_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_log_set_read_state_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_log_set_read_state_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_log_set_read_state_t_copy(player_log_set_read_state_t *dest, const player_log_set_read_state_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_log_set_read_state_t));
  return sizeof(player_log_set_read_state_t);
} 
void player_log_set_read_state_t_cleanup(const player_log_set_read_state_t *msg)
{
} 
player_log_set_read_state_t * player_log_set_read_state_t_clone(const player_log_set_read_state_t *msg)
{      
  player_log_set_read_state_t * clone = malloc(sizeof(player_log_set_read_state_t));
  if (clone)
    player_log_set_read_state_t_copy(clone,msg);
  return clone;
}
void player_log_set_read_state_t_free(player_log_set_read_state_t *msg)
{      
  player_log_set_read_state_t_cleanup(msg);
  free(msg);
}
unsigned int player_log_set_read_state_t_sizeof(player_log_set_read_state_t *msg)
{
  return sizeof(player_log_set_read_state_t);
} 

int xdr_player_log_set_read_rewind_t (XDR* xdrs, player_log_set_read_rewind_t * msg)
{   return(1);
}
int 
player_log_set_read_rewind_pack(void* buf, size_t buflen, player_log_set_read_rewind_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_log_set_read_rewind_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_log_set_read_rewind_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_log_set_read_rewind_t_copy(player_log_set_read_rewind_t *dest, const player_log_set_read_rewind_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_log_set_read_rewind_t));
  return sizeof(player_log_set_read_rewind_t);
} 
void player_log_set_read_rewind_t_cleanup(const player_log_set_read_rewind_t *msg)
{
} 
player_log_set_read_rewind_t * player_log_set_read_rewind_t_clone(const player_log_set_read_rewind_t *msg)
{      
  player_log_set_read_rewind_t * clone = malloc(sizeof(player_log_set_read_rewind_t));
  if (clone)
    player_log_set_read_rewind_t_copy(clone,msg);
  return clone;
}
void player_log_set_read_rewind_t_free(player_log_set_read_rewind_t *msg)
{      
  player_log_set_read_rewind_t_cleanup(msg);
  free(msg);
}
unsigned int player_log_set_read_rewind_t_sizeof(player_log_set_read_rewind_t *msg)
{
  return sizeof(player_log_set_read_rewind_t);
} 

int xdr_player_log_get_state_t (XDR* xdrs, player_log_get_state_t * msg)
{   if(xdr_u_char(xdrs,&msg->type) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_log_get_state_pack(void* buf, size_t buflen, player_log_get_state_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_log_get_state_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_log_get_state_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_log_get_state_t_copy(player_log_get_state_t *dest, const player_log_get_state_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_log_get_state_t));
  return sizeof(player_log_get_state_t);
} 
void player_log_get_state_t_cleanup(const player_log_get_state_t *msg)
{
} 
player_log_get_state_t * player_log_get_state_t_clone(const player_log_get_state_t *msg)
{      
  player_log_get_state_t * clone = malloc(sizeof(player_log_get_state_t));
  if (clone)
    player_log_get_state_t_copy(clone,msg);
  return clone;
}
void player_log_get_state_t_free(player_log_get_state_t *msg)
{      
  player_log_get_state_t_cleanup(msg);
  free(msg);
}
unsigned int player_log_get_state_t_sizeof(player_log_get_state_t *msg)
{
  return sizeof(player_log_get_state_t);
} 

int xdr_player_log_set_filename_t (XDR* xdrs, player_log_set_filename_t * msg)
{   if(xdr_u_int(xdrs,&msg->filename_count) != 1)
    return(0);
  {
    char* filename_p = msg->filename;
    if(xdr_bytes(xdrs, (char**)&filename_p, &msg->filename_count, 256) != 1)
      return(0);
  }
  return(1);
}
int 
player_log_set_filename_pack(void* buf, size_t buflen, player_log_set_filename_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_log_set_filename_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_log_set_filename_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_log_set_filename_t_copy(player_log_set_filename_t *dest, const player_log_set_filename_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_log_set_filename_t));
  return sizeof(player_log_set_filename_t);
} 
void player_log_set_filename_t_cleanup(const player_log_set_filename_t *msg)
{
} 
player_log_set_filename_t * player_log_set_filename_t_clone(const player_log_set_filename_t *msg)
{      
  player_log_set_filename_t * clone = malloc(sizeof(player_log_set_filename_t));
  if (clone)
    player_log_set_filename_t_copy(clone,msg);
  return clone;
}
void player_log_set_filename_t_free(player_log_set_filename_t *msg)
{      
  player_log_set_filename_t_cleanup(msg);
  free(msg);
}
unsigned int player_log_set_filename_t_sizeof(player_log_set_filename_t *msg)
{
  return sizeof(player_log_set_filename_t);
} 

int xdr_player_rfid_tag_t (XDR* xdrs, player_rfid_tag_t * msg)
{   if(xdr_u_int(xdrs,&msg->type) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->guid_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->guid = malloc(msg->guid_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* guid_p = msg->guid;
    if(xdr_bytes(xdrs, (char**)&guid_p, &msg->guid_count, msg->guid_count) != 1)
      return(0);
  }
  return(1);
}
int 
player_rfid_tag_pack(void* buf, size_t buflen, player_rfid_tag_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_rfid_tag_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_rfid_tag_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_rfid_tag_t_copy(player_rfid_tag_t *dest, const player_rfid_tag_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->type,&src->type,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->guid_count,&src->guid_count,sizeof(uint32_t)*1); 
  if(src->guid != NULL && src->guid_count > 0)
  {
    if((dest->guid = malloc(src->guid_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->guid = NULL;
  size += sizeof(char)*src->guid_count;
  memcpy(dest->guid,src->guid,sizeof(char)*src->guid_count); 
  return(size);
}
void player_rfid_tag_t_cleanup(const player_rfid_tag_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->guid); 
}
player_rfid_tag_t * player_rfid_tag_t_clone(const player_rfid_tag_t *msg)
{      
  player_rfid_tag_t * clone = malloc(sizeof(player_rfid_tag_t));
  if (clone)
    player_rfid_tag_t_copy(clone,msg);
  return clone;
}
void player_rfid_tag_t_free(player_rfid_tag_t *msg)
{      
  player_rfid_tag_t_cleanup(msg);
  free(msg);
}
unsigned int player_rfid_tag_t_sizeof(player_rfid_tag_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->guid_count; 
  return(size);
}

int xdr_player_rfid_data_t (XDR* xdrs, player_rfid_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->tags_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->tags = malloc(msg->tags_count*sizeof(player_rfid_tag_t))) == NULL)
      return(0);
  }
  {
    player_rfid_tag_t* tags_p = msg->tags;
    if(xdr_array(xdrs, (char**)&tags_p, &msg->tags_count, msg->tags_count, sizeof(player_rfid_tag_t), (xdrproc_t)xdr_player_rfid_tag_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_rfid_data_pack(void* buf, size_t buflen, player_rfid_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_rfid_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_rfid_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_rfid_data_t_copy(player_rfid_data_t *dest, const player_rfid_data_t *src)
{      
  unsigned ii;
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->tags_count,&src->tags_count,sizeof(uint32_t)*1); 
  if(src->tags != NULL && src->tags_count > 0)
  {
    if((dest->tags = malloc(src->tags_count*sizeof(player_rfid_tag_t))) == NULL)
      return(0);
  }
  else
    dest->tags = NULL;
  for(ii = 0; ii < src->tags_count; ii++)
  {size += player_rfid_tag_t_copy(&dest->tags[ii], &src->tags[ii]);}
  return(size);
}
void player_rfid_data_t_cleanup(const player_rfid_data_t *msg)
{      
  unsigned ii;
  if(msg == NULL)
    return;
  for(ii = 0; ii < msg->tags_count; ii++)
  {
    player_rfid_tag_t_cleanup(&msg->tags[ii]);
  }
  free(msg->tags); 
}
player_rfid_data_t * player_rfid_data_t_clone(const player_rfid_data_t *msg)
{      
  player_rfid_data_t * clone = malloc(sizeof(player_rfid_data_t));
  if (clone)
    player_rfid_data_t_copy(clone,msg);
  return clone;
}
void player_rfid_data_t_free(player_rfid_data_t *msg)
{      
  player_rfid_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_rfid_data_t_sizeof(player_rfid_data_t *msg)
{
  unsigned ii;
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  for(ii = 0; ii < msg->tags_count; ii++)
  {size += player_rfid_tag_t_sizeof(&msg->tags[ii]);}
  return(size);
}

int xdr_player_gripper_data_t (XDR* xdrs, player_gripper_data_t * msg)
{   if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->beams) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->stored) != 1)
    return(0);
  return(1);
}
int 
player_gripper_data_pack(void* buf, size_t buflen, player_gripper_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_gripper_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_gripper_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_gripper_data_t_copy(player_gripper_data_t *dest, const player_gripper_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_gripper_data_t));
  return sizeof(player_gripper_data_t);
} 
void player_gripper_data_t_cleanup(const player_gripper_data_t *msg)
{
} 
player_gripper_data_t * player_gripper_data_t_clone(const player_gripper_data_t *msg)
{      
  player_gripper_data_t * clone = malloc(sizeof(player_gripper_data_t));
  if (clone)
    player_gripper_data_t_copy(clone,msg);
  return clone;
}
void player_gripper_data_t_free(player_gripper_data_t *msg)
{      
  player_gripper_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_gripper_data_t_sizeof(player_gripper_data_t *msg)
{
  return sizeof(player_gripper_data_t);
} 

int xdr_player_gripper_geom_t (XDR* xdrs, player_gripper_geom_t * msg)
{   if(xdr_player_pose3d_t(xdrs,&msg->pose) != 1)
    return(0);
  if(xdr_player_bbox3d_t(xdrs,&msg->outer_size) != 1)
    return(0);
  if(xdr_player_bbox3d_t(xdrs,&msg->inner_size) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->num_beams) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->capacity) != 1)
    return(0);
  return(1);
}
int 
player_gripper_geom_pack(void* buf, size_t buflen, player_gripper_geom_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_gripper_geom_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_gripper_geom_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_gripper_geom_t_copy(player_gripper_geom_t *dest, const player_gripper_geom_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_gripper_geom_t));
  return sizeof(player_gripper_geom_t);
} 
void player_gripper_geom_t_cleanup(const player_gripper_geom_t *msg)
{
} 
player_gripper_geom_t * player_gripper_geom_t_clone(const player_gripper_geom_t *msg)
{      
  player_gripper_geom_t * clone = malloc(sizeof(player_gripper_geom_t));
  if (clone)
    player_gripper_geom_t_copy(clone,msg);
  return clone;
}
void player_gripper_geom_t_free(player_gripper_geom_t *msg)
{      
  player_gripper_geom_t_cleanup(msg);
  free(msg);
}
unsigned int player_gripper_geom_t_sizeof(player_gripper_geom_t *msg)
{
  return sizeof(player_gripper_geom_t);
} 

int xdr_player_gps_data_t (XDR* xdrs, player_gps_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->time_sec) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->time_usec) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->latitude) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->longitude) != 1)
    return(0);
  if(xdr_int(xdrs,&msg->altitude) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->utm_e) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->utm_n) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->quality) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->num_sats) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->hdop) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->vdop) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->err_horz) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->err_vert) != 1)
    return(0);
  return(1);
}
int 
player_gps_data_pack(void* buf, size_t buflen, player_gps_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_gps_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_gps_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_gps_data_t_copy(player_gps_data_t *dest, const player_gps_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_gps_data_t));
  return sizeof(player_gps_data_t);
} 
void player_gps_data_t_cleanup(const player_gps_data_t *msg)
{
} 
player_gps_data_t * player_gps_data_t_clone(const player_gps_data_t *msg)
{      
  player_gps_data_t * clone = malloc(sizeof(player_gps_data_t));
  if (clone)
    player_gps_data_t_copy(clone,msg);
  return clone;
}
void player_gps_data_t_free(player_gps_data_t *msg)
{      
  player_gps_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_gps_data_t_sizeof(player_gps_data_t *msg)
{
  return sizeof(player_gps_data_t);
} 

int xdr_player_localize_hypoth_t (XDR* xdrs, player_localize_hypoth_t * msg)
{   if(xdr_player_pose2d_t(xdrs,&msg->mean) != 1)
    return(0);
  if(xdr_vector(xdrs, (char*)&msg->cov, 3, sizeof(double), (xdrproc_t)xdr_double) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->alpha) != 1)
    return(0);
  return(1);
}
int 
player_localize_hypoth_pack(void* buf, size_t buflen, player_localize_hypoth_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_localize_hypoth_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_localize_hypoth_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_localize_hypoth_t_copy(player_localize_hypoth_t *dest, const player_localize_hypoth_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_localize_hypoth_t));
  return sizeof(player_localize_hypoth_t);
} 
void player_localize_hypoth_t_cleanup(const player_localize_hypoth_t *msg)
{
} 
player_localize_hypoth_t * player_localize_hypoth_t_clone(const player_localize_hypoth_t *msg)
{      
  player_localize_hypoth_t * clone = malloc(sizeof(player_localize_hypoth_t));
  if (clone)
    player_localize_hypoth_t_copy(clone,msg);
  return clone;
}
void player_localize_hypoth_t_free(player_localize_hypoth_t *msg)
{      
  player_localize_hypoth_t_cleanup(msg);
  free(msg);
}
unsigned int player_localize_hypoth_t_sizeof(player_localize_hypoth_t *msg)
{
  return sizeof(player_localize_hypoth_t);
} 

int xdr_player_localize_data_t (XDR* xdrs, player_localize_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->pending_count) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->pending_time) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->hypoths_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->hypoths = malloc(msg->hypoths_count*sizeof(player_localize_hypoth_t))) == NULL)
      return(0);
  }
  {
    player_localize_hypoth_t* hypoths_p = msg->hypoths;
    if(xdr_array(xdrs, (char**)&hypoths_p, &msg->hypoths_count, msg->hypoths_count, sizeof(player_localize_hypoth_t), (xdrproc_t)xdr_player_localize_hypoth_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_localize_data_pack(void* buf, size_t buflen, player_localize_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_localize_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_localize_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_localize_data_t_copy(player_localize_data_t *dest, const player_localize_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->pending_count,&src->pending_count,sizeof(uint32_t)*1); 
  size += sizeof(double)*1;
  memcpy(&dest->pending_time,&src->pending_time,sizeof(double)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->hypoths_count,&src->hypoths_count,sizeof(uint32_t)*1); 
  if(src->hypoths != NULL && src->hypoths_count > 0)
  {
    if((dest->hypoths = malloc(src->hypoths_count*sizeof(player_localize_hypoth_t))) == NULL)
      return(0);
  }
  else
    dest->hypoths = NULL;
  size += sizeof(player_localize_hypoth_t)*src->hypoths_count;
  memcpy(dest->hypoths,src->hypoths,sizeof(player_localize_hypoth_t)*src->hypoths_count); 
  return(size);
}
void player_localize_data_t_cleanup(const player_localize_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->hypoths); 
}
player_localize_data_t * player_localize_data_t_clone(const player_localize_data_t *msg)
{      
  player_localize_data_t * clone = malloc(sizeof(player_localize_data_t));
  if (clone)
    player_localize_data_t_copy(clone,msg);
  return clone;
}
void player_localize_data_t_free(player_localize_data_t *msg)
{      
  player_localize_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_localize_data_t_sizeof(player_localize_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(double)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_localize_hypoth_t)*msg->hypoths_count; 
  return(size);
}

int xdr_player_localize_set_pose_t (XDR* xdrs, player_localize_set_pose_t * msg)
{   if(xdr_player_pose2d_t(xdrs,&msg->mean) != 1)
    return(0);
  if(xdr_vector(xdrs, (char*)&msg->cov, 3, sizeof(double), (xdrproc_t)xdr_double) != 1)
    return(0);
  return(1);
}
int 
player_localize_set_pose_pack(void* buf, size_t buflen, player_localize_set_pose_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_localize_set_pose_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_localize_set_pose_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_localize_set_pose_t_copy(player_localize_set_pose_t *dest, const player_localize_set_pose_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_localize_set_pose_t));
  return sizeof(player_localize_set_pose_t);
} 
void player_localize_set_pose_t_cleanup(const player_localize_set_pose_t *msg)
{
} 
player_localize_set_pose_t * player_localize_set_pose_t_clone(const player_localize_set_pose_t *msg)
{      
  player_localize_set_pose_t * clone = malloc(sizeof(player_localize_set_pose_t));
  if (clone)
    player_localize_set_pose_t_copy(clone,msg);
  return clone;
}
void player_localize_set_pose_t_free(player_localize_set_pose_t *msg)
{      
  player_localize_set_pose_t_cleanup(msg);
  free(msg);
}
unsigned int player_localize_set_pose_t_sizeof(player_localize_set_pose_t *msg)
{
  return sizeof(player_localize_set_pose_t);
} 

int xdr_player_localize_particle_t (XDR* xdrs, player_localize_particle_t * msg)
{   if(xdr_player_pose2d_t(xdrs,&msg->pose) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->alpha) != 1)
    return(0);
  return(1);
}
int 
player_localize_particle_pack(void* buf, size_t buflen, player_localize_particle_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_localize_particle_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_localize_particle_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_localize_particle_t_copy(player_localize_particle_t *dest, const player_localize_particle_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_localize_particle_t));
  return sizeof(player_localize_particle_t);
} 
void player_localize_particle_t_cleanup(const player_localize_particle_t *msg)
{
} 
player_localize_particle_t * player_localize_particle_t_clone(const player_localize_particle_t *msg)
{      
  player_localize_particle_t * clone = malloc(sizeof(player_localize_particle_t));
  if (clone)
    player_localize_particle_t_copy(clone,msg);
  return clone;
}
void player_localize_particle_t_free(player_localize_particle_t *msg)
{      
  player_localize_particle_t_cleanup(msg);
  free(msg);
}
unsigned int player_localize_particle_t_sizeof(player_localize_particle_t *msg)
{
  return sizeof(player_localize_particle_t);
} 

int xdr_player_localize_get_particles_t (XDR* xdrs, player_localize_get_particles_t * msg)
{   if(xdr_player_pose2d_t(xdrs,&msg->mean) != 1)
    return(0);
  if(xdr_double(xdrs,&msg->variance) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->particles_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->particles = malloc(msg->particles_count*sizeof(player_localize_particle_t))) == NULL)
      return(0);
  }
  {
    player_localize_particle_t* particles_p = msg->particles;
    if(xdr_array(xdrs, (char**)&particles_p, &msg->particles_count, msg->particles_count, sizeof(player_localize_particle_t), (xdrproc_t)xdr_player_localize_particle_t) != 1)
      return(0);
  }
  return(1);
}
int 
player_localize_get_particles_pack(void* buf, size_t buflen, player_localize_get_particles_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_localize_get_particles_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_localize_get_particles_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_localize_get_particles_t_copy(player_localize_get_particles_t *dest, const player_localize_get_particles_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(player_pose2d_t)*1;
  memcpy(&dest->mean,&src->mean,sizeof(player_pose2d_t)*1); 
  size += sizeof(double)*1;
  memcpy(&dest->variance,&src->variance,sizeof(double)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->particles_count,&src->particles_count,sizeof(uint32_t)*1); 
  if(src->particles != NULL && src->particles_count > 0)
  {
    if((dest->particles = malloc(src->particles_count*sizeof(player_localize_particle_t))) == NULL)
      return(0);
  }
  else
    dest->particles = NULL;
  size += sizeof(player_localize_particle_t)*src->particles_count;
  memcpy(dest->particles,src->particles,sizeof(player_localize_particle_t)*src->particles_count); 
  return(size);
}
void player_localize_get_particles_t_cleanup(const player_localize_get_particles_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->particles); 
}
player_localize_get_particles_t * player_localize_get_particles_t_clone(const player_localize_get_particles_t *msg)
{      
  player_localize_get_particles_t * clone = malloc(sizeof(player_localize_get_particles_t));
  if (clone)
    player_localize_get_particles_t_copy(clone,msg);
  return clone;
}
void player_localize_get_particles_t_free(player_localize_get_particles_t *msg)
{      
  player_localize_get_particles_t_cleanup(msg);
  free(msg);
}
unsigned int player_localize_get_particles_t_sizeof(player_localize_get_particles_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(player_pose2d_t)*1; 
  size += sizeof(double)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(player_localize_particle_t)*msg->particles_count; 
  return(size);
}

int xdr_player_position1d_data_t (XDR* xdrs, player_position1d_data_t * msg)
{   if(xdr_float(xdrs,&msg->pos) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->vel) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->stall) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->status) != 1)
    return(0);
  return(1);
}
int 
player_position1d_data_pack(void* buf, size_t buflen, player_position1d_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position1d_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position1d_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position1d_data_t_copy(player_position1d_data_t *dest, const player_position1d_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position1d_data_t));
  return sizeof(player_position1d_data_t);
} 
void player_position1d_data_t_cleanup(const player_position1d_data_t *msg)
{
} 
player_position1d_data_t * player_position1d_data_t_clone(const player_position1d_data_t *msg)
{      
  player_position1d_data_t * clone = malloc(sizeof(player_position1d_data_t));
  if (clone)
    player_position1d_data_t_copy(clone,msg);
  return clone;
}
void player_position1d_data_t_free(player_position1d_data_t *msg)
{      
  player_position1d_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_position1d_data_t_sizeof(player_position1d_data_t *msg)
{
  return sizeof(player_position1d_data_t);
} 

int xdr_player_position1d_cmd_vel_t (XDR* xdrs, player_position1d_cmd_vel_t * msg)
{   if(xdr_float(xdrs,&msg->vel) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_position1d_cmd_vel_pack(void* buf, size_t buflen, player_position1d_cmd_vel_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position1d_cmd_vel_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position1d_cmd_vel_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position1d_cmd_vel_t_copy(player_position1d_cmd_vel_t *dest, const player_position1d_cmd_vel_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position1d_cmd_vel_t));
  return sizeof(player_position1d_cmd_vel_t);
} 
void player_position1d_cmd_vel_t_cleanup(const player_position1d_cmd_vel_t *msg)
{
} 
player_position1d_cmd_vel_t * player_position1d_cmd_vel_t_clone(const player_position1d_cmd_vel_t *msg)
{      
  player_position1d_cmd_vel_t * clone = malloc(sizeof(player_position1d_cmd_vel_t));
  if (clone)
    player_position1d_cmd_vel_t_copy(clone,msg);
  return clone;
}
void player_position1d_cmd_vel_t_free(player_position1d_cmd_vel_t *msg)
{      
  player_position1d_cmd_vel_t_cleanup(msg);
  free(msg);
}
unsigned int player_position1d_cmd_vel_t_sizeof(player_position1d_cmd_vel_t *msg)
{
  return sizeof(player_position1d_cmd_vel_t);
} 

int xdr_player_position1d_cmd_pos_t (XDR* xdrs, player_position1d_cmd_pos_t * msg)
{   if(xdr_float(xdrs,&msg->pos) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->vel) != 1)
    return(0);
  if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_position1d_cmd_pos_pack(void* buf, size_t buflen, player_position1d_cmd_pos_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position1d_cmd_pos_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position1d_cmd_pos_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position1d_cmd_pos_t_copy(player_position1d_cmd_pos_t *dest, const player_position1d_cmd_pos_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position1d_cmd_pos_t));
  return sizeof(player_position1d_cmd_pos_t);
} 
void player_position1d_cmd_pos_t_cleanup(const player_position1d_cmd_pos_t *msg)
{
} 
player_position1d_cmd_pos_t * player_position1d_cmd_pos_t_clone(const player_position1d_cmd_pos_t *msg)
{      
  player_position1d_cmd_pos_t * clone = malloc(sizeof(player_position1d_cmd_pos_t));
  if (clone)
    player_position1d_cmd_pos_t_copy(clone,msg);
  return clone;
}
void player_position1d_cmd_pos_t_free(player_position1d_cmd_pos_t *msg)
{      
  player_position1d_cmd_pos_t_cleanup(msg);
  free(msg);
}
unsigned int player_position1d_cmd_pos_t_sizeof(player_position1d_cmd_pos_t *msg)
{
  return sizeof(player_position1d_cmd_pos_t);
} 

int xdr_player_position1d_geom_t (XDR* xdrs, player_position1d_geom_t * msg)
{   if(xdr_player_pose3d_t(xdrs,&msg->pose) != 1)
    return(0);
  if(xdr_player_bbox3d_t(xdrs,&msg->size) != 1)
    return(0);
  return(1);
}
int 
player_position1d_geom_pack(void* buf, size_t buflen, player_position1d_geom_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position1d_geom_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position1d_geom_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position1d_geom_t_copy(player_position1d_geom_t *dest, const player_position1d_geom_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position1d_geom_t));
  return sizeof(player_position1d_geom_t);
} 
void player_position1d_geom_t_cleanup(const player_position1d_geom_t *msg)
{
} 
player_position1d_geom_t * player_position1d_geom_t_clone(const player_position1d_geom_t *msg)
{      
  player_position1d_geom_t * clone = malloc(sizeof(player_position1d_geom_t));
  if (clone)
    player_position1d_geom_t_copy(clone,msg);
  return clone;
}
void player_position1d_geom_t_free(player_position1d_geom_t *msg)
{      
  player_position1d_geom_t_cleanup(msg);
  free(msg);
}
unsigned int player_position1d_geom_t_sizeof(player_position1d_geom_t *msg)
{
  return sizeof(player_position1d_geom_t);
} 

int xdr_player_position1d_power_config_t (XDR* xdrs, player_position1d_power_config_t * msg)
{   if(xdr_u_char(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_position1d_power_config_pack(void* buf, size_t buflen, player_position1d_power_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position1d_power_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position1d_power_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position1d_power_config_t_copy(player_position1d_power_config_t *dest, const player_position1d_power_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position1d_power_config_t));
  return sizeof(player_position1d_power_config_t);
} 
void player_position1d_power_config_t_cleanup(const player_position1d_power_config_t *msg)
{
} 
player_position1d_power_config_t * player_position1d_power_config_t_clone(const player_position1d_power_config_t *msg)
{      
  player_position1d_power_config_t * clone = malloc(sizeof(player_position1d_power_config_t));
  if (clone)
    player_position1d_power_config_t_copy(clone,msg);
  return clone;
}
void player_position1d_power_config_t_free(player_position1d_power_config_t *msg)
{      
  player_position1d_power_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_position1d_power_config_t_sizeof(player_position1d_power_config_t *msg)
{
  return sizeof(player_position1d_power_config_t);
} 

int xdr_player_position1d_velocity_mode_config_t (XDR* xdrs, player_position1d_velocity_mode_config_t * msg)
{   if(xdr_u_int(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_position1d_velocity_mode_config_pack(void* buf, size_t buflen, player_position1d_velocity_mode_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position1d_velocity_mode_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position1d_velocity_mode_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position1d_velocity_mode_config_t_copy(player_position1d_velocity_mode_config_t *dest, const player_position1d_velocity_mode_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position1d_velocity_mode_config_t));
  return sizeof(player_position1d_velocity_mode_config_t);
} 
void player_position1d_velocity_mode_config_t_cleanup(const player_position1d_velocity_mode_config_t *msg)
{
} 
player_position1d_velocity_mode_config_t * player_position1d_velocity_mode_config_t_clone(const player_position1d_velocity_mode_config_t *msg)
{      
  player_position1d_velocity_mode_config_t * clone = malloc(sizeof(player_position1d_velocity_mode_config_t));
  if (clone)
    player_position1d_velocity_mode_config_t_copy(clone,msg);
  return clone;
}
void player_position1d_velocity_mode_config_t_free(player_position1d_velocity_mode_config_t *msg)
{      
  player_position1d_velocity_mode_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_position1d_velocity_mode_config_t_sizeof(player_position1d_velocity_mode_config_t *msg)
{
  return sizeof(player_position1d_velocity_mode_config_t);
} 

int xdr_player_position1d_reset_odom_config_t (XDR* xdrs, player_position1d_reset_odom_config_t * msg)
{   if(xdr_u_int(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_position1d_reset_odom_config_pack(void* buf, size_t buflen, player_position1d_reset_odom_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position1d_reset_odom_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position1d_reset_odom_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position1d_reset_odom_config_t_copy(player_position1d_reset_odom_config_t *dest, const player_position1d_reset_odom_config_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position1d_reset_odom_config_t));
  return sizeof(player_position1d_reset_odom_config_t);
} 
void player_position1d_reset_odom_config_t_cleanup(const player_position1d_reset_odom_config_t *msg)
{
} 
player_position1d_reset_odom_config_t * player_position1d_reset_odom_config_t_clone(const player_position1d_reset_odom_config_t *msg)
{      
  player_position1d_reset_odom_config_t * clone = malloc(sizeof(player_position1d_reset_odom_config_t));
  if (clone)
    player_position1d_reset_odom_config_t_copy(clone,msg);
  return clone;
}
void player_position1d_reset_odom_config_t_free(player_position1d_reset_odom_config_t *msg)
{      
  player_position1d_reset_odom_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_position1d_reset_odom_config_t_sizeof(player_position1d_reset_odom_config_t *msg)
{
  return sizeof(player_position1d_reset_odom_config_t);
} 

int xdr_player_position1d_position_mode_req_t (XDR* xdrs, player_position1d_position_mode_req_t * msg)
{   if(xdr_u_int(xdrs,&msg->state) != 1)
    return(0);
  return(1);
}
int 
player_position1d_position_mode_req_pack(void* buf, size_t buflen, player_position1d_position_mode_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position1d_position_mode_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position1d_position_mode_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position1d_position_mode_req_t_copy(player_position1d_position_mode_req_t *dest, const player_position1d_position_mode_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position1d_position_mode_req_t));
  return sizeof(player_position1d_position_mode_req_t);
} 
void player_position1d_position_mode_req_t_cleanup(const player_position1d_position_mode_req_t *msg)
{
} 
player_position1d_position_mode_req_t * player_position1d_position_mode_req_t_clone(const player_position1d_position_mode_req_t *msg)
{      
  player_position1d_position_mode_req_t * clone = malloc(sizeof(player_position1d_position_mode_req_t));
  if (clone)
    player_position1d_position_mode_req_t_copy(clone,msg);
  return clone;
}
void player_position1d_position_mode_req_t_free(player_position1d_position_mode_req_t *msg)
{      
  player_position1d_position_mode_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position1d_position_mode_req_t_sizeof(player_position1d_position_mode_req_t *msg)
{
  return sizeof(player_position1d_position_mode_req_t);
} 

int xdr_player_position1d_set_odom_req_t (XDR* xdrs, player_position1d_set_odom_req_t * msg)
{   if(xdr_float(xdrs,&msg->pos) != 1)
    return(0);
  return(1);
}
int 
player_position1d_set_odom_req_pack(void* buf, size_t buflen, player_position1d_set_odom_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position1d_set_odom_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position1d_set_odom_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position1d_set_odom_req_t_copy(player_position1d_set_odom_req_t *dest, const player_position1d_set_odom_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position1d_set_odom_req_t));
  return sizeof(player_position1d_set_odom_req_t);
} 
void player_position1d_set_odom_req_t_cleanup(const player_position1d_set_odom_req_t *msg)
{
} 
player_position1d_set_odom_req_t * player_position1d_set_odom_req_t_clone(const player_position1d_set_odom_req_t *msg)
{      
  player_position1d_set_odom_req_t * clone = malloc(sizeof(player_position1d_set_odom_req_t));
  if (clone)
    player_position1d_set_odom_req_t_copy(clone,msg);
  return clone;
}
void player_position1d_set_odom_req_t_free(player_position1d_set_odom_req_t *msg)
{      
  player_position1d_set_odom_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position1d_set_odom_req_t_sizeof(player_position1d_set_odom_req_t *msg)
{
  return sizeof(player_position1d_set_odom_req_t);
} 

int xdr_player_position1d_speed_pid_req_t (XDR* xdrs, player_position1d_speed_pid_req_t * msg)
{   if(xdr_float(xdrs,&msg->kp) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->ki) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->kd) != 1)
    return(0);
  return(1);
}
int 
player_position1d_speed_pid_req_pack(void* buf, size_t buflen, player_position1d_speed_pid_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position1d_speed_pid_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position1d_speed_pid_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position1d_speed_pid_req_t_copy(player_position1d_speed_pid_req_t *dest, const player_position1d_speed_pid_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position1d_speed_pid_req_t));
  return sizeof(player_position1d_speed_pid_req_t);
} 
void player_position1d_speed_pid_req_t_cleanup(const player_position1d_speed_pid_req_t *msg)
{
} 
player_position1d_speed_pid_req_t * player_position1d_speed_pid_req_t_clone(const player_position1d_speed_pid_req_t *msg)
{      
  player_position1d_speed_pid_req_t * clone = malloc(sizeof(player_position1d_speed_pid_req_t));
  if (clone)
    player_position1d_speed_pid_req_t_copy(clone,msg);
  return clone;
}
void player_position1d_speed_pid_req_t_free(player_position1d_speed_pid_req_t *msg)
{      
  player_position1d_speed_pid_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position1d_speed_pid_req_t_sizeof(player_position1d_speed_pid_req_t *msg)
{
  return sizeof(player_position1d_speed_pid_req_t);
} 

int xdr_player_position1d_position_pid_req_t (XDR* xdrs, player_position1d_position_pid_req_t * msg)
{   if(xdr_float(xdrs,&msg->kp) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->ki) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->kd) != 1)
    return(0);
  return(1);
}
int 
player_position1d_position_pid_req_pack(void* buf, size_t buflen, player_position1d_position_pid_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position1d_position_pid_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position1d_position_pid_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position1d_position_pid_req_t_copy(player_position1d_position_pid_req_t *dest, const player_position1d_position_pid_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position1d_position_pid_req_t));
  return sizeof(player_position1d_position_pid_req_t);
} 
void player_position1d_position_pid_req_t_cleanup(const player_position1d_position_pid_req_t *msg)
{
} 
player_position1d_position_pid_req_t * player_position1d_position_pid_req_t_clone(const player_position1d_position_pid_req_t *msg)
{      
  player_position1d_position_pid_req_t * clone = malloc(sizeof(player_position1d_position_pid_req_t));
  if (clone)
    player_position1d_position_pid_req_t_copy(clone,msg);
  return clone;
}
void player_position1d_position_pid_req_t_free(player_position1d_position_pid_req_t *msg)
{      
  player_position1d_position_pid_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position1d_position_pid_req_t_sizeof(player_position1d_position_pid_req_t *msg)
{
  return sizeof(player_position1d_position_pid_req_t);
} 

int xdr_player_position1d_speed_prof_req_t (XDR* xdrs, player_position1d_speed_prof_req_t * msg)
{   if(xdr_float(xdrs,&msg->speed) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->acc) != 1)
    return(0);
  return(1);
}
int 
player_position1d_speed_prof_req_pack(void* buf, size_t buflen, player_position1d_speed_prof_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_position1d_speed_prof_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_position1d_speed_prof_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_position1d_speed_prof_req_t_copy(player_position1d_speed_prof_req_t *dest, const player_position1d_speed_prof_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_position1d_speed_prof_req_t));
  return sizeof(player_position1d_speed_prof_req_t);
} 
void player_position1d_speed_prof_req_t_cleanup(const player_position1d_speed_prof_req_t *msg)
{
} 
player_position1d_speed_prof_req_t * player_position1d_speed_prof_req_t_clone(const player_position1d_speed_prof_req_t *msg)
{      
  player_position1d_speed_prof_req_t * clone = malloc(sizeof(player_position1d_speed_prof_req_t));
  if (clone)
    player_position1d_speed_prof_req_t_copy(clone,msg);
  return clone;
}
void player_position1d_speed_prof_req_t_free(player_position1d_speed_prof_req_t *msg)
{      
  player_position1d_speed_prof_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_position1d_speed_prof_req_t_sizeof(player_position1d_speed_prof_req_t *msg)
{
  return sizeof(player_position1d_speed_prof_req_t);
} 

int xdr_player_mcom_data_t (XDR* xdrs, player_mcom_data_t * msg)
{   if(xdr_char(xdrs,&msg->full) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->data_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->data = malloc(msg->data_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* data_p = msg->data;
    if(xdr_bytes(xdrs, (char**)&data_p, &msg->data_count, msg->data_count) != 1)
      return(0);
  }
  return(1);
}
int 
player_mcom_data_pack(void* buf, size_t buflen, player_mcom_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_mcom_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_mcom_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_mcom_data_t_copy(player_mcom_data_t *dest, const player_mcom_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(char)*1;
  memcpy(&dest->full,&src->full,sizeof(char)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->data_count,&src->data_count,sizeof(uint32_t)*1); 
  if(src->data != NULL && src->data_count > 0)
  {
    if((dest->data = malloc(src->data_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->data = NULL;
  size += sizeof(char)*src->data_count;
  memcpy(dest->data,src->data,sizeof(char)*src->data_count); 
  return(size);
}
void player_mcom_data_t_cleanup(const player_mcom_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->data); 
}
player_mcom_data_t * player_mcom_data_t_clone(const player_mcom_data_t *msg)
{      
  player_mcom_data_t * clone = malloc(sizeof(player_mcom_data_t));
  if (clone)
    player_mcom_data_t_copy(clone,msg);
  return clone;
}
void player_mcom_data_t_free(player_mcom_data_t *msg)
{      
  player_mcom_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_mcom_data_t_sizeof(player_mcom_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(char)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->data_count; 
  return(size);
}

int xdr_player_mcom_config_t (XDR* xdrs, player_mcom_config_t * msg)
{   if(xdr_u_int(xdrs,&msg->command) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->type) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->channel_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->channel = malloc(msg->channel_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* channel_p = msg->channel;
    if(xdr_bytes(xdrs, (char**)&channel_p, &msg->channel_count, msg->channel_count) != 1)
      return(0);
  }
  if(xdr_player_mcom_data_t(xdrs,&msg->data) != 1)
    return(0);
  return(1);
}
int 
player_mcom_config_pack(void* buf, size_t buflen, player_mcom_config_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_mcom_config_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_mcom_config_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_mcom_config_t_copy(player_mcom_config_t *dest, const player_mcom_config_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->command,&src->command,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->type,&src->type,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->channel_count,&src->channel_count,sizeof(uint32_t)*1); 
  if(src->channel != NULL && src->channel_count > 0)
  {
    if((dest->channel = malloc(src->channel_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->channel = NULL;
  size += sizeof(char)*src->channel_count;
  memcpy(dest->channel,src->channel,sizeof(char)*src->channel_count); 
  {size += player_mcom_data_t_copy(&dest->data, &src->data);}
  return(size);
}
void player_mcom_config_t_cleanup(const player_mcom_config_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->channel); 
  player_mcom_data_t_cleanup(&msg->data); 
}
player_mcom_config_t * player_mcom_config_t_clone(const player_mcom_config_t *msg)
{      
  player_mcom_config_t * clone = malloc(sizeof(player_mcom_config_t));
  if (clone)
    player_mcom_config_t_copy(clone,msg);
  return clone;
}
void player_mcom_config_t_free(player_mcom_config_t *msg)
{      
  player_mcom_config_t_cleanup(msg);
  free(msg);
}
unsigned int player_mcom_config_t_sizeof(player_mcom_config_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->channel_count; 
  {size += player_mcom_data_t_sizeof(&msg->data);}
  return(size);
}

int xdr_player_mcom_return_t (XDR* xdrs, player_mcom_return_t * msg)
{   if(xdr_u_int(xdrs,&msg->type) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->channel_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->channel = malloc(msg->channel_count*sizeof(char))) == NULL)
      return(0);
  }
  {
    char* channel_p = msg->channel;
    if(xdr_bytes(xdrs, (char**)&channel_p, &msg->channel_count, msg->channel_count) != 1)
      return(0);
  }
  if(xdr_player_mcom_data_t(xdrs,&msg->data) != 1)
    return(0);
  return(1);
}
int 
player_mcom_return_pack(void* buf, size_t buflen, player_mcom_return_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_mcom_return_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_mcom_return_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_mcom_return_t_copy(player_mcom_return_t *dest, const player_mcom_return_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->type,&src->type,sizeof(uint32_t)*1); 
  size += sizeof(uint32_t)*1;
  memcpy(&dest->channel_count,&src->channel_count,sizeof(uint32_t)*1); 
  if(src->channel != NULL && src->channel_count > 0)
  {
    if((dest->channel = malloc(src->channel_count*sizeof(char))) == NULL)
      return(0);
  }
  else
    dest->channel = NULL;
  size += sizeof(char)*src->channel_count;
  memcpy(dest->channel,src->channel,sizeof(char)*src->channel_count); 
  {size += player_mcom_data_t_copy(&dest->data, &src->data);}
  return(size);
}
void player_mcom_return_t_cleanup(const player_mcom_return_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->channel); 
  player_mcom_data_t_cleanup(&msg->data); 
}
player_mcom_return_t * player_mcom_return_t_clone(const player_mcom_return_t *msg)
{      
  player_mcom_return_t * clone = malloc(sizeof(player_mcom_return_t));
  if (clone)
    player_mcom_return_t_copy(clone,msg);
  return clone;
}
void player_mcom_return_t_free(player_mcom_return_t *msg)
{      
  player_mcom_return_t_cleanup(msg);
  free(msg);
}
unsigned int player_mcom_return_t_sizeof(player_mcom_return_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*1; 
  size += sizeof(char)*msg->channel_count; 
  {size += player_mcom_data_t_sizeof(&msg->data);}
  return(size);
}

int xdr_player_ptz_data_t (XDR* xdrs, player_ptz_data_t * msg)
{   if(xdr_float(xdrs,&msg->pan) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->tilt) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->zoom) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->panspeed) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->tiltspeed) != 1)
    return(0);
  if(xdr_u_int(xdrs,&msg->status) != 1)
    return(0);
  return(1);
}
int 
player_ptz_data_pack(void* buf, size_t buflen, player_ptz_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ptz_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ptz_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ptz_data_t_copy(player_ptz_data_t *dest, const player_ptz_data_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_ptz_data_t));
  return sizeof(player_ptz_data_t);
} 
void player_ptz_data_t_cleanup(const player_ptz_data_t *msg)
{
} 
player_ptz_data_t * player_ptz_data_t_clone(const player_ptz_data_t *msg)
{      
  player_ptz_data_t * clone = malloc(sizeof(player_ptz_data_t));
  if (clone)
    player_ptz_data_t_copy(clone,msg);
  return clone;
}
void player_ptz_data_t_free(player_ptz_data_t *msg)
{      
  player_ptz_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_ptz_data_t_sizeof(player_ptz_data_t *msg)
{
  return sizeof(player_ptz_data_t);
} 

int xdr_player_ptz_cmd_t (XDR* xdrs, player_ptz_cmd_t * msg)
{   if(xdr_float(xdrs,&msg->pan) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->tilt) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->zoom) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->panspeed) != 1)
    return(0);
  if(xdr_float(xdrs,&msg->tiltspeed) != 1)
    return(0);
  return(1);
}
int 
player_ptz_cmd_pack(void* buf, size_t buflen, player_ptz_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ptz_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ptz_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ptz_cmd_t_copy(player_ptz_cmd_t *dest, const player_ptz_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_ptz_cmd_t));
  return sizeof(player_ptz_cmd_t);
} 
void player_ptz_cmd_t_cleanup(const player_ptz_cmd_t *msg)
{
} 
player_ptz_cmd_t * player_ptz_cmd_t_clone(const player_ptz_cmd_t *msg)
{      
  player_ptz_cmd_t * clone = malloc(sizeof(player_ptz_cmd_t));
  if (clone)
    player_ptz_cmd_t_copy(clone,msg);
  return clone;
}
void player_ptz_cmd_t_free(player_ptz_cmd_t *msg)
{      
  player_ptz_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_ptz_cmd_t_sizeof(player_ptz_cmd_t *msg)
{
  return sizeof(player_ptz_cmd_t);
} 

int xdr_player_ptz_req_status_t (XDR* xdrs, player_ptz_req_status_t * msg)
{   if(xdr_u_int(xdrs,&msg->status) != 1)
    return(0);
  return(1);
}
int 
player_ptz_req_status_pack(void* buf, size_t buflen, player_ptz_req_status_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ptz_req_status_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ptz_req_status_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ptz_req_status_t_copy(player_ptz_req_status_t *dest, const player_ptz_req_status_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_ptz_req_status_t));
  return sizeof(player_ptz_req_status_t);
} 
void player_ptz_req_status_t_cleanup(const player_ptz_req_status_t *msg)
{
} 
player_ptz_req_status_t * player_ptz_req_status_t_clone(const player_ptz_req_status_t *msg)
{      
  player_ptz_req_status_t * clone = malloc(sizeof(player_ptz_req_status_t));
  if (clone)
    player_ptz_req_status_t_copy(clone,msg);
  return clone;
}
void player_ptz_req_status_t_free(player_ptz_req_status_t *msg)
{      
  player_ptz_req_status_t_cleanup(msg);
  free(msg);
}
unsigned int player_ptz_req_status_t_sizeof(player_ptz_req_status_t *msg)
{
  return sizeof(player_ptz_req_status_t);
} 

int xdr_player_ptz_geom_t (XDR* xdrs, player_ptz_geom_t * msg)
{   if(xdr_player_pose3d_t(xdrs,&msg->pos) != 1)
    return(0);
  if(xdr_player_bbox3d_t(xdrs,&msg->size) != 1)
    return(0);
  return(1);
}
int 
player_ptz_geom_pack(void* buf, size_t buflen, player_ptz_geom_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ptz_geom_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ptz_geom_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ptz_geom_t_copy(player_ptz_geom_t *dest, const player_ptz_geom_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_ptz_geom_t));
  return sizeof(player_ptz_geom_t);
} 
void player_ptz_geom_t_cleanup(const player_ptz_geom_t *msg)
{
} 
player_ptz_geom_t * player_ptz_geom_t_clone(const player_ptz_geom_t *msg)
{      
  player_ptz_geom_t * clone = malloc(sizeof(player_ptz_geom_t));
  if (clone)
    player_ptz_geom_t_copy(clone,msg);
  return clone;
}
void player_ptz_geom_t_free(player_ptz_geom_t *msg)
{      
  player_ptz_geom_t_cleanup(msg);
  free(msg);
}
unsigned int player_ptz_geom_t_sizeof(player_ptz_geom_t *msg)
{
  return sizeof(player_ptz_geom_t);
} 

int xdr_player_ptz_req_generic_t (XDR* xdrs, player_ptz_req_generic_t * msg)
{   if(xdr_u_int(xdrs,&msg->config_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->config = malloc(msg->config_count*sizeof(uint32_t))) == NULL)
      return(0);
  }
  {
    uint32_t* config_p = msg->config;
    if(xdr_array(xdrs, (char**)&config_p, &msg->config_count, msg->config_count, sizeof(uint32_t), (xdrproc_t)xdr_u_int) != 1)
      return(0);
  }
  return(1);
}
int 
player_ptz_req_generic_pack(void* buf, size_t buflen, player_ptz_req_generic_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ptz_req_generic_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ptz_req_generic_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ptz_req_generic_t_copy(player_ptz_req_generic_t *dest, const player_ptz_req_generic_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->config_count,&src->config_count,sizeof(uint32_t)*1); 
  if(src->config != NULL && src->config_count > 0)
  {
    if((dest->config = malloc(src->config_count*sizeof(uint32_t))) == NULL)
      return(0);
  }
  else
    dest->config = NULL;
  size += sizeof(uint32_t)*src->config_count;
  memcpy(dest->config,src->config,sizeof(uint32_t)*src->config_count); 
  return(size);
}
void player_ptz_req_generic_t_cleanup(const player_ptz_req_generic_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->config); 
}
player_ptz_req_generic_t * player_ptz_req_generic_t_clone(const player_ptz_req_generic_t *msg)
{      
  player_ptz_req_generic_t * clone = malloc(sizeof(player_ptz_req_generic_t));
  if (clone)
    player_ptz_req_generic_t_copy(clone,msg);
  return clone;
}
void player_ptz_req_generic_t_free(player_ptz_req_generic_t *msg)
{      
  player_ptz_req_generic_t_cleanup(msg);
  free(msg);
}
unsigned int player_ptz_req_generic_t_sizeof(player_ptz_req_generic_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(uint32_t)*msg->config_count; 
  return(size);
}

int xdr_player_ptz_req_control_mode_t (XDR* xdrs, player_ptz_req_control_mode_t * msg)
{   if(xdr_u_int(xdrs,&msg->mode) != 1)
    return(0);
  return(1);
}
int 
player_ptz_req_control_mode_pack(void* buf, size_t buflen, player_ptz_req_control_mode_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_ptz_req_control_mode_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_ptz_req_control_mode_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_ptz_req_control_mode_t_copy(player_ptz_req_control_mode_t *dest, const player_ptz_req_control_mode_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_ptz_req_control_mode_t));
  return sizeof(player_ptz_req_control_mode_t);
} 
void player_ptz_req_control_mode_t_cleanup(const player_ptz_req_control_mode_t *msg)
{
} 
player_ptz_req_control_mode_t * player_ptz_req_control_mode_t_clone(const player_ptz_req_control_mode_t *msg)
{      
  player_ptz_req_control_mode_t * clone = malloc(sizeof(player_ptz_req_control_mode_t));
  if (clone)
    player_ptz_req_control_mode_t_copy(clone,msg);
  return clone;
}
void player_ptz_req_control_mode_t_free(player_ptz_req_control_mode_t *msg)
{      
  player_ptz_req_control_mode_t_cleanup(msg);
  free(msg);
}
unsigned int player_ptz_req_control_mode_t_sizeof(player_ptz_req_control_mode_t *msg)
{
  return sizeof(player_ptz_req_control_mode_t);
} 
