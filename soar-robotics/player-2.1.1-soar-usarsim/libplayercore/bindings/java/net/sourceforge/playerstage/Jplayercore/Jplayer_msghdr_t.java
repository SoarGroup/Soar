package net.sourceforge.playerstage.Jplayercore;
import java.io.Serializable;
public class Jplayer_msghdr_t implements Serializable {
  public final static long serialVersionUID = 4569190411700822117L;
  public Jplayer_devaddr_t addr;
  public short type;
  public short subtype;
  public double timestamp;
  public long seq;
  public long size;
  public Jplayer_msghdr_t() {
    addr = new Jplayer_devaddr_t();
  }
}
