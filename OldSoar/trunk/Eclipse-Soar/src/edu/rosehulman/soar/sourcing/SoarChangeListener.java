/**
 *
 * @file SoarChangeListener.java
 * @date Feb 29, 2004
 */
package edu.rosehulman.soar.sourcing;

import edu.rosehulman.soar.natures.*;

import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.*;

/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class SoarChangeListener implements IResourceChangeListener {
	public void resourceChanged(IResourceChangeEvent event) {
		if (event.getType() == IResourceChangeEvent.PRE_AUTO_BUILD) {
			try {
				
				event.getDelta().accept(new SoarDeltaVisitor());
				
			} catch (Exception e) {
				e.printStackTrace();
			}
		} // if
	} // void resourceChanged( ... )
	
	public class SoarDeltaVisitor implements IResourceDeltaVisitor {
		public boolean visit(IResourceDelta delta) {
			
			
			switch (delta.getKind()) {
				case IResourceDelta.ADDED:
				case IResourceDelta.REMOVED:
				
					IProject proj = delta.getResource().getProject();
					try {
						if (proj != null &&
							proj.hasNature(SoarProjectNature.NATURE_ID)) {
							
							//SourcingFile.createSourcingFile(proj);
							proj.build(IncrementalProjectBuilder.INCREMENTAL_BUILD, null);
						} // if
					} catch (Exception e) {
						e.printStackTrace();
					} // catch
					
					break;
			} // switch
			
			return true; // visit the children
			
		} // boolean visit( ... )
	} // class
} // class