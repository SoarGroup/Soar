/**
 *
 * @file DatamapInputPersistable.java
 * @date Jun 14, 2004
 */
package edu.rosehulman.soar.datamap.editor;

import org.eclipse.ui.IMemento;
import org.eclipse.ui.IPersistableElement;


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
		
		memento.putInteger("rootID", _dei.getRootID());

		memento.putString("file", _dei.getFile().getFullPath().toString());

	}

}
