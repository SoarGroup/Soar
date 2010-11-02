package edu.umich.soar.gridmap2d.map;

import java.util.HashSet;
import java.util.Set;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.atomic.AtomicLong;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.config.Config;

/**
 * Essentially houses a concurrent property map that converts its values from
 * String types to a few other types (Boolean, Double, Float, Integer) on demand
 * using generics. The converted values are then stored in the array as their
 * new type under the assumption that's how they are to be used in the future.
 * This has an effect of losing precision when floating point numbers are
 * converted away from their initial value.
 * 
 * Objects that are placed in the property list as a specific type stay that
 * way.
 * 
 * @author voigtjr
 * 
 */
public class CellObject implements Comparable<CellObject> {
	private static final Log logger = LogFactory.getLog(CellObject.class);
	private static final AtomicLong nextSerial = new AtomicLong();

	/**
	 * Children keys affected by property apply start with this prefix.
	 * 
	 * @see CellObject#doApplyProperties()
	 */
	private final String APPLY_PROPERTIES = "apply.properties.";

	/**
	 * The property source map, holding arbitrary keys mapped to properties.
	 * Null keys and values are not supported.
	 */
	private final ConcurrentMap<String, Object> properties = new ConcurrentHashMap<String, Object>();

	/**
	 * The cell that this object is currently in. Might be null if not currently
	 * on the map (could be carried or in a box or something).
	 */
	private volatile Cell cell;

	/**
	 * Cell's unique identifying number.
	 */
	private final long serial;

	/**
	 * Construct a new, empty cell object.
	 */
	CellObject() {
		serial = nextSerial.getAndIncrement();
	}

	/**
	 * Copy constructor, creates a clone of the passed cell object. All objects
	 * are unique. The new object will share the same cell reference as the
	 * passed object.
	 * 
	 * @param other
	 *            The object to be copied.
	 */
	CellObject(CellObject other) {
		serial = nextSerial.getAndIncrement();
		this.properties.putAll(other.properties);
		this.cell = other.cell;
	}

	/**
	 * Create a new object from a config file. Adds all key/value pairs to the
	 * new object's property list. If the config file value is a single-element
	 * array, the element is added as a single String instance instead of a one
	 * element String array. Emtpy String arrays are ignored. Mulitple-element
	 * String arrays are saved as-is.
	 * 
	 * @param config
	 */
	CellObject(Config config) {
		serial = nextSerial.getAndIncrement();
		for (String key : config.getKeys()) {
			String[] strings = config.getStrings(key);
			if (strings != null) {
				if (strings.length == 0) {
					continue;
				} else if (strings.length == 1) {
					this.properties.put(key, strings[0]);
				} else {
					this.properties.put(key, strings);
				}
			}
		}
	}

	/**
	 * Returns the last recorded containing cell. Can be null, indicating the
	 * cell is not on the map.
	 * 
	 * @return The cell reference this object is in, or null
	 */
	public Cell getCell() {
		return cell;
	}

	/**
	 * Sets the containing cell. Can be null, indicating the cell is not on the
	 * map. Should only be called by the cell during an add/remove object.
	 * 
	 * @param cell
	 *            The cell reference this object is in, or null.
	 */
	void setCell(Cell cell) {
		this.cell = cell;
	}

	/**
	 * Puts a new property in the property list, replacing an existing property
	 * if one exists.
	 * 
	 * @throws NullPointerException
	 *             if the key or the value is null.
	 * @param key
	 *            The string to file the value under
	 * @param value
	 *            The value, of any type
	 * @return The previous value associated with key, or null if there was no
	 *         object.
	 * @see ConcurrentMap#put(Object, Object)
	 */
	public Object setProperty(String key, Object value) {
		if (value == null) {
			throw new NullPointerException();
		}
		return properties.put(key, value);
	}

	/**
	 * The fastest way to find out if there is a value associated with the
	 * passed key.
	 * 
	 * @param key
	 *            The key to check
	 * @return True if there is currently any value associated with the key.
	 *         Null values are not supported.
	 */
	public boolean hasProperty(String key) {
		return properties.containsKey(key);
	}

	/**
	 * Get a snapshot of an object's current properties.
	 * 
	 * @return A copy of the property key set.
	 */
	public Set<String> getProperties() {
		return new HashSet<String>(properties.keySet());
	}

	/**
	 * Get a String property with a null default value if it doesn't exist.
	 * 
	 * @param key
	 *            The key to find a value for
	 * @return The value associated with key, as a String, or null if there is
	 *         none.
	 * @see CellObject#getProperty(String, Object, Class)
	 */
	public String getProperty(String key) {
		return getProperty(key, null, String.class);
	}

	/**
	 * Null-default helper for
	 * {@link CellObject#getProperty(String, Object, Class)}
	 * 
	 * @param <T>
	 *            Return type
	 * @param key
	 *            The key to find a value for
	 * @param c
	 *            The class to cast to
	 * @return The value associated with key, or null if there is none.
	 * @see CellObject#getProperty(String, Object, Class)
	 */
	public <T> T getProperty(String key, Class<? extends T> c) {
		return getProperty(key, null, c);
	}

	/**
	 * Get the value associated with key, casting it to a specific type.
	 * 
	 * If the value is not found, the default is returned. If the value is
	 * found, it casts it to c if it is an instance of c. If that fails and
	 * value is a string, it tries to cast it to one of the supported types
	 * (Boolean, Integer, Float, Double) if c is one of these types. If all of
	 * this fails or there is a parsing error converting it to c, then the
	 * default is returned.
	 * 
	 * @param <T>
	 *            Return type
	 * @param key
	 *            The key to find a value for
	 * @param def
	 *            The default value to return in case there is no value
	 *            associated with the key
	 * @param c
	 *            The class to cast to
	 * @return The value associated with key, or the value passed in def
	 * @see CellObject#getProperty(String, Class)
	 */
	public <T> T getProperty(String key, T def, Class<? extends T> c) {
		return convert(key, properties.get(key), def, c);
	}

	/**
	 * Removes any property entry associated with key.
	 * 
	 * @param key
	 *            The key to remove
	 * @return true if something was removed
	 */
	public boolean removeProperty(String key) {
		return properties.remove(key) != null;
	}

	/**
	 * Same as {@link CellObject#getProperty(String, Class)} except that the
	 * retrieved property is removed from the property map.
	 * 
	 * @param <T>
	 *            Return type
	 * @param key
	 *            The key to remove
	 * @param c
	 *            The class to cast the value to
	 * @return null if there is no property to remove, or the property value
	 *         that was removed
	 * @see CellObject#getProperty(String, Class)
	 */
	public <T> T removeProperty(String key, Class<? extends T> c) {
		return convert(key, properties.remove(key), null, c);
	}

	/**
	 * Converts a key value pair to the passed class type, or to a String and
	 * then one of the convertible types (Boolean, Double, Float, Integer).
	 * Replaces the old key/value pair with the new type for value. Returns a
	 * default if there is a failure.
	 * 
	 * @param <T>
	 *            The return type
	 * @param key
	 *            The key, used to replace the old value type with the new value
	 *            type
	 * @param value
	 *            The value object
	 * @param def
	 *            A default to return if there is failure.
	 * @param c
	 *            The class to try and convert it to.
	 * @return Default if it is unable to cast it to c, or to string and then
	 *         parsed to c.
	 * @see CellObject#getProperty(String, Class)
	 * @see CellObject#getProperty(String, Object, Class)
	 * @see CellObject#removeProperty(String, Class)
	 */
	private <T> T convert(String key, Object value, T def, Class<? extends T> c) {
		if (value == null) {
			return def;
		}
		if (value.getClass().equals(c)) {
			return c.cast(value);
		}
		if (c.equals(String.class)) {
			return c.cast(value.toString());
		}
		if (value instanceof String) {
			String str = (String) value;
			if (c.equals(Boolean.class)) {
				Boolean v = Boolean.valueOf(str);
				properties.replace(key, value, v);
				return c.cast(v);
			}
			try {
				if (c.equals(Integer.class)) {
					Integer v = Integer.valueOf(str);
					properties.replace(key, value, v);
					return c.cast(v);
				}
				if (c.equals(Double.class)) {
					Double v = Double.valueOf(str);
					properties.replace(key, value, v);
					return c.cast(v);
				}
				if (c.equals(Float.class)) {
					Float v = Float.valueOf(str);
					properties.replace(key, value, v);
					return c.cast(v);
				}
			} catch (NumberFormatException e) {
				e.printStackTrace();
			}
		}
		logger.warn("Unable to convert property " + key + " => " + value
				+ " to " + c.getName() + " (it is a "
				+ value.getClass().getName() + ")");
		return def;
	}

	/**
	 * When a executed on an object, all of the properties who's keys start with
	 * {@link CellObject#APPLY_PROPERTIES} get that prefix removed, becoming
	 * "top-level" properties. This method performs this conversion, removing
	 * the old key-value mappings and adding the new mappings.
	 * 
	 * @see CellObject#APPLY_PROPERTIES
	 */
	public void doApplyProperties() {
		// move apply properties to regular properties
		for (Entry<String, Object> entry : properties.entrySet()) {
			if (entry.getKey().length() > APPLY_PROPERTIES.length()) {
				int pos = entry.getKey().indexOf(APPLY_PROPERTIES);
				if (pos == 0) {
					properties.put(
							entry.getKey().substring(APPLY_PROPERTIES.length(),
									entry.getKey().length()), entry.getValue());
					properties.remove(entry.getKey(), entry.getValue());
				}
			}
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();
		builder.append("(");
		for (Entry<String, Object> entry : properties.entrySet()) {
			builder.append(entry.getKey());
			builder.append(" => ");
			builder.append(entry.getValue().toString());
			builder.append(", ");
		}
		builder.deleteCharAt(builder.length() - 1);
		builder.append(")");
		return builder.toString();
	}

	public int compareTo(CellObject o) {
		return (int) (this.serial - o.serial);
	}
}
