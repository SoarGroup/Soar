package edu.umich.soar.config;

/** Abstract source of configuration information. **/
public abstract class ConfigSource
{
    public abstract String getPrefixPath();

    public abstract boolean hasKey(String key);

    public abstract String[] getKeys(String root);

    public abstract int[] getInts(String key);

    public abstract void setInts(String key, int v[]);

    public abstract String[] getStrings(String key);

    public abstract void setStrings(String key, String v[]);

    public abstract boolean[] getBooleans(String key);

    public abstract void setBooleans(String key, boolean v[]);

    public abstract double[] getDoubles(String key);

    public abstract void setDoubles(String key, double v[]);

    public abstract byte[] getBytes(String key);

    public abstract void setBytes(String key, byte v[]);
}
