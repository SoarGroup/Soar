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

import java.util.*;

/**
 * 
 * 
 * @author Tim Jasko (tj9582@yahoo.com)
 */
public class DatamapInputFactory implements IElementFactory {
	
	public static final String ID = "edu.rosehulman.soar.datamap.editor.DatamapInputFactory";


	public IAdaptable createElement(IMemento memento) {
		
		ArrayList path = new ArrayList();
		int size = memento.getInteger("pathSize").intValue();
		
		for (int i=0; i<size; ++i) {
			String key = new Integer(i).toString();
			
			path.add(memento.getString(key));
		}
		
		String filePath =  memento.getString("file");
		
		IFile file = SoarPlugin.getWorkspace().getRoot().getFile(new Path(filePath));
		
		String opName = memento.getString("opName");
		

		return new DatamapEditorInput(file, path, opName);
	}

}
