/**
 *
 * @file SoarChangeListener.java
 * @date Feb 29, 2004
 */
package edu.rosehulman.soar.sourcing;

import edu.rosehulman.soar.*;
import edu.rosehulman.soar.natures.*;

import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.CoreException;

/**
 * Keeps track of changes to the soar project,
 *  allowing us to rebuild the sourcing file when necessary.
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class SoarChangeListener implements IResourceChangeListener {
	public void resourceChanged(IResourceChangeEvent event) {
		if (event.getType() == IResourceChangeEvent.PRE_BUILD) {
			try {
				
				event.getDelta().accept(new SoarDeltaVisitor());
				
			} catch (CoreException e) {
				//e.printStackTrace();
			}
		} // if
	} // void resourceChanged( ... )
	
	public class SoarDeltaVisitor implements IResourceDeltaVisitor {
		public boolean visit(IResourceDelta delta) {
			
			
			switch (delta.getKind()) {
				
			case IResourceDelta.REMOVED:
				// Deregister the datamap if we delete a project
				// Otherwise, if we create a new project during this 
				//  session with the same name, it will show up as having
				//  this project's datamap.
				IResource dRes = delta.getResource();
				
				if (dRes.equals(dRes.getProject())) {
					SoarPlugin.removeDataMap(dRes);
				}
			
				//fall through
			case IResourceDelta.MOVED_TO:
			case IResourceDelta.MOVED_FROM:
			case IResourceDelta.ADDED:
			
				IResource res = delta.getResource();
				IProject proj = res.getProject();
				try {
					if (proj != null &&
						proj.hasNature(SoarProjectNature.NATURE_ID)) {
						
						//SourcingFile.createSourcingFile(proj);
						SourcingFile.createSourcingFile(res.getParent(), null);
						//proj.build(IncrementalProjectBuilder.INCREMENTAL_BUILD, null);
					} // if
				} catch (Exception e) {
					//e.printStackTrace();
				} // catch
				
				break;
			} // switch
			
			return true; // visit the children
			
		} // boolean visit( ... )
	} // class
} // class