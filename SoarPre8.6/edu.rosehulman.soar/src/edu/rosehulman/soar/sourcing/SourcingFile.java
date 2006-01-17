/**
 *
 * @file SourcingFile.java
 * @date Feb 26, 2004
 */
package edu.rosehulman.soar.sourcing;

import org.eclipse.core.runtime.*;
import org.eclipse.core.resources.*;

import java.io.*;
import java.util.*;

/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class SourcingFile {
	
	
	public static void createSourcingFile(IProject project, IProgressMonitor monitor) {
		
		try {
			System.out.println("Building sourcing file");
			String contents = doFolder(project, 0);
		
			IFile agentFile = project.getFile("Agent.soar");
		
			ByteArrayInputStream is = new ByteArrayInputStream(contents.getBytes());
		
			if (agentFile.exists()) {
				agentFile.setContents(is, true, false, monitor);
			} else {
				agentFile.create(is, true, monitor);
			}
		} catch (CoreException e) {
			e.printStackTrace();
		}
	
	}
	
	public static void createSourcingFile(IProject project) {
		
		try {
			System.out.println("Building sourcing file");
			String contents = doFolder(project, 0);
			
			IFile agentFile = project.getFile("Agent.soar");
			
			ByteArrayInputStream is = new ByteArrayInputStream(contents.getBytes());
			
			if (agentFile.exists()) {
				agentFile.setContents(is, true, false, null);
			} else {
				agentFile.create(is, true, null);
			}
		} catch (CoreException e) {
			e.printStackTrace();
		}
		
	}
	
	private static String doFolder(IContainer res, int depth) {
		String ret = "";
		String indent = "";
		
		//get the indenting.
		for (int i=0; i<depth; i++) {
			indent += "\t";
		} // for indent
		
		try {
			IResource kids[] = res.members();
			ArrayList files = new ArrayList();
			ArrayList folders = new ArrayList();
			
			for (int i=0; i<kids.length; i++) {
				IResource kid = kids[i];
				
				
				//Split this list into files and folders, for readability's sake 
				if (kid instanceof IFile) {
					
					if (kid.getFileExtension().equals("soar")
						&& ! kid.getName().equals("Agent.soar")) {
						
						files.add(kid);
					}
					
				} else if (kid instanceof IContainer) {
					folders.add(kid);
				} // else
			} // for members
			
			
			//Deal with the files
			for (int i=0; i<files.size(); i++) {
				IFile file = (IFile) files.get(i);
				
				ret += indent + "source " + file.getName() + "\n";
				
			} // for files
			
			for (int i=0; i<folders.size(); i++) {
				IContainer folder = (IContainer) folders.get(i);
				
				ret += indent + "pushd " + folder.getName() + "\n";
				ret += doFolder(folder, depth + 1);
				ret += indent + "popd\n";
				
			} // for folders
			
			
		} catch (CoreException e) {
			e.printStackTrace();
		}
		
		return ret;
	}

}
