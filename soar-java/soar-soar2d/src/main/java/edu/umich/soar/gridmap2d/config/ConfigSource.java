package edu.umich.soar.gridmap2d.config;

import java.io.FileNotFoundException;

/** Abstract source of configuration information. **/
public abstract class ConfigSource {
	public abstract boolean hasKey(String key);

	public abstract String[] getKeys(String rootWithDot);

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
	
	public abstract void save(String path) throws FileNotFoundException;
	
	public abstract ConfigSource copy();

	public abstract void removeKey(String key);
}
