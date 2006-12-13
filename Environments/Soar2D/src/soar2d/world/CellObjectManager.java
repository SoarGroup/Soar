package soar2d.world;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import soar2d.Simulation;

/**
 * @author voigtjr
 *
 * This object keeps track of the "object templates" or prototypes.
 * These objects are copied to create new objects.
 */
public class CellObjectManager {
	/**
	 * The templates, mapped by name.
	 */
	private HashMap<String, CellObject> templates = new HashMap<String, CellObject>();
	/**
	 * A count of the number of templates that are updatable.
	 * Used for optimization, if there are no updatable templates, some possibly
	 * expensive code is skipped over.
	 */
	private int updatableCount = 0;
	
	/**
	 * @param name template object name
	 * @return true if it exists
	 */
	public boolean hasTemplate(String name) {
		return templates.containsKey(name);
	}
	
	/**
	 * expunges all templates
	 */
	public void removeAllTemplates() {
		templates.clear();
		updatableCount = 0;
	}
	
	/**
	 * @param name the template to remove
	 * @return true if removed (false if it didn't exist)
	 */
	public boolean removeTemplate(String name) {
		if (templates.containsKey(name)) {
			CellObject object = templates.remove(name);
			if (object.updatable) {
				--updatableCount;
				assert updatableCount >= 0;
			}
			return true;
		}
		return false;
	}
	
	/**
	 * @param cellObject the new template
	 * @return true if added to list
	 * 
	 * Register a new prototype
	 */
	public boolean registerTemplate(CellObject cellObject) {
		if (templates.containsKey(cellObject.getName())) {
			return false;
		}
		if (cellObject == null) {
			return false;
		}
		if (cellObject.updatable) {
			++updatableCount;
			assert updatableCount > 0;
		}
		templates.put(cellObject.getName(), cellObject);
		return true;
	}
	
	/**
	 * @param name object name to create
	 * @return the new object
	 * 
	 * Clones an object and returns the new copy
	 */
	public CellObject createObject(String name) {
		if (templates.containsKey(name)) {
			return new CellObject(templates.get(name));
		}
		return null;
	}
	
	/**
	 * @return true if there is at least one object that needs to be updated.
	 */
	public boolean updatablesExist() {
		return updatableCount > 0;
	}
	
	/**
	 * @param name the property name
	 * @return list of cell object templates that have that property
	 */
	public ArrayList<CellObject> getTemplatesWithProperty(String name) {
		ArrayList<CellObject> ret = new ArrayList<CellObject>(templates.values());
		Iterator<CellObject> iter = ret.iterator();
		while (iter.hasNext()) {
			CellObject obj = iter.next();
			if (!obj.hasProperty(name)) {
				iter.remove();
			}
		}
		return ret;
	}
	
	/**
	 * @param name the property name
	 * @return true if any templates have that property
	 */
	public boolean hasTemplatesWithProperty(String name) {
		ArrayList<CellObject> all = getTemplatesWithProperty(name);
		if (all.size() <= 0) {
			return false;
		}
		return true;
	}
	
	/**
	 * @param name the property
	 * @return an object with that property
	 * 
	 * creates an object that has the specified property. Many different templates
	 * could possibly have the same property, this randomly picks one to clone.
	 */
	public CellObject createRandomObjectWithProperty(String name) {
		ArrayList<CellObject> all = getTemplatesWithProperty(name);
		if (all.size() <= 0) {
			return null;
		}
		return all.get(Simulation.random.nextInt(all.size()));
	}
	/**
	 * @param name1 a property
	 * @param name1 another property
	 * @return an object with those properties
	 * 
	 * same as createRandomObjectWithProperty but with two properties
	 */
	public CellObject createRandomObjectWithProperties(String name1, String name2) {
		ArrayList<CellObject> all = getTemplatesWithProperty(name1);
		if (all.size() <= 0) {
			return null;
		}
		Iterator<CellObject> iter = all.iterator();
		while (iter.hasNext()) {
			CellObject obj = iter.next();
			if (!obj.hasProperty(name2)) {
				iter.remove();
			}
		}
		if (all.size() <= 0) {
			return null;
		}
		return all.get(Simulation.random.nextInt(all.size()));
	}
}
