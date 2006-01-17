/*
 * Created on Oct 26, 2003
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.natures;

import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.CoreException;

/**
 * Class to handle the Soar Project Nature. 
 *  Currently adds the builder for the sourcing file to the build manifold.
 *  That's it for now.
 * 
 * @author Tim Jasko
 *
 */

public class SoarProjectNature implements IProjectNature {

	static public String NATURE_ID = "edu.rosehulman.soar.natures.SoarProjectNature"; 
	private IProject project;

	public void configure() throws CoreException {
		// Add nature-specific information
		// for the project, such as adding a builder
		// to a project's build spec.
		
		//Stolen (almost) verbatim from example code
		//http://help.eclipse.org/help20/help.jsp - like it, love it, need it
		String builderID = "edu.rosehulman.soar.sourcing.SourcingBuilder";
		IProjectDescription desc = project.getDescription();
		ICommand[] commands = desc.getBuildSpec();
		boolean found = false;

		for (int i = 0; i < commands.length; ++i) {
			if (commands[i].getBuilderName().equals(builderID)) {
				found = true;
				break;
			}
		}
		if (!found) { 
			//add builder to project
			ICommand command = desc.newCommand();
			command.setBuilderName(builderID);
			ICommand[] newCommands = new ICommand[commands.length + 1];

			// Add it before other builders.
			System.arraycopy(commands, 0, newCommands, 1, commands.length);
			newCommands[0] = command;
			desc.setBuildSpec(newCommands);
			project.setDescription(desc, null);
		}
		
	} // void configure()
	
	public void deconfigure() throws CoreException {
		// Remove the nature-specific information here.
	} // void deconfigure()
	
	
	public IProject getProject() {
		return project;
	} // IProject getProject()
	
	public void setProject(IProject value) {
		project = value;
	} // void setProject(IProject value)


} // class SoarProjectNature
