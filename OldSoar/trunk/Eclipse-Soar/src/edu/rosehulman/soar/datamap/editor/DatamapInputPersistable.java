/**
 *
 * @file DatamapInputPersistable.java
 * @date Jun 14, 2004
 */
package edu.rosehulman.soar.datamap.editor;

import org.eclipse.ui.IMemento;
import org.eclipse.ui.IPersistableElement;

import java.util.*;

/**
 * 
 * 
 * @author Tim Jasko (tj9582@yahoo.com)
 */
public class DatamapInputPersistable implements IPersistableElement {
	private DatamapEditorInput _dei;

	public DatamapInputPersistable(DatamapEditorInput dei) {
		_dei = dei;
	}

	public String getFactoryId() {
		return DatamapInputFactory.ID;
	}


	public void saveState(IMemento memento) {
		
		ArrayList path = _dei.getPath();
		
		memento.putInteger("pathSize", path.size());
		for (int i=0; i<path.size(); ++i) {
			String key = new Integer(i).toString();
			memento.putString(key, (String) path.get(i));
		}
		
		memento.putString("opName", _dei.getName());
		memento.putString("file", _dei.getFile().getFullPath().toString());

	}

}
