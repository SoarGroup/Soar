/**
 *
 * @file SoarChangeListener.java
 * @date Feb 29, 2004
 */
package edu.rosehulman.soar.sourcing;

import edu.rosehulman.soar.natures.*;

import org.eclipse.core.resources.*;

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
				//e.printStackTrace();
			}
		} // if
	} // void resourceChanged( ... )
	
	public class SoarDeltaVisitor implements IResourceDeltaVisitor {
		public boolean visit(IResourceDelta delta) {
			
			
			switch (delta.getKind()) {
				
				case IResourceDelta.MOVED_TO:
				case IResourceDelta.MOVED_FROM:
				case IResourceDelta.ADDED:
				case IResourceDelta.REMOVED:
				
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