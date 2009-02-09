package net.sourceforge.playerstage.Jplayercore;
import java.io.Serializable;
public class Jplayer_blackboard_entry_t implements Serializable {
  public final static long serialVersionUID = 7161378214038789771L;
  public long key_count;
  public Jchar* key;
  public long group_count;
  public Jchar* group;
  public int type;
  public int subtype;
  public long data_count;
  public Juint8_t* data;
  public long timestamp_sec;
  public long timestamp_usec;
  public Jplayer_blackboard_entry_t() {
    key = new Jchar*();
    group = new Jchar*();
    data = new Juint8_t*();
  }
}
