package soar2d.world;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

public class CellObjectManager {
	private HashMap<String, CellObject> templates = new HashMap<String, CellObject>();
	public boolean hasTemplate(String name) {
		return templates.containsKey(name);
	}
	public void removeAllTemplates() {
		templates.clear();
	}
	public boolean removeTemplate(String name) {
		if (templates.containsKey(name)) {
			templates.remove(name);
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
		templates.put(name, cellObject);
		return true;
	}
	public CellObject createObject(String name) {
		if (templates.containsKey(name)) {
			return new CellObject(templates.get(name));
		}
		return null;
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
