package lcmtypes;
 
import java.io.*;
import java.util.*;
 
public class differential_drive_command_t implements lcm.lcm.LCMEncodable
{
    public long utime;
    public double left;
    public boolean left_enabled;
    public double right;
    public boolean right_enabled;
 
    public differential_drive_command_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x2ab615ed58b54790L;
 
    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class>());
    }
 
    public static long _hashRecursive(ArrayList<Class> classes)
    {
        if (classes.contains(lcmtypes.differential_drive_command_t.class))
            return 0L;
 
        classes.add(lcmtypes.differential_drive_command_t.class);
        long hash = LCM_FINGERPRINT_BASE
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
 
        outs.writeDouble(this.left); 
 
        outs.writeByte( this.left_enabled ? 1 : 0); 
 
        outs.writeDouble(this.right); 
 
        outs.writeByte( this.right_enabled ? 1 : 0); 
 
    }
 
    public differential_drive_command_t(DataInputStream ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static lcmtypes.differential_drive_command_t _decodeRecursiveFactory(DataInputStream ins) throws IOException
    {
        lcmtypes.differential_drive_command_t o = new lcmtypes.differential_drive_command_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInputStream ins) throws IOException
    {
        byte[] __strbuf = null;
        this.utime = ins.readLong();
 
        this.left = ins.readDouble();
 
        this.left_enabled = ins.readByte()!=0;
 
        this.right = ins.readDouble();
 
        this.right_enabled = ins.readByte()!=0;
 
    }
 
    public lcmtypes.differential_drive_command_t copy()
    {
        lcmtypes.differential_drive_command_t outobj = new lcmtypes.differential_drive_command_t();
        outobj.utime = this.utime;
 
        outobj.left = this.left;
 
        outobj.left_enabled = this.left_enabled;
 
        outobj.right = this.right;
 
        outobj.right_enabled = this.right_enabled;
 
        return outobj;
    }
 
}

