/*
 * Created on Oct 26, 2003
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.editor.menus;


import java.io.*;
import java.util.Calendar;
import java.util.TimeZone;

import org.eclipse.core.runtime.*;
import org.eclipse.core.resources.*;

import edu.rosehulman.soar.SoarPlugin;

/**
 * Provides handy shared functions for the Soar source editor menu items.
 * 
 * @author Tim Jasko
 *
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
	 * 
	 * @param file The file to be initialized.
	 * @param templateFile The name of the template file.
	 * @return A Stream containing the contents to be placed in the file.
	 */
	public static String getFileTemplate(IFile file, String templateFile) {
		
		Calendar cal = Calendar.getInstance(TimeZone.getDefault());
		String DATE_FORMAT = "yyyy-MM-dd HH:mm:ss";
		java.text.SimpleDateFormat sdf =
			new java.text.SimpleDateFormat(DATE_FORMAT);
		sdf.setTimeZone(TimeZone.getDefault());
		
		String temp = "";
		String filePath = file.getFullPath().toString();
		String fileName = file.getName();
		String fileExtensionless = fileName.split("\\.")[0];
		String projectName = file.getProject().getName();
		
		// Get the path to our template file
		try {
			filePath = Platform.resolve(
			  SoarPlugin.getDefault().getDescriptor().getInstallURL()).getFile()
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
			
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			System.out.println(filePath + " not found");
			
		} catch  (IOException e) {
			e.printStackTrace();
			System.out.println("Error opening " + filePath);
			
		} // catch
		
		return temp;
	} // InputStream getFileTemplate(String fileName, String projectName)


} // class Utility
