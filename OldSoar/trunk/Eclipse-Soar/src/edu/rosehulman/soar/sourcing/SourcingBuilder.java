/**
 *
 * @file SorucingBuilder.java
 * @date Feb 28, 2004
 */
package edu.rosehulman.soar.sourcing;

import java.util.Map;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IncrementalProjectBuilder;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;

/**
 * Generates a sourcing file for the project.
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class SourcingBuilder extends IncrementalProjectBuilder {

	protected IProject[] build(int kind, Map args, IProgressMonitor monitor)
		throws CoreException {
		
		if (monitor != null) {
			monitor.beginTask("Building sourcing file..", 1);
		}
		
		
		IProject proj = getProject();
		SourcingFile.createSourcingFile(proj, monitor);
		
		if (monitor != null) {
			monitor.worked(1);
			monitor.done();
		}
		
		return null;
	}

}
