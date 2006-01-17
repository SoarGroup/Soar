/*
 * Created on Oct 19, 2003
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.wizards;


import edu.rosehulman.soar.*;
import edu.rosehulman.soar.datamap.*;

import java.io.*;
import java.util.Calendar;
import java.util.TimeZone;

import org.eclipse.core.runtime.*;
import org.eclipse.core.resources.*;


/**
 *
 * Loads the content from a template file and fills in the appropriate fields.
 * Also contains other miscellaneous useful functions
 * 
 * @author Tim Jasko
 */
public class Utility {
	
	

	/**
	 * Initializes file contents with a template. 
	 * The contents of the template are loaded, and then the following fields 
	 * are filled in:
	 * <p>
	 * <code>${file}</code> - The name of the file, including extension<p>
	 * <code>${name}</code> - The name of the file, minus the extension.<p>
	 * <code>${filePath}</code> - The full path of the file.<p>
	 * <code>${template}</code> - The name of the template that is being loaded from.<p>
	 * <code>${parentDir}</code> - The name of the directory the file is in.<p>
	 * <code>${date}</code> - The current date and time.<p>
	 * <code>${project}</code> - The project this file belongs to.<p>
	 * <code>${createdComment}</code> - A Soar comment telling the name of the file and when it was created.<p>
	 * 
	 * @param file The file to be initialized.
	 * @param templateFile The name of the template file.
	 * @return A Stream containing the contents to be placed in the file.
	 */
	
	public static InputStream getFileTemplate(IFile file, String templateFile) {
		Calendar cal = Calendar.getInstance(TimeZone.getDefault());
    
		String DATE_FORMAT = "yyyy-MM-dd HH:mm:ss";
		java.text.SimpleDateFormat sdf =
			new java.text.SimpleDateFormat(DATE_FORMAT);
		sdf.setTimeZone(TimeZone.getDefault());
		
		String filePath = file.getFullPath().toString();
		String fileName = file.getName();
		String fileExtensionless = fileName.split("\\.")[0];
		String projectName = file.getProject().getName();
		
		String createdComment = "# " + fileName + " created on " + sdf.format(cal.getTime()); 
		String temp;
		
		// Get the path to our template file
		try {
			
			filePath = Platform.resolve(
			  //SoarPlugin.getDefault().getDescriptor().getInstallURL()
					Platform.getBundle("edu.rosehulman.soar").getEntry("/")
			  ).getFile()
			  + "templates/" + templateFile;
		} catch (IOException e) {
			filePath = "";
		} // catch
		
		// Fill in the variable fields with the appropriate data 
		try {
			InputStream template = new FileInputStream(filePath);
			
			byte fileContents[] = new byte[template.available()];
			
			template.read(fileContents, 0, template.available());
			
			temp = new String(fileContents);
			
			temp = temp.replaceAll("\\$\\{file\\}", fileName);
			temp = temp.replaceAll("\\$\\{name\\}", fileExtensionless);
			temp = temp.replaceAll("\\$\\{filePath\\}", filePath);
			temp = temp.replaceAll("\\$\\{template\\}", templateFile);
			temp = temp.replaceAll("\\$\\{parentDir\\}",
			  file.getParent().getName().toString());
			temp = temp.replaceAll("\\$\\{date\\}", sdf.format(cal.getTime()));
			temp = temp.replaceAll("\\$\\{project\\}", projectName);

			temp = temp.replaceAll("\\$\\{createdComment\\}", createdComment);
			
			
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			System.out.println(filePath + " not found");
			temp = createdComment;
		} catch  (IOException e) {
			e.printStackTrace();
			System.out.println("Error opening " + filePath);
			temp = createdComment;
		} // catch
		
		return new ByteArrayInputStream(temp.getBytes());
	} // InputStream getFileTemplate(String fileName, String projectName)
	
	
	
	
	
	public static void associateID(IResource res, int id) {
		try {
			res.setPersistentProperty(DataMap.VERTEX_ID, new Integer(id).toString());
		} catch (CoreException e) {
			e.printStackTrace();
		} // catch
	}
	
	public static int getAssociatedID(IResource res) {
		try {
			return new Integer(res.getPersistentProperty(DataMap.VERTEX_ID)).intValue();
		} catch (CoreException e) {
			e.printStackTrace();
			return 0;
		} catch (NumberFormatException e) {
			return 0;
		}
	}
	
	
	/**
	 * Determines if this is the soar file that represents the folder, ie:
	 *  the file that would be opened if we double-clicked the folder from
	 *  Visual Soar's navigator. Basically, it sees if the resource's
	 *  name is the same as that of its parent folder. My apologies
	 *  to everyone for not coming up with a better name.
	 *  
	 * @param res The resource to check
	 * @return <code>true</code> if it represents the folder, <code>false</code> if not.
	 */
	public static boolean isFolderFile(IResource res) {
		if (res instanceof IProject) {
			return false;
		}
		if (getFileFolder(res).exists()) {
			return true;
		}
		
		return false;
	} // boolean isFolderFile(IResource file)
	
	
	/**
	 * Gets the folder associated with this file (note: may not exist). 
	 * @param res the file to check on.
	 * @return The folder associated with it.
	 */
	public static IContainer getFileFolder(IResource res) {
		IContainer parent = res.getParent();
		String extensionless = removeExtension(res);
		return parent.getFolder(new Path("/" + extensionless));
	}
	
	
	/**
	 * Returns the name of this resource, minus the extension.
	 * @param res The resource.
	 * @return The name, minus the extension.
	 */
	public static String removeExtension(IResource res) {
		return res.getName().split("\\.")[0];
	}

	
	/**
	 * Prepares a Soar resource for having a[n] {suboperator|file|impasse} added
	 *  to it. If the resource is a file not inside a folder of the same name,
	 *  it creates that folder, moves this file there, creates a datamap, etc.
	 *  The value returned is the folder in which the new file should be placed.
	 *   
	 * @param res The resources to folderify.
	 * @param monitor A progress monitor.
	 * @return The folder in which to place the new file.
	 */
	public static IContainer folderify(IResource res, IProgressMonitor monitor) {
		String extensionless = res.getName().split("\\.")[0];
		
		try {
			//Does this file stand in for a folder? If so, we just need to
			// to make the new file in this same folder
			if (Utility.isFolderFile(res)) {
				return res.getParent().getFolder(new Path(extensionless));
				
			//Well poop, looks like it didn't. Oh well. Now we have to 
			// do just a tad bit more.
			} else {  
				if (! (res instanceof IContainer)) {
					String folderPath = res.getParent().getFullPath().toString()
						+ "/" + res.getName().split("\\.")[0];
					//System.out.println(folderPath);
					IFolder newFolder = ResourcesPlugin.getWorkspace().getRoot()
						.getFolder(new Path(folderPath));
					
					
					if (! newFolder.exists()) { //it probably doesn't
		
						newFolder.create(true, true, monitor);
						
						FileMarker.markResource(newFolder, FileMarker.getSoarType(res));
						associateID(newFolder, getAssociatedID(res));
						
					} // if
					
					
					/*String movePath = newFolder.getFullPath().toString()
						+ "/" + res.getName(); 
					res.move(new Path(movePath), true, monitor);*/
					
					
					///We need to add an elaborations file to the folder
					final IFile elabFile = newFolder.getFile(new Path("elaborations.soar"));
		
					try {
						InputStream stream =
						  Utility.getFileTemplate(elabFile, "elaborations.soar");
					  
						if (elabFile.exists()) {
							
						} else {
							elabFile.create(stream, true, monitor);
						} // else
					
						FileMarker.markResource (elabFile, "file");
					
						stream.close();
					} catch (Exception e) {
						e.printStackTrace();
					} // catch
					
					return newFolder;
				} else {
				
					return (IContainer) res;
					
				} // else
			} // else
		} catch (Exception e) {
			e.printStackTrace();
			return (IContainer) res;
		} // catch
		
		
	} // static IContainer folderify(IResource res, IProgressMonitor monitor)
	
	
	/**
	 * Finds an available filename to go in whatever folder this resource will be
	 *  in once we <code>folderify</code> it. The file's name is based off of 
	 *  <code>nameSuggestion</code>, with a number added to make it unique, 
	 *  as well as a .soar extension.
	 * 
	 * @param res The resource we will be adding to.
	 * @param nameSuggestion A suggestion for the file name.
	 * @return A unique file name.
	 */
	public static String getAvailableName(IResource res, String nameSuggestion) {
		String fileName = nameSuggestion + ".soar";
		
		try {
	
			if (res instanceof IContainer) {
		
				IContainer ict = (IContainer) res;
		
				IFile file;
				int fileNum = 1;
				
				fileName = nameSuggestion + ".soar";
		
				file = ict.getFile(new Path(fileName));
				if (file.exists()) {
					do {
						fileName = nameSuggestion + fileNum + ".soar";
			
						file = ict.getFile(new Path(fileName));
				
						fileNum++;
					} while (file.exists() && fileNum <= 99);
				} // if
		
			} else if (Utility.isFolderFile(res)) {
				IContainer ict = Utility.getFileFolder(res);
		
				IFile file;
				int fileNum = 1;
				
				fileName = nameSuggestion + ".soar";
		
				file = ict.getFile(new Path(fileName));
				
				if (file.exists()) {
					do {
						fileName = nameSuggestion + fileNum + ".soar";
			
						file = ict.getFile(new Path(fileName));
				
						fileNum++;
					} while (file.exists() && fileNum <= 99);
				} // if
			} else {
				fileName = nameSuggestion + ".soar";
			} // else
	
		} catch (NullPointerException e) {
			e.printStackTrace();
		} // catch
		
		return fileName;
	} // static String getAvailableName(IResource res, String nameSuggestion)


} // class Utility
