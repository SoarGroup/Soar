package soar2d.world;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

public class CellObjectManager {
	private HashMap<String, CellObject> templates = new HashMap<String, CellObject>();
	private int updatableCount = 0;
	
	public boolean hasTemplate(String name) {
		return templates.containsKey(name);
	}
	
	public void removeAllTemplates() {
		templates.clear();
		updatableCount = 0;
	}
	
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
	
	public boolean registerTemplate(String name, CellObject cellObject) {
		if (templates.containsKey(name)) {
			return false;
		}
		if (cellObject == null) {
			return false;
		}
		if (cellObject.updatable) {
			++updatableCount;
			assert updatableCount > 0;
		}
		templates.put(name, cellObject);
		return true;
	}
	
	public CellObject createObject(String name) {
		if (templates.containsKey(name)) {
			return new CellObject(templates.get(name));
		}
		return null;
	}
	
	public boolean updatablesExist() {
		return updatableCount > 0;
	}
	
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
}
