package lcmtypes;
 
import java.io.*;
import java.util.*;
 
public class waypoints_t implements lcm.lcm.LCMEncodable
{
    public long utime;
    public int nwaypoints;
    public String names[];
    public lcmtypes.xy_t locations[];
 
    public waypoints_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x850dc593109c3e7dL;
 
    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class>());
    }
 
    public static long _hashRecursive(ArrayList<Class> classes)
    {
        if (classes.contains(lcmtypes.waypoints_t.class))
            return 0L;
 
        classes.add(lcmtypes.waypoints_t.class);
        long hash = LCM_FINGERPRINT_BASE
             + lcmtypes.xy_t._hashRecursive(classes)
            ;
        classes.remove(classes.size() - 1);
        return (hash<<1) + ((hash>>63)&1);
    }
 
    public void encode(DataOutputStream outs) throws IOException
    {
        outs.writeLong(LCM_FINGERPRINT);
        _encodeRecursive(outs);
    }
 
    public void _encodeRecursive(DataOutputStream outs) throws IOException
    {
        byte[] __strbuf = null;
        outs.writeLong(this.utime); 
 
        outs.writeInt(this.nwaypoints); 
 
        for (int a = 0; a < nwaypoints; a++) {
            __strbuf = this.names[a].getBytes("UTF-8"); outs.writeInt(__strbuf.length+1); outs.write(__strbuf, 0, __strbuf.length); outs.writeByte(0); 
        }
 
        for (int a = 0; a < nwaypoints; a++) {
            this.locations[a]._encodeRecursive(outs); 
        }
 
    }
 
    public waypoints_t(DataInputStream ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static lcmtypes.waypoints_t _decodeRecursiveFactory(DataInputStream ins) throws IOException
    {
        lcmtypes.waypoints_t o = new lcmtypes.waypoints_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInputStream ins) throws IOException
    {
        byte[] __strbuf = null;
        this.utime = ins.readLong();
 
        this.nwaypoints = ins.readInt();
 
        this.names = new String[(int) nwaypoints];
        for (int a = 0; a < nwaypoints; a++) {
            __strbuf = new byte[ins.readInt()-1]; ins.readFully(__strbuf); ins.readByte(); this.names[a] = new String(__strbuf, "UTF-8");
        }
 
        this.locations = new lcmtypes.xy_t[(int) nwaypoints];
        for (int a = 0; a < nwaypoints; a++) {
            this.locations[a] = lcmtypes.xy_t._decodeRecursiveFactory(ins);
        }
 
    }
 
    public lcmtypes.waypoints_t copy()
    {
        lcmtypes.waypoints_t outobj = new lcmtypes.waypoints_t();
        outobj.utime = this.utime;
 
        outobj.nwaypoints = this.nwaypoints;
 
        outobj.names = new String[(int) nwaypoints];
        for (int a = 0; a < nwaypoints; a++) {
            outobj.names[a] = this.names[a];
        }
 
        outobj.locations = new lcmtypes.xy_t[(int) nwaypoints];
        for (int a = 0; a < nwaypoints; a++) {
            outobj.locations[a] = this.locations[a].copy();
        }
 
        return outobj;
    }
 
}

