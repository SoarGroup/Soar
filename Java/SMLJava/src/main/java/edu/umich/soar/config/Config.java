package edu.umich.soar.config;

/**
 * A view into a ConfigSource. The view can have a specific
 * "scope", i.e., a prefix that is automatically added to every
 * get/set request.
 *
 **/
public class Config
{
    ConfigSource source;
    Config       parent;

    String       prefix; // either empty or has a trailing "." so that prefix+key is always well-formed.

    boolean      inherit = true; // can a.c be an answer to a.b.c?

    public Config(ConfigSource source)
    {
	this.source = source;
	this.prefix = "";
	this.parent = this;
    }

    public Config getParent()
    {
	return parent;
    }

    public Config getChild(String childprefix)
    {
	Config child = new Config(source);
	child.parent = this.parent;
	
	child.prefix = this.prefix;
	if (child.prefix.length() > 0 && !child.prefix.endsWith("."))
	    child.prefix = child.prefix+".";
	child.prefix = child.prefix+childprefix+".";
	child.inherit = inherit;

	return child;
    }

    public boolean hasKey(String key)
    {
	return resolveKey(key) != null;
    }

    public String[] getKeys()
    {
	if (prefix.isEmpty())
	    return source.getKeys(prefix);
	else
	    return source.getKeys(prefix.substring(0, prefix.length() - 1));
    }

    void missingRequired(String key)
    {
	System.out.println("Config: Required key '"+key+"' missing.");
	assert(false);
    }

    // find the the fully-resolved key that is the most recent
    // ancestor of the key.  i.e., abc.def.ghi returns abc.def.ghi if
    // it exists, else abc.ghi, else ghi.
    //
    // Returns a resolved key if it exists, otherwise null.
    String resolveKey(String key)
    {
	String keypath = prefix+key;
	if (!inherit) {
	    if (source.hasKey(keypath))
		return keypath;
	    return null;
	}

	String toks[] = (keypath).split("\\.");

	for (int i = toks.length-1; i >= 0; i--) {
	    String kp = "";

	    for (int j = 0; j < i; j++) {
		kp = kp + toks[j] + ".";
	    }

	    kp += toks[toks.length-1];

	    if (source.hasKey(kp))
		return kp;
	}

	return null;
    }

    ////////////////////////////
    // int
    public int[] getInts(String key)
    {
	return getInts(key, null);
    }

    public int[] getInts(String key, int defaults[])
    {
	int v[] = source.getInts(resolveKey(key));
	return (v==null) ? defaults : v;
    }

    public int[] requireInts(String key)
    {
	int v[] = getInts(key, null);
	if (v == null) 
	    missingRequired(key);
	return v;
    }

    public int getInt(String key, int def)
    {
	return getInts(key, new int[] { def })[0];
    }

    public int requireInt(String key)
    { 
	int v[] = getInts(key, null);
	if (v == null) 
	    missingRequired(key);
	return v[0];
    }

    public void setInt(String key, int v)
    {
	source.setInts(key, new int[] {v});
    }

    public void setInts(String key, int v[])
    {
	source.setInts(key, v);
    }
    
    ///////////////////////////
    // Paths
    public String getPath(String key, String def)
    {
	String path = getString(key, def);
	if (path == null) 
	    return null;

	path = path.trim(); // remove white space
	if (!path.startsWith("/")) 
	    return source.getPrefixPath() + path;

	return path; 
    }

    public String getPath(String key)
    {
	return getPath(key, null);
    }

    ///////////////////////////
    // String

    // All other string ops written in terms of this one.
    public String[] getStrings(String key, String defaults[])
    {
	String v[] = source.getStrings(resolveKey(key));
	return (v==null) ? defaults : v;
    }

    public String[] getStrings(String key)
    {
	return getStrings(key, null);
    }

    public String[] requireStrings(String key)
    {
	String v[] = getStrings(key, null);
	if (v == null) 
	    missingRequired(key);
	return v;
    }

    public String getString(String key)
    {
	return getString(key, null);
    }

    public String getString(String key, String def)
    {
	return getStrings(key, new String[] { def })[0];
    }

    public String requireString(String key)
    { 
	String v[] = getStrings(key, null);
	if (v == null) 
	    missingRequired(key);

	return v[0];
    }

    public void setString(String key, String v)
    {
	source.setStrings(prefix+key, new String[] {v});
    }

    public void setStrings(String key, String v[])
    {
	source.setStrings(prefix+key, v);
    }

    ////////////////////////////
    // boolean

    public boolean[] getBooleans(String key, boolean defaults[])
    {
	boolean v[] = source.getBooleans(resolveKey(key));
	return (v==null) ? defaults : v;
    }

    public boolean[] getBooleans(String key)
    {
	return getBooleans(key, null);
    }

    public boolean[] requireBooleans(String key)
    {
	boolean v[] = getBooleans(key, null);
	if (v == null) 
	    missingRequired(key);
	return v;
    }

    public boolean getBoolean(String key, boolean def)
    {
	return getBooleans(key, new boolean[] { def })[0];
    }

    public boolean requireBoolean(String key)
    { 
	boolean v[] = getBooleans(key);
	if (v == null) 
	    missingRequired(key);
	return v[0];
    }

    public void setBoolean(String key, boolean v)
    {
	source.setBooleans(prefix+key, new boolean[] {v});
    }

    public void setBooleans(String key, boolean v[])
    {
	source.setBooleans(prefix+key, v);
    }

    ////////////////////////////
    // double
    public double[] getDoubles(String key, double defaults[])
    {
	double v[] = source.getDoubles(resolveKey(key));
	return (v==null) ? defaults : v;
    }

    public double[] getDoubles(String key)
    {
	return getDoubles(key, null);
    }

    public double[] requireDoubles(String key)
    {
	double v[] = getDoubles(key, null);
	if (v == null) 
	    missingRequired(key);
	return v;
    }

    public double getDouble(String key, double def)
    {
	return getDoubles(key, new double[] {def})[0];
    }

    public double requireDouble(String key)
    { 
	double v[] = getDoubles(key, null);
	if (v == null) 
	    missingRequired(key);
	return v[0];
    }

    public void setDouble(String key, double v)
    {
	source.setDoubles(prefix+key, new double[] {v});
    }

    public void setDoubles(String key, double v[])
    {
	source.setDoubles(prefix+key, v);
    }

    ////////////////////////////
    // double
    public byte[] getBytes(String key, byte defaults[])
    {
	byte v[] = source.getBytes(resolveKey(key));
	return (v==null) ? defaults : v;
    }

    public byte[] requireBytes(String key)
    {
	byte v[] = getBytes(key, null);
	if (v == null) 
	    missingRequired(key);
	return v;
    }

    public void setBytes(String key, byte v[])
    {
	source.setBytes(prefix+key, v);
    }

    public void merge(Config includedConfig) {
	for (String key : includedConfig.getKeys()) {
	    this.setStrings(key, includedConfig.getStrings(key));
	}
    }
}

