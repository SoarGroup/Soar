package soar2d.config;

/**
 * A view into a ConfigSource. The view can have a specific "scope", i.e., a
 * prefix that is automatically added to every get/set request.
 * 
 **/
public class Config {
	ConfigSource source;
	Config parent;

	String prefix; // has a trailing "." as necessary.

	public Config(ConfigSource source) {
		this.source = source;
		this.parent = null;
		this.prefix = "";
	}

	public Config getParent() {
		return parent;
	}

	public Config getChild(String childprefix) {
		Config child = new Config(source);
		child.parent = this;

		child.prefix = this.prefix;
		if (child.prefix.length() > 0)
			child.prefix = child.prefix + ".";
		child.prefix = child.prefix + childprefix + ".";

		return child;
	}

	public boolean hasKey(String key) {
		return source.hasKey(prefix + key);
	}

	void missingRequired(String key) {
		System.out.println("Config: Required key '" + key + "' missing.");
		assert (false);
	}

	// //////////////////////////
	// int
	public int[] getInts(String key) {
		return getInts(key, null);
	}

	public int[] getInts(String key, int defaults[]) {
		int v[] = source.getInts(prefix + key);
		return (v == null) ? defaults : v;
	}

	public int[] requireInts(String key) {
		int v[] = source.getInts(prefix + key);
		if (v == null)
			missingRequired(prefix + key);
		return v;
	}

	public int getInt(String key, int def) {
		int v[] = source.getInts(prefix + key);
		return (v == null) ? def : v[0];
	}

	public int requireInt(String key) {
		int v[] = source.getInts(prefix + key);
		if (v == null)
			missingRequired(prefix + key);
		return v[0];
	}

	public void setInt(String key, int v) {
		source.setInts(prefix + key, new int[] { v });
	}

	public void setInts(String key, int v[]) {
		source.setInts(prefix + key, v);
	}

	// /////////////////////////
	// String
	public String[] getStrings(String key) {
		return getStrings(key, null);
	}

	public String[] getStrings(String key, String defaults[]) {
		String v[] = source.getStrings(prefix + key);
		return (v == null) ? defaults : v;
	}

	public String[] requireStrings(String key) {
		String v[] = source.getStrings(prefix + key);
		if (v == null)
			missingRequired(prefix + key);
		return v;
	}

	public String getString(String key) {
		return getString(key, null);
	}

	public String getString(String key, String def) {
		String v[] = source.getStrings(prefix + key);
		return (v == null) ? def : v[0];
	}

	public String requireString(String key) {
		String v[] = source.getStrings(prefix + key);
		if (v == null)
			missingRequired(prefix + key);

		return v[0];
	}

	public void setString(String key, String v) {
		source.setStrings(prefix + key, new String[] { v });
	}

	public void setStrings(String key, String v[]) {
		source.setStrings(prefix + key, v);
	}

	// //////////////////////////
	// boolean
	public boolean[] getBooleans(String key) {
		return getBooleans(key, null);
	}

	public boolean[] getBooleans(String key, boolean defaults[]) {
		boolean v[] = source.getBooleans(prefix + key);
		return (v == null) ? defaults : v;
	}

	public boolean[] requireBooleans(String key) {
		boolean v[] = source.getBooleans(prefix + key);
		if (v == null)
			missingRequired(prefix + key);
		return v;
	}

	public boolean getBoolean(String key, boolean def) {
		boolean v[] = source.getBooleans(prefix + key);
		return (v == null) ? def : v[0];
	}

	public boolean requireBoolean(String key) {
		boolean v[] = source.getBooleans(prefix + key);
		if (v == null)
			missingRequired(prefix + key);
		return v[0];
	}

	public void setBoolean(String key, boolean v) {
		source.setBooleans(prefix + key, new boolean[] { v });
	}

	public void setBooleans(String key, boolean v[]) {
		source.setBooleans(prefix + key, v);
	}

	// //////////////////////////
	// double
	public double[] getDoubles(String key) {
		return getDoubles(key, null);
	}

	public double[] getDoubles(String key, double defaults[]) {
		double v[] = source.getDoubles(prefix + key);
		return (v == null) ? defaults : v;
	}

	public double[] requireDoubles(String key) {
		double v[] = source.getDoubles(prefix + key);
		if (v == null)
			missingRequired(prefix + key);
		return v;
	}

	public double getDouble(String key, double def) {
		double v[] = source.getDoubles(prefix + key);
		return (v == null) ? def : v[0];
	}

	public double requireDouble(String key) {
		double v[] = source.getDoubles(prefix + key);
		if (v == null)
			missingRequired(prefix + key);
		return v[0];
	}

	public void setDouble(String key, double v) {
		source.setDoubles(prefix + key, new double[] { v });
	}

	public void setDoubles(String key, double v[]) {
		source.setDoubles(prefix + key, v);
	}

	// //////////////////////////
	// double
	public byte[] getBytes(String key, byte defaults[]) {
		byte v[] = source.getBytes(prefix + key);
		return (v == null) ? defaults : v;
	}

	public byte[] requireBytes(String key) {
		byte v[] = source.getBytes(prefix + key);
		if (v == null)
			missingRequired(prefix + key);
		return v;
	}

	public void setBytes(String key, byte v[]) {
		source.setBytes(prefix + key, v);
	}
}
