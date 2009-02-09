%inline
%{

size_t player_devaddr_t_sizeof(void)
{
  return(sizeof(player_devaddr_t));
}
player_devaddr_t* buf_to_player_devaddr_t(void* buf)
{
  return((player_devaddr_t*)(buf));
}
void* player_devaddr_t_to_buf(player_devaddr_t* msg)
{
  return((void*)(msg));
}
size_t player_msghdr_t_sizeof(void)
{
  return(sizeof(player_msghdr_t));
}
player_msghdr_t* buf_to_player_msghdr_t(void* buf)
{
  return((player_msghdr_t*)(buf));
}
void* player_msghdr_t_to_buf(player_msghdr_t* msg)
{
  return((void*)(msg));
}
size_t player_null_t_sizeof(void)
{
  return(sizeof(player_null_t));
}
player_null_t* buf_to_player_null_t(void* buf)
{
  return((player_null_t*)(buf));
}
void* player_null_t_to_buf(player_null_t* msg)
{
  return((void*)(msg));
}
size_t player_point_2d_t_sizeof(void)
{
  return(sizeof(player_point_2d_t));
}
player_point_2d_t* buf_to_player_point_2d_t(void* buf)
{
  return((player_point_2d_t*)(buf));
}
void* player_point_2d_t_to_buf(player_point_2d_t* msg)
{
  return((void*)(msg));
}
size_t player_point_3d_t_sizeof(void)
{
  return(sizeof(player_point_3d_t));
}
player_point_3d_t* buf_to_player_point_3d_t(void* buf)
{
  return((player_point_3d_t*)(buf));
}
void* player_point_3d_t_to_buf(player_point_3d_t* msg)
{
  return((void*)(msg));
}
size_t player_orientation_3d_t_sizeof(void)
{
  return(sizeof(player_orientation_3d_t));
}
player_orientation_3d_t* buf_to_player_orientation_3d_t(void* buf)
{
  return((player_orientation_3d_t*)(buf));
}
void* player_orientation_3d_t_to_buf(player_orientation_3d_t* msg)
{
  return((void*)(msg));
}
size_t player_pose2d_t_sizeof(void)
{
  return(sizeof(player_pose2d_t));
}
player_pose2d_t* buf_to_player_pose2d_t(void* buf)
{
  return((player_pose2d_t*)(buf));
}
void* player_pose2d_t_to_buf(player_pose2d_t* msg)
{
  return((void*)(msg));
}
size_t player_pose3d_t_sizeof(void)
{
  return(sizeof(player_pose3d_t));
}
player_pose3d_t* buf_to_player_pose3d_t(void* buf)
{
  return((player_pose3d_t*)(buf));
}
void* player_pose3d_t_to_buf(player_pose3d_t* msg)
{
  return((void*)(msg));
}
size_t player_bbox2d_t_sizeof(void)
{
  return(sizeof(player_bbox2d_t));
}
player_bbox2d_t* buf_to_player_bbox2d_t(void* buf)
{
  return((player_bbox2d_t*)(buf));
}
void* player_bbox2d_t_to_buf(player_bbox2d_t* msg)
{
  return((void*)(msg));
}
size_t player_bbox3d_t_sizeof(void)
{
  return(sizeof(player_bbox3d_t));
}
player_bbox3d_t* buf_to_player_bbox3d_t(void* buf)
{
  return((player_bbox3d_t*)(buf));
}
void* player_bbox3d_t_to_buf(player_bbox3d_t* msg)
{
  return((void*)(msg));
}
size_t player_blackboard_entry_t_sizeof(void)
{
  return(sizeof(player_blackboard_entry_t));
}
player_blackboard_entry_t* buf_to_player_blackboard_entry_t(void* buf)
{
  return((player_blackboard_entry_t*)(buf));
}
void* player_blackboard_entry_t_to_buf(player_blackboard_entry_t* msg)
{
  return((void*)(msg));
}
size_t player_segment_t_sizeof(void)
{
  return(sizeof(player_segment_t));
}
player_segment_t* buf_to_player_segment_t(void* buf)
{
  return((player_segment_t*)(buf));
}
void* player_segment_t_to_buf(player_segment_t* msg)
{
  return((void*)(msg));
}
size_t player_extent2d_t_sizeof(void)
{
  return(sizeof(player_extent2d_t));
}
player_extent2d_t* buf_to_player_extent2d_t(void* buf)
{
  return((player_extent2d_t*)(buf));
}
void* player_extent2d_t_to_buf(player_extent2d_t* msg)
{
  return((void*)(msg));
}
size_t player_color_t_sizeof(void)
{
  return(sizeof(player_color_t));
}
player_color_t* buf_to_player_color_t(void* buf)
{
  return((player_color_t*)(buf));
}
void* player_color_t_to_buf(player_color_t* msg)
{
  return((void*)(msg));
}
size_t player_bool_t_sizeof(void)
{
  return(sizeof(player_bool_t));
}
player_bool_t* buf_to_player_bool_t(void* buf)
{
  return((player_bool_t*)(buf));
}
void* player_bool_t_to_buf(player_bool_t* msg)
{
  return((void*)(msg));
}
size_t player_uint32_t_sizeof(void)
{
  return(sizeof(player_uint32_t));
}
player_uint32_t* buf_to_player_uint32_t(void* buf)
{
  return((player_uint32_t*)(buf));
}
void* player_uint32_t_to_buf(player_uint32_t* msg)
{
  return((void*)(msg));
}
size_t player_capabilities_req_t_sizeof(void)
{
  return(sizeof(player_capabilities_req_t));
}
player_capabilities_req_t* buf_to_player_capabilities_req_t(void* buf)
{
  return((player_capabilities_req_t*)(buf));
}
void* player_capabilities_req_t_to_buf(player_capabilities_req_t* msg)
{
  return((void*)(msg));
}
size_t player_intprop_req_t_sizeof(void)
{
  return(sizeof(player_intprop_req_t));
}
player_intprop_req_t* buf_to_player_intprop_req_t(void* buf)
{
  return((player_intprop_req_t*)(buf));
}
void* player_intprop_req_t_to_buf(player_intprop_req_t* msg)
{
  return((void*)(msg));
}
size_t player_dblprop_req_t_sizeof(void)
{
  return(sizeof(player_dblprop_req_t));
}
player_dblprop_req_t* buf_to_player_dblprop_req_t(void* buf)
{
  return((player_dblprop_req_t*)(buf));
}
void* player_dblprop_req_t_to_buf(player_dblprop_req_t* msg)
{
  return((void*)(msg));
}
size_t player_strprop_req_t_sizeof(void)
{
  return(sizeof(player_strprop_req_t));
}
player_strprop_req_t* buf_to_player_strprop_req_t(void* buf)
{
  return((player_strprop_req_t*)(buf));
}
void* player_strprop_req_t_to_buf(player_strprop_req_t* msg)
{
  return((void*)(msg));
}

%}
