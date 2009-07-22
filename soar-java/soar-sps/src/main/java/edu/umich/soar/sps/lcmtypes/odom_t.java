package edu.umich.soar.sps.lcmtypes;
 
import java.io.*;
import java.util.*;
 
public class odom_t implements lcm.lcm.LCMEncodable
{
    public long utime;
    public int left;
    public int right;
 
    public odom_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x70588cffc58d48a7L;
 
    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class>());
    }
 
    public static long _hashRecursive(ArrayList<Class> classes)
    {
        if (classes.contains(edu.umich.soar.sps.lcmtypes.odom_t.class))
            return 0L;
 
        classes.add(edu.umich.soar.sps.lcmtypes.odom_t.class);
        long hash = LCM_FINGERPRINT_BASE
            ;
        classes.remove(classes.size() - 1);
        return (hash<<1) + ((hash>>63)&1);
    }
 
    public void encode(DataOutput outs) throws IOException
    {
        outs.writeLong(LCM_FINGERPRINT);
        _encodeRecursive(outs);
    }
 
    public void _encodeRecursive(DataOutput outs) throws IOException
    {
        byte[] __strbuf = null;
        outs.writeLong(this.utime); 
 
        outs.writeInt(this.left); 
 
        outs.writeInt(this.right); 
 
    }
 
    public odom_t(DataInputStream ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static edu.umich.soar.sps.lcmtypes.odom_t _decodeRecursiveFactory(DataInputStream ins) throws IOException
    {
        edu.umich.soar.sps.lcmtypes.odom_t o = new edu.umich.soar.sps.lcmtypes.odom_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInputStream ins) throws IOException
    {
        byte[] __strbuf = null;
        this.utime = ins.readLong();
 
        this.left = ins.readInt();
 
        this.right = ins.readInt();
 
    }
 
    public edu.umich.soar.sps.lcmtypes.odom_t copy()
    {
        edu.umich.soar.sps.lcmtypes.odom_t outobj = new edu.umich.soar.sps.lcmtypes.odom_t();
        outobj.utime = this.utime;
 
        outobj.left = this.left;
 
        outobj.right = this.right;
 
        return outobj;
    }
 
}

