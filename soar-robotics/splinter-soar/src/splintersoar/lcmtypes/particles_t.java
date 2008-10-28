package splintersoar.lcmtypes;
 
import java.io.*;
import java.util.*;
 
public class particles_t implements lcm.lcm.LCMEncodable
{
    public long utime;
    public int nparticles;
    public double particle[][];
 
    public particles_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x82c1c7b491ef5a37L;
 
    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class>());
    }
 
    public static long _hashRecursive(ArrayList<Class> classes)
    {
        if (classes.contains(splintersoar.lcmtypes.particles_t.class))
            return 0L;
 
        classes.add(splintersoar.lcmtypes.particles_t.class);
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
 
        outs.writeInt(this.nparticles); 
 
        for (int a = 0; a < nparticles; a++) {
            for (int b = 0; b < 3; b++) {
                outs.writeDouble(this.particle[a][b]); 
            }
            }
 
    }
 
    public particles_t(DataInputStream ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static splintersoar.lcmtypes.particles_t _decodeRecursiveFactory(DataInputStream ins) throws IOException
    {
        splintersoar.lcmtypes.particles_t o = new splintersoar.lcmtypes.particles_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInputStream ins) throws IOException
    {
        byte[] __strbuf = null;
        this.utime = ins.readLong();
 
        this.nparticles = ins.readInt();
 
        this.particle = new double[(int) nparticles][(int) 3];
        for (int a = 0; a < nparticles; a++) {
            for (int b = 0; b < 3; b++) {
                this.particle[a][b] = ins.readDouble();
            }
            }
 
    }
 
    public splintersoar.lcmtypes.particles_t copy()
    {
        splintersoar.lcmtypes.particles_t outobj = new splintersoar.lcmtypes.particles_t();
        outobj.utime = this.utime;
 
        outobj.nparticles = this.nparticles;
 
        outobj.particle = new double[(int) nparticles][(int) 3];
        for (int a = 0; a < nparticles; a++) {
            for (int b = 0; b < 3; b++) {
                outobj.particle = this.particle;
            }
            }
 
        return outobj;
    }
 
}

