package soar2d.world;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;

import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;

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
	 * @param name template object name
	 * @return true if it exists
	 */
	boolean hasTemplate(String name) {
		return templates.containsKey(name);
	}
	
	/**
	 * expunges all templates
	 */
	void removeAllTemplates() {
		templates.clear();
	}
	
	/**
	 * @param name the template to remove
	 * @return true if removed (false if it didn't exist)
	 */
	boolean removeTemplate(String name) {
		CellObject object = templates.remove(name);
		return object != null;
	}
	
	/**
	 * @param cellObject the new template
	 * @return true if added to list
	 * 
	 * Register a new prototype
	 */
	boolean registerTemplate(CellObject cellObject) {
		if (templates.containsKey(cellObject.getName())) {
			return false;
		}
		if (cellObject == null) {
			return false;
		}
		templates.put(cellObject.getName(), cellObject);
		return true;
	}
	
	public Collection<CellObject> getTemplates() {
		return templates.values();
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
	boolean hasTemplatesWithProperty(String name) {
		ArrayList<CellObject> all = getTemplatesWithProperty(name);
		if (all.size() <= 0) {
			return false;
		}
		return true;
	}
	
	ArrayList<CellObject> rewardObjects = new ArrayList<CellObject>();
	/**
	 * @param name object name to create
	 * @return the new object
	 * 
	 * Clones an object and returns the new copy
	 */
	CellObject createObject(String name) {
		if (templates.containsKey(name)) {
			CellObject newObject = new CellObject(templates.get(name));
			if (newObject.rewardApply > 0) {
				// assign identification number
				newObject.addProperty(Names.kPropertyBoxID, Integer.toString(rewardObjects.size() + 1));

				// keep track of reward objects
				rewardObjects.add(newObject);
				
			} else if (newObject.rewardInfoApply) {
				// assign identification properties
				newObject.addProperty(Names.kPropertyColor, Soar2D.simulation.kColors[0]);
				newObject.addProperty(Names.kPropertyBoxID, "0");
			}
			return newObject;
		}
		return null;
	}
	
	/**
	 * @param name the property
	 * @return an object with that property
	 * 
	 * creates an object that has the specified property. Many different templates
	 * could possibly have the same property, this randomly picks one to clone.
	 */
	CellObject createRandomObjectWithProperty(String name) {
		ArrayList<CellObject> all = getTemplatesWithProperty(name);
		if (all.size() <= 0) {
			return null;
		}
		return new CellObject(all.get(Simulation.random.nextInt(all.size())));
	}
	/**
	 * @param name1 a property
	 * @param name1 another property
	 * @return an object with those properties
	 * 
	 * same as createRandomObjectWithProperty but with two properties
	 */
	CellObject createRandomObjectWithProperties(String name1, String name2) {
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
		return new CellObject(all.get(Simulation.random.nextInt(all.size())));
	}
}
