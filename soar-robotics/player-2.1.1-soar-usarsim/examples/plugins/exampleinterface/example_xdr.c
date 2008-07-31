#include <rpc/types.h>
#include <rpc/xdr.h>

#include "example_xdr.h"
#include <string.h>
#include <stdlib.h>


int xdr_player_eginterf_data_t (XDR* xdrs, player_eginterf_data_t * msg)
{   if(xdr_u_int(xdrs,&msg->stuff_count) != 1)
    return(0);
  if(xdrs->x_op == XDR_DECODE)
  {
    if((msg->stuff = malloc(msg->stuff_count*sizeof(double))) == NULL)
      return(0);
  }
  {
    double* stuff_p = msg->stuff;
    if(xdr_array(xdrs, (char**)&stuff_p, &msg->stuff_count, msg->stuff_count, sizeof(double), (xdrproc_t)xdr_double) != 1)
      return(0);
  }
  return(1);
}
int 
player_eginterf_data_pack(void* buf, size_t buflen, player_eginterf_data_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_eginterf_data_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_eginterf_data_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_eginterf_data_t_copy(player_eginterf_data_t *dest, const player_eginterf_data_t *src)
{      
  
  unsigned int size = 0;
  if(src == NULL)
    return(0);
  size += sizeof(uint32_t)*1;
  memcpy(&dest->stuff_count,&src->stuff_count,sizeof(uint32_t)*1); 
  if(src->stuff != NULL && src->stuff_count > 0)
  {
    if((dest->stuff = malloc(src->stuff_count*sizeof(double))) == NULL)
      return(0);
  }
  else
    dest->stuff = NULL;
  size += sizeof(double)*src->stuff_count;
  memcpy(dest->stuff,src->stuff,sizeof(double)*src->stuff_count); 
  return(size);
}
void player_eginterf_data_t_cleanup(const player_eginterf_data_t *msg)
{      
  
  if(msg == NULL)
    return;
  free(msg->stuff); 
}
player_eginterf_data_t * player_eginterf_data_t_clone(const player_eginterf_data_t *msg)
{      
  player_eginterf_data_t * clone = malloc(sizeof(player_eginterf_data_t));
  if (clone)
    player_eginterf_data_t_copy(clone,msg);
  return clone;
}
void player_eginterf_data_t_free(player_eginterf_data_t *msg)
{      
  player_eginterf_data_t_cleanup(msg);
  free(msg);
}
unsigned int player_eginterf_data_t_sizeof(player_eginterf_data_t *msg)
{
  
  unsigned int size = 0;
  if(msg == NULL)
    return(0);
  size += sizeof(uint32_t)*1; 
  size += sizeof(double)*msg->stuff_count; 
  return(size);
}

int xdr_player_eginterf_req_t (XDR* xdrs, player_eginterf_req_t * msg)
{   if(xdr_int(xdrs,&msg->value) != 1)
    return(0);
  return(1);
}
int 
player_eginterf_req_pack(void* buf, size_t buflen, player_eginterf_req_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_eginterf_req_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_eginterf_req_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_eginterf_req_t_copy(player_eginterf_req_t *dest, const player_eginterf_req_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_eginterf_req_t));
  return sizeof(player_eginterf_req_t);
} 
void player_eginterf_req_t_cleanup(const player_eginterf_req_t *msg)
{
} 
player_eginterf_req_t * player_eginterf_req_t_clone(const player_eginterf_req_t *msg)
{      
  player_eginterf_req_t * clone = malloc(sizeof(player_eginterf_req_t));
  if (clone)
    player_eginterf_req_t_copy(clone,msg);
  return clone;
}
void player_eginterf_req_t_free(player_eginterf_req_t *msg)
{      
  player_eginterf_req_t_cleanup(msg);
  free(msg);
}
unsigned int player_eginterf_req_t_sizeof(player_eginterf_req_t *msg)
{
  return sizeof(player_eginterf_req_t);
} 

int xdr_player_eginterf_cmd_t (XDR* xdrs, player_eginterf_cmd_t * msg)
{   if(xdr_char(xdrs,&msg->doStuff) != 1)
    return(0);
  return(1);
}
int 
player_eginterf_cmd_pack(void* buf, size_t buflen, player_eginterf_cmd_t * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_player_eginterf_cmd_t(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(player_eginterf_cmd_t);
  xdr_destroy(&xdrs);
  return(len);
} 
unsigned int player_eginterf_cmd_t_copy(player_eginterf_cmd_t *dest, const player_eginterf_cmd_t *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(player_eginterf_cmd_t));
  return sizeof(player_eginterf_cmd_t);
} 
void player_eginterf_cmd_t_cleanup(const player_eginterf_cmd_t *msg)
{
} 
player_eginterf_cmd_t * player_eginterf_cmd_t_clone(const player_eginterf_cmd_t *msg)
{      
  player_eginterf_cmd_t * clone = malloc(sizeof(player_eginterf_cmd_t));
  if (clone)
    player_eginterf_cmd_t_copy(clone,msg);
  return clone;
}
void player_eginterf_cmd_t_free(player_eginterf_cmd_t *msg)
{      
  player_eginterf_cmd_t_cleanup(msg);
  free(msg);
}
unsigned int player_eginterf_cmd_t_sizeof(player_eginterf_cmd_t *msg)
{
  return sizeof(player_eginterf_cmd_t);
} 
