/**
 *
 * @file DatamapInputFactory.java
 * @date Jun 14, 2004
 */
package edu.rosehulman.soar.datamap.editor;

import edu.rosehulman.soar.*;

import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.*;
import org.eclipse.ui.*;



/**
 * 
 * 
 * @author Tim Jasko (tj9582@yahoo.com)
 */
public class DatamapInputFactory implements IElementFactory {
	
	public static final String ID = "edu.rosehulman.soar.datamap.editor.DatamapInputFactory";


	public IAdaptable createElement(IMemento memento) {
		
		Integer temp = memento.getInteger("rootID");
		int rootID = 0;
		if (temp != null) {
			rootID = temp.intValue();
		}
		
		String filePath = memento.getString("file");
		
		IFile file = SoarPlugin.getWorkspace().getRoot().getFile(new Path(filePath));
		

		return new DatamapEditorInput(file, rootID);
	}

}
