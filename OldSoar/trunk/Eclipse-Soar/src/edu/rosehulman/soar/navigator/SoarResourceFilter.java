/**
 *
 * @file SoarResourceFilter.java
 * @date Mar 22, 2004
 */
package edu.rosehulman.soar.navigator;

import edu.rosehulman.soar.natures.*;
import edu.rosehulman.soar.sourcing.*;

import org.eclipse.ui.views.navigator.ResourcePatternFilter;
import org.eclipse.jface.viewers.*;
import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.*;

/**
 * Hides things from the SoarNavigator.
 *  "Things" includes non-Soar projects, sourcing files, and the datamap file.
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class SoarResourceFilter extends ResourcePatternFilter {
	public boolean select(Viewer viewer, Object parentElement, Object element) {
		
		
		
		//If this is not a Soar project, don't display it
		if (element instanceof IProject) {
			IProject proj = (IProject) element;
			
			try {
				if (proj.hasNature(SoarProjectNature.NATURE_ID)) {
					return true;
				} else {
					return false;
				}
			} catch (CoreException e) {
				e.printStackTrace();
			}
			
		//Hide certain files as well
		} else if (element instanceof IFile) {
			IFile file = (IFile) element;
			String sourcingName = SourcingFile.getSourcingFileName(file.getParent());
			
			String fileName = file.getName();
			
			if (fileName.equals(".project")) {
				return false;
			} else if (fileName.equals(sourcingName)) {
				return false;
			} else if (fileName.endsWith(".xdm")) {
				return false;
			}
		}
		
		return true;	
	}
}
