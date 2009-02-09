package net.sourceforge.playerstage.Jplayercore;
public class player {

  public static Jplayer_devaddr_t buf_to_Jplayer_devaddr_t(SWIGTYPE_p_void buf) {
    player_devaddr_t data = playercore_java.buf_to_player_devaddr_t(buf);
    return(player_devaddr_t_to_Jplayer_devaddr_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_devaddr_t_to_buf(Jplayer_devaddr_t Jdata) {
    player_devaddr_t data = Jplayer_devaddr_t_to_player_devaddr_t(Jdata);
    return(playercore_java.player_devaddr_t_to_buf(data));
  }

  public static Jplayer_devaddr_t player_devaddr_t_to_Jplayer_devaddr_t(player_devaddr_t data) {
    Jplayer_devaddr_t Jdata = new Jplayer_devaddr_t();
    Jdata.host = data.getHost();
    Jdata.robot = data.getRobot();
    Jdata.interf = data.getInterf();
    Jdata.index = data.getIndex();
    return(Jdata);
  }

  public static player_devaddr_t Jplayer_devaddr_t_to_player_devaddr_t(Jplayer_devaddr_t Jdata) {
    player_devaddr_t data = new player_devaddr_t();
    data.setHost(Jdata.host);
    data.setRobot(Jdata.robot);
    data.setInterf(Jdata.interf);
    data.setIndex(Jdata.index);
    return(data);
  }

  public static Jplayer_msghdr_t buf_to_Jplayer_msghdr_t(SWIGTYPE_p_void buf) {
    player_msghdr_t data = playercore_java.buf_to_player_msghdr_t(buf);
    return(player_msghdr_t_to_Jplayer_msghdr_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_msghdr_t_to_buf(Jplayer_msghdr_t Jdata) {
    player_msghdr_t data = Jplayer_msghdr_t_to_player_msghdr_t(Jdata);
    return(playercore_java.player_msghdr_t_to_buf(data));
  }

  public static Jplayer_msghdr_t player_msghdr_t_to_Jplayer_msghdr_t(player_msghdr_t data) {
    Jplayer_msghdr_t Jdata = new Jplayer_msghdr_t();
    Jdata.addr = player_devaddr_t_to_Jplayer_devaddr_t(data.getAddr());
    Jdata.type = data.getType();
    Jdata.subtype = data.getSubtype();
    Jdata.timestamp = data.getTimestamp();
    Jdata.seq = data.getSeq();
    Jdata.size = data.getSize();
    return(Jdata);
  }

  public static player_msghdr_t Jplayer_msghdr_t_to_player_msghdr_t(Jplayer_msghdr_t Jdata) {
    player_msghdr_t data = new player_msghdr_t();
    data.setAddr(Jplayer_devaddr_t_to_player_devaddr_t(Jdata.addr));
    data.setType(Jdata.type);
    data.setSubtype(Jdata.subtype);
    data.setTimestamp(Jdata.timestamp);
    data.setSeq(Jdata.seq);
    data.setSize(Jdata.size);
    return(data);
  }

  public static Jplayer_null_t buf_to_Jplayer_null_t(SWIGTYPE_p_void buf) {
    player_null_t data = playercore_java.buf_to_player_null_t(buf);
    return(player_null_t_to_Jplayer_null_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_null_t_to_buf(Jplayer_null_t Jdata) {
    player_null_t data = Jplayer_null_t_to_player_null_t(Jdata);
    return(playercore_java.player_null_t_to_buf(data));
  }

  public static Jplayer_null_t player_null_t_to_Jplayer_null_t(player_null_t data) {
    Jplayer_null_t Jdata = new Jplayer_null_t();
    return(Jdata);
  }

  public static player_null_t Jplayer_null_t_to_player_null_t(Jplayer_null_t Jdata) {
    player_null_t data = new player_null_t();
    return(data);
  }

  public static Jplayer_point_2d_t buf_to_Jplayer_point_2d_t(SWIGTYPE_p_void buf) {
    player_point_2d_t data = playercore_java.buf_to_player_point_2d_t(buf);
    return(player_point_2d_t_to_Jplayer_point_2d_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_point_2d_t_to_buf(Jplayer_point_2d_t Jdata) {
    player_point_2d_t data = Jplayer_point_2d_t_to_player_point_2d_t(Jdata);
    return(playercore_java.player_point_2d_t_to_buf(data));
  }

  public static Jplayer_point_2d_t player_point_2d_t_to_Jplayer_point_2d_t(player_point_2d_t data) {
    Jplayer_point_2d_t Jdata = new Jplayer_point_2d_t();
    Jdata.px = data.getPx();
    Jdata.py = data.getPy();
    return(Jdata);
  }

  public static player_point_2d_t Jplayer_point_2d_t_to_player_point_2d_t(Jplayer_point_2d_t Jdata) {
    player_point_2d_t data = new player_point_2d_t();
    data.setPx(Jdata.px);
    data.setPy(Jdata.py);
    return(data);
  }

  public static Jplayer_point_3d_t buf_to_Jplayer_point_3d_t(SWIGTYPE_p_void buf) {
    player_point_3d_t data = playercore_java.buf_to_player_point_3d_t(buf);
    return(player_point_3d_t_to_Jplayer_point_3d_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_point_3d_t_to_buf(Jplayer_point_3d_t Jdata) {
    player_point_3d_t data = Jplayer_point_3d_t_to_player_point_3d_t(Jdata);
    return(playercore_java.player_point_3d_t_to_buf(data));
  }

  public static Jplayer_point_3d_t player_point_3d_t_to_Jplayer_point_3d_t(player_point_3d_t data) {
    Jplayer_point_3d_t Jdata = new Jplayer_point_3d_t();
    Jdata.px = data.getPx();
    Jdata.py = data.getPy();
    Jdata.pz = data.getPz();
    return(Jdata);
  }

  public static player_point_3d_t Jplayer_point_3d_t_to_player_point_3d_t(Jplayer_point_3d_t Jdata) {
    player_point_3d_t data = new player_point_3d_t();
    data.setPx(Jdata.px);
    data.setPy(Jdata.py);
    data.setPz(Jdata.pz);
    return(data);
  }

  public static Jplayer_orientation_3d_t buf_to_Jplayer_orientation_3d_t(SWIGTYPE_p_void buf) {
    player_orientation_3d_t data = playercore_java.buf_to_player_orientation_3d_t(buf);
    return(player_orientation_3d_t_to_Jplayer_orientation_3d_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_orientation_3d_t_to_buf(Jplayer_orientation_3d_t Jdata) {
    player_orientation_3d_t data = Jplayer_orientation_3d_t_to_player_orientation_3d_t(Jdata);
    return(playercore_java.player_orientation_3d_t_to_buf(data));
  }

  public static Jplayer_orientation_3d_t player_orientation_3d_t_to_Jplayer_orientation_3d_t(player_orientation_3d_t data) {
    Jplayer_orientation_3d_t Jdata = new Jplayer_orientation_3d_t();
    Jdata.proll = data.getProll();
    Jdata.ppitch = data.getPpitch();
    Jdata.pyaw = data.getPyaw();
    return(Jdata);
  }

  public static player_orientation_3d_t Jplayer_orientation_3d_t_to_player_orientation_3d_t(Jplayer_orientation_3d_t Jdata) {
    player_orientation_3d_t data = new player_orientation_3d_t();
    data.setProll(Jdata.proll);
    data.setPpitch(Jdata.ppitch);
    data.setPyaw(Jdata.pyaw);
    return(data);
  }

  public static Jplayer_pose2d_t buf_to_Jplayer_pose2d_t(SWIGTYPE_p_void buf) {
    player_pose2d_t data = playercore_java.buf_to_player_pose2d_t(buf);
    return(player_pose2d_t_to_Jplayer_pose2d_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_pose2d_t_to_buf(Jplayer_pose2d_t Jdata) {
    player_pose2d_t data = Jplayer_pose2d_t_to_player_pose2d_t(Jdata);
    return(playercore_java.player_pose2d_t_to_buf(data));
  }

  public static Jplayer_pose2d_t player_pose2d_t_to_Jplayer_pose2d_t(player_pose2d_t data) {
    Jplayer_pose2d_t Jdata = new Jplayer_pose2d_t();
    Jdata.px = data.getPx();
    Jdata.py = data.getPy();
    Jdata.pa = data.getPa();
    return(Jdata);
  }

  public static player_pose2d_t Jplayer_pose2d_t_to_player_pose2d_t(Jplayer_pose2d_t Jdata) {
    player_pose2d_t data = new player_pose2d_t();
    data.setPx(Jdata.px);
    data.setPy(Jdata.py);
    data.setPa(Jdata.pa);
    return(data);
  }

  public static Jplayer_pose3d_t buf_to_Jplayer_pose3d_t(SWIGTYPE_p_void buf) {
    player_pose3d_t data = playercore_java.buf_to_player_pose3d_t(buf);
    return(player_pose3d_t_to_Jplayer_pose3d_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_pose3d_t_to_buf(Jplayer_pose3d_t Jdata) {
    player_pose3d_t data = Jplayer_pose3d_t_to_player_pose3d_t(Jdata);
    return(playercore_java.player_pose3d_t_to_buf(data));
  }

  public static Jplayer_pose3d_t player_pose3d_t_to_Jplayer_pose3d_t(player_pose3d_t data) {
    Jplayer_pose3d_t Jdata = new Jplayer_pose3d_t();
    Jdata.px = data.getPx();
    Jdata.py = data.getPy();
    Jdata.pz = data.getPz();
    Jdata.proll = data.getProll();
    Jdata.ppitch = data.getPpitch();
    Jdata.pyaw = data.getPyaw();
    return(Jdata);
  }

  public static player_pose3d_t Jplayer_pose3d_t_to_player_pose3d_t(Jplayer_pose3d_t Jdata) {
    player_pose3d_t data = new player_pose3d_t();
    data.setPx(Jdata.px);
    data.setPy(Jdata.py);
    data.setPz(Jdata.pz);
    data.setProll(Jdata.proll);
    data.setPpitch(Jdata.ppitch);
    data.setPyaw(Jdata.pyaw);
    return(data);
  }

  public static Jplayer_bbox2d_t buf_to_Jplayer_bbox2d_t(SWIGTYPE_p_void buf) {
    player_bbox2d_t data = playercore_java.buf_to_player_bbox2d_t(buf);
    return(player_bbox2d_t_to_Jplayer_bbox2d_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_bbox2d_t_to_buf(Jplayer_bbox2d_t Jdata) {
    player_bbox2d_t data = Jplayer_bbox2d_t_to_player_bbox2d_t(Jdata);
    return(playercore_java.player_bbox2d_t_to_buf(data));
  }

  public static Jplayer_bbox2d_t player_bbox2d_t_to_Jplayer_bbox2d_t(player_bbox2d_t data) {
    Jplayer_bbox2d_t Jdata = new Jplayer_bbox2d_t();
    Jdata.sw = data.getSw();
    Jdata.sl = data.getSl();
    return(Jdata);
  }

  public static player_bbox2d_t Jplayer_bbox2d_t_to_player_bbox2d_t(Jplayer_bbox2d_t Jdata) {
    player_bbox2d_t data = new player_bbox2d_t();
    data.setSw(Jdata.sw);
    data.setSl(Jdata.sl);
    return(data);
  }

  public static Jplayer_bbox3d_t buf_to_Jplayer_bbox3d_t(SWIGTYPE_p_void buf) {
    player_bbox3d_t data = playercore_java.buf_to_player_bbox3d_t(buf);
    return(player_bbox3d_t_to_Jplayer_bbox3d_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_bbox3d_t_to_buf(Jplayer_bbox3d_t Jdata) {
    player_bbox3d_t data = Jplayer_bbox3d_t_to_player_bbox3d_t(Jdata);
    return(playercore_java.player_bbox3d_t_to_buf(data));
  }

  public static Jplayer_bbox3d_t player_bbox3d_t_to_Jplayer_bbox3d_t(player_bbox3d_t data) {
    Jplayer_bbox3d_t Jdata = new Jplayer_bbox3d_t();
    Jdata.sw = data.getSw();
    Jdata.sl = data.getSl();
    Jdata.sh = data.getSh();
    return(Jdata);
  }

  public static player_bbox3d_t Jplayer_bbox3d_t_to_player_bbox3d_t(Jplayer_bbox3d_t Jdata) {
    player_bbox3d_t data = new player_bbox3d_t();
    data.setSw(Jdata.sw);
    data.setSl(Jdata.sl);
    data.setSh(Jdata.sh);
    return(data);
  }

  public static Jplayer_blackboard_entry_t buf_to_Jplayer_blackboard_entry_t(SWIGTYPE_p_void buf) {
    player_blackboard_entry_t data = playercore_java.buf_to_player_blackboard_entry_t(buf);
    return(player_blackboard_entry_t_to_Jplayer_blackboard_entry_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_blackboard_entry_t_to_buf(Jplayer_blackboard_entry_t Jdata) {
    player_blackboard_entry_t data = Jplayer_blackboard_entry_t_to_player_blackboard_entry_t(Jdata);
    return(playercore_java.player_blackboard_entry_t_to_buf(data));
  }

  public static Jplayer_blackboard_entry_t player_blackboard_entry_t_to_Jplayer_blackboard_entry_t(player_blackboard_entry_t data) {
    Jplayer_blackboard_entry_t Jdata = new Jplayer_blackboard_entry_t();
    Jdata.key_count = data.getKey_count();
    Jdata.key = char*_to_Jchar*(data.getKey());
    Jdata.group_count = data.getGroup_count();
    Jdata.group = char*_to_Jchar*(data.getGroup());
    Jdata.type = data.getType();
    Jdata.subtype = data.getSubtype();
    Jdata.data_count = data.getData_count();
    Jdata.data = uint8_t*_to_Juint8_t*(data.getData());
    Jdata.timestamp_sec = data.getTimestamp_sec();
    Jdata.timestamp_usec = data.getTimestamp_usec();
    return(Jdata);
  }

  public static player_blackboard_entry_t Jplayer_blackboard_entry_t_to_player_blackboard_entry_t(Jplayer_blackboard_entry_t Jdata) {
    player_blackboard_entry_t data = new player_blackboard_entry_t();
    data.setKey_count(Jdata.key_count);
    data.setKey(Jchar*_to_char*(Jdata.key));
    data.setGroup_count(Jdata.group_count);
    data.setGroup(Jchar*_to_char*(Jdata.group));
    data.setType(Jdata.type);
    data.setSubtype(Jdata.subtype);
    data.setData_count(Jdata.data_count);
    data.setData(Juint8_t*_to_uint8_t*(Jdata.data));
    data.setTimestamp_sec(Jdata.timestamp_sec);
    data.setTimestamp_usec(Jdata.timestamp_usec);
    return(data);
  }

  public static Jplayer_segment_t buf_to_Jplayer_segment_t(SWIGTYPE_p_void buf) {
    player_segment_t data = playercore_java.buf_to_player_segment_t(buf);
    return(player_segment_t_to_Jplayer_segment_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_segment_t_to_buf(Jplayer_segment_t Jdata) {
    player_segment_t data = Jplayer_segment_t_to_player_segment_t(Jdata);
    return(playercore_java.player_segment_t_to_buf(data));
  }

  public static Jplayer_segment_t player_segment_t_to_Jplayer_segment_t(player_segment_t data) {
    Jplayer_segment_t Jdata = new Jplayer_segment_t();
    Jdata.x0 = data.getX0();
    Jdata.y0 = data.getY0();
    Jdata.x1 = data.getX1();
    Jdata.y1 = data.getY1();
    return(Jdata);
  }

  public static player_segment_t Jplayer_segment_t_to_player_segment_t(Jplayer_segment_t Jdata) {
    player_segment_t data = new player_segment_t();
    data.setX0(Jdata.x0);
    data.setY0(Jdata.y0);
    data.setX1(Jdata.x1);
    data.setY1(Jdata.y1);
    return(data);
  }

  public static Jplayer_extent2d_t buf_to_Jplayer_extent2d_t(SWIGTYPE_p_void buf) {
    player_extent2d_t data = playercore_java.buf_to_player_extent2d_t(buf);
    return(player_extent2d_t_to_Jplayer_extent2d_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_extent2d_t_to_buf(Jplayer_extent2d_t Jdata) {
    player_extent2d_t data = Jplayer_extent2d_t_to_player_extent2d_t(Jdata);
    return(playercore_java.player_extent2d_t_to_buf(data));
  }

  public static Jplayer_extent2d_t player_extent2d_t_to_Jplayer_extent2d_t(player_extent2d_t data) {
    Jplayer_extent2d_t Jdata = new Jplayer_extent2d_t();
    Jdata.x0 = data.getX0();
    Jdata.y0 = data.getY0();
    Jdata.x1 = data.getX1();
    Jdata.y1 = data.getY1();
    return(Jdata);
  }

  public static player_extent2d_t Jplayer_extent2d_t_to_player_extent2d_t(Jplayer_extent2d_t Jdata) {
    player_extent2d_t data = new player_extent2d_t();
    data.setX0(Jdata.x0);
    data.setY0(Jdata.y0);
    data.setX1(Jdata.x1);
    data.setY1(Jdata.y1);
    return(data);
  }

  public static Jplayer_color_t buf_to_Jplayer_color_t(SWIGTYPE_p_void buf) {
    player_color_t data = playercore_java.buf_to_player_color_t(buf);
    return(player_color_t_to_Jplayer_color_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_color_t_to_buf(Jplayer_color_t Jdata) {
    player_color_t data = Jplayer_color_t_to_player_color_t(Jdata);
    return(playercore_java.player_color_t_to_buf(data));
  }

  public static Jplayer_color_t player_color_t_to_Jplayer_color_t(player_color_t data) {
    Jplayer_color_t Jdata = new Jplayer_color_t();
    Jdata.alpha = data.getAlpha();
    Jdata.red = data.getRed();
    Jdata.green = data.getGreen();
    Jdata.blue = data.getBlue();
    return(Jdata);
  }

  public static player_color_t Jplayer_color_t_to_player_color_t(Jplayer_color_t Jdata) {
    player_color_t data = new player_color_t();
    data.setAlpha(Jdata.alpha);
    data.setRed(Jdata.red);
    data.setGreen(Jdata.green);
    data.setBlue(Jdata.blue);
    return(data);
  }

  public static Jplayer_bool_t buf_to_Jplayer_bool_t(SWIGTYPE_p_void buf) {
    player_bool_t data = playercore_java.buf_to_player_bool_t(buf);
    return(player_bool_t_to_Jplayer_bool_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_bool_t_to_buf(Jplayer_bool_t Jdata) {
    player_bool_t data = Jplayer_bool_t_to_player_bool_t(Jdata);
    return(playercore_java.player_bool_t_to_buf(data));
  }

  public static Jplayer_bool_t player_bool_t_to_Jplayer_bool_t(player_bool_t data) {
    Jplayer_bool_t Jdata = new Jplayer_bool_t();
    Jdata.state = data.getState();
    return(Jdata);
  }

  public static player_bool_t Jplayer_bool_t_to_player_bool_t(Jplayer_bool_t Jdata) {
    player_bool_t data = new player_bool_t();
    data.setState(Jdata.state);
    return(data);
  }

  public static Jplayer_uint32_t buf_to_Jplayer_uint32_t(SWIGTYPE_p_void buf) {
    player_uint32_t data = playercore_java.buf_to_player_uint32_t(buf);
    return(player_uint32_t_to_Jplayer_uint32_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_uint32_t_to_buf(Jplayer_uint32_t Jdata) {
    player_uint32_t data = Jplayer_uint32_t_to_player_uint32_t(Jdata);
    return(playercore_java.player_uint32_t_to_buf(data));
  }

  public static Jplayer_uint32_t player_uint32_t_to_Jplayer_uint32_t(player_uint32_t data) {
    Jplayer_uint32_t Jdata = new Jplayer_uint32_t();
    Jdata.value = data.getValue();
    return(Jdata);
  }

  public static player_uint32_t Jplayer_uint32_t_to_player_uint32_t(Jplayer_uint32_t Jdata) {
    player_uint32_t data = new player_uint32_t();
    data.setValue(Jdata.value);
    return(data);
  }

  public static Jplayer_capabilities_req_t buf_to_Jplayer_capabilities_req_t(SWIGTYPE_p_void buf) {
    player_capabilities_req_t data = playercore_java.buf_to_player_capabilities_req_t(buf);
    return(player_capabilities_req_t_to_Jplayer_capabilities_req_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_capabilities_req_t_to_buf(Jplayer_capabilities_req_t Jdata) {
    player_capabilities_req_t data = Jplayer_capabilities_req_t_to_player_capabilities_req_t(Jdata);
    return(playercore_java.player_capabilities_req_t_to_buf(data));
  }

  public static Jplayer_capabilities_req_t player_capabilities_req_t_to_Jplayer_capabilities_req_t(player_capabilities_req_t data) {
    Jplayer_capabilities_req_t Jdata = new Jplayer_capabilities_req_t();
    Jdata.type = data.getType();
    Jdata.subtype = data.getSubtype();
    return(Jdata);
  }

  public static player_capabilities_req_t Jplayer_capabilities_req_t_to_player_capabilities_req_t(Jplayer_capabilities_req_t Jdata) {
    player_capabilities_req_t data = new player_capabilities_req_t();
    data.setType(Jdata.type);
    data.setSubtype(Jdata.subtype);
    return(data);
  }

  public static Jplayer_intprop_req_t buf_to_Jplayer_intprop_req_t(SWIGTYPE_p_void buf) {
    player_intprop_req_t data = playercore_java.buf_to_player_intprop_req_t(buf);
    return(player_intprop_req_t_to_Jplayer_intprop_req_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_intprop_req_t_to_buf(Jplayer_intprop_req_t Jdata) {
    player_intprop_req_t data = Jplayer_intprop_req_t_to_player_intprop_req_t(Jdata);
    return(playercore_java.player_intprop_req_t_to_buf(data));
  }

  public static Jplayer_intprop_req_t player_intprop_req_t_to_Jplayer_intprop_req_t(player_intprop_req_t data) {
    Jplayer_intprop_req_t Jdata = new Jplayer_intprop_req_t();
    Jdata.key_count = data.getKey_count();
    Jdata.*key = data.get*key();
    Jdata.value = data.getValue();
    return(Jdata);
  }

  public static player_intprop_req_t Jplayer_intprop_req_t_to_player_intprop_req_t(Jplayer_intprop_req_t Jdata) {
    player_intprop_req_t data = new player_intprop_req_t();
    data.setKey_count(Jdata.key_count);
    data.set*key(Jdata.*key);
    data.setValue(Jdata.value);
    return(data);
  }

  public static Jplayer_dblprop_req_t buf_to_Jplayer_dblprop_req_t(SWIGTYPE_p_void buf) {
    player_dblprop_req_t data = playercore_java.buf_to_player_dblprop_req_t(buf);
    return(player_dblprop_req_t_to_Jplayer_dblprop_req_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_dblprop_req_t_to_buf(Jplayer_dblprop_req_t Jdata) {
    player_dblprop_req_t data = Jplayer_dblprop_req_t_to_player_dblprop_req_t(Jdata);
    return(playercore_java.player_dblprop_req_t_to_buf(data));
  }

  public static Jplayer_dblprop_req_t player_dblprop_req_t_to_Jplayer_dblprop_req_t(player_dblprop_req_t data) {
    Jplayer_dblprop_req_t Jdata = new Jplayer_dblprop_req_t();
    Jdata.key_count = data.getKey_count();
    Jdata.*key = data.get*key();
    Jdata.value = data.getValue();
    return(Jdata);
  }

  public static player_dblprop_req_t Jplayer_dblprop_req_t_to_player_dblprop_req_t(Jplayer_dblprop_req_t Jdata) {
    player_dblprop_req_t data = new player_dblprop_req_t();
    data.setKey_count(Jdata.key_count);
    data.set*key(Jdata.*key);
    data.setValue(Jdata.value);
    return(data);
  }

  public static Jplayer_strprop_req_t buf_to_Jplayer_strprop_req_t(SWIGTYPE_p_void buf) {
    player_strprop_req_t data = playercore_java.buf_to_player_strprop_req_t(buf);
    return(player_strprop_req_t_to_Jplayer_strprop_req_t(data));
  }

  public static SWIGTYPE_p_void Jplayer_strprop_req_t_to_buf(Jplayer_strprop_req_t Jdata) {
    player_strprop_req_t data = Jplayer_strprop_req_t_to_player_strprop_req_t(Jdata);
    return(playercore_java.player_strprop_req_t_to_buf(data));
  }

  public static Jplayer_strprop_req_t player_strprop_req_t_to_Jplayer_strprop_req_t(player_strprop_req_t data) {
    Jplayer_strprop_req_t Jdata = new Jplayer_strprop_req_t();
    Jdata.key_count = data.getKey_count();
    Jdata.*key = data.get*key();
    Jdata.value_count = data.getValue_count();
    Jdata.*value = data.get*value();
    return(Jdata);
  }

  public static player_strprop_req_t Jplayer_strprop_req_t_to_player_strprop_req_t(Jplayer_strprop_req_t Jdata) {
    player_strprop_req_t data = new player_strprop_req_t();
    data.setKey_count(Jdata.key_count);
    data.set*key(Jdata.*key);
    data.setValue_count(Jdata.value_count);
    data.set*value(Jdata.*value);
    return(data);
  }


}
