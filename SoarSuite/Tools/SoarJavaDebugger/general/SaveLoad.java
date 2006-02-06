/********************************************************************************************
*
* SaveLoad.java
* 
* Created on 	Nov 20, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package general;

import java.io.*;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;

/********************************************************************************************
* 
* Save and load file dialogs
* 
********************************************************************************************/
public class SaveLoad
{
	/********************************************************************************************
	* 
	* Shows a file dialog and returns a file name
	* 
	* @param	parent			A parent window for the file dialog
	* @param	fileExt			The file extension to load (e.g. "xml")
	* @param	fileDesc		A description for this type of file (e.g. "Editor file")
	* @param	props			The app properties to use to store the current directory preference
	* @param	fileSaveName	The name of the property to use to store directory to use for saving files (e.g. "SaveFile")
	* @param	fileLoadName	The name of the property to use to store directory to use for loading files (e.g. "LoadFile")
	*
	* @return	The name of the file to use (or null if user canceled)
	* 
	********************************************************************************************/
	public static String LoadFileDialog(Composite parent, String[] fileExts, String[] fileDescriptions, AppProperties props, String fileSaveName, String fileLoadName)
	{
		String currentDirSave = fileSaveName + ".CurrentDir" ;
		String currentDirLoad = fileLoadName + ".CurrentDir" ;
		
		// Retrieve the initial directory to use.
		String initialDir = props.getProperty(currentDirLoad) ;
	
		// If there's no FileLoad entry, try the FileSave entry
		if (initialDir == null)
			initialDir = props.getProperty(currentDirSave) ;
		
	    FileDialog chooser = new FileDialog(parent.getShell(), SWT.OPEN) ;

		// Specify the file extensions we want
	    chooser.setFilterNames(fileDescriptions) ;
	    chooser.setFilterExtensions(fileExts) ;
	    
	    // Open up on the same folder we last used, if possible.
	    if (initialDir != null)
		    chooser.setFilterPath(initialDir) ;
		    
		// Ask the user to select a file to open.
	    String returnVal = chooser.open() ;
	    
	    // Check for cancel
	    if (returnVal == null)
	    	return null ;
	    
	    // Store the directory that the user chose
	    File currentDir = new File(returnVal) ;
	    props.setProperty(currentDirLoad, currentDir.getParent()) ;
	    	    
	    return returnVal ;
	}
		
	/********************************************************************************************
	* 
	* Shows a file dialog and returns a file name
	* 
	* @param	parent			A parent window for the file dialog
	* @param	fileExt			The file extension to load (e.g. "xml")
	* @param	fileDesc		A description for this type of file (e.g. "Editor file")
	* @param	props			The app properties to use to store the current directory preference
	* @param	fileSaveName	The name of the property to use to store directory to use for saving files (e.g. "SaveFile")
	* @param	fileLoadName	The name of the property to use to store directory to use for loading files (e.g. "LoadFile")
	*
	* @return	The name of the file to use (or null if user canceled)
	* 
	********************************************************************************************/
	public static String SaveFileDialog(Composite parent, String[] fileExts, String[] fileDescriptions, AppProperties props, String fileSaveName, String fileLoadName)
	{
		String currentDirSave = fileSaveName + ".CurrentDir" ;
		String currentDirLoad = fileLoadName + ".CurrentDir" ;

		// Retrieve the initial directory to use.
		String initialDir = props.getProperty(currentDirSave) ;
	
		// If there's no FileSave entry, try the FileLoad entry
		if (initialDir == null)
			initialDir = props.getProperty(currentDirLoad) ;
	
	    FileDialog chooser = new FileDialog(parent.getShell(), SWT.SAVE) ;

		// Specify the file extensions we want
	    chooser.setFilterNames(fileDescriptions) ;
	    chooser.setFilterExtensions(fileExts) ;
	    
	    // Open up on the same folder we last used, if possible.
	    if (initialDir != null)
		    chooser.setFilterPath(initialDir) ;
		    
		// Ask the user to select a file to open.
	    String filePath = chooser.open() ;
	    
	    // Check for cancel
	    if (filePath == null)
	    	return null ;
	    	    
	    // Store the directory that the user chose
	    File currentDir = new File(filePath) ;
	    props.setProperty(currentDirSave, currentDir.getParent()) ;
	       	
    	// Find out if the user entered an extension
    	// If not, we will use a default one.
    	if (FileExt.GetLowerFileExtension(filePath) == "")
    	{
    		// Don't think with SWT we can get the user's choice in the extensions
    		// so just take the first
	    	String ext = FileExt.GetLowerFileExtension(fileExts[0]) ;
	    	
	    	// Change/add the file extension as necessary
	    	filePath = FileExt.SetFileExtension(filePath, ext) ;
    	}

    	// Check if the file already exists and if so prompt.
    	// We could fix this so it appears while the dialog is still visible
    	// but only be writing a facade for FileDialog() and replacing the logic for "open".
    	// This is fine for now.
    	File file = new File(filePath) ;
    	if (file.exists())
    	{
    		MessageBox msg = new MessageBox(parent.getShell(), SWT.ICON_WARNING | SWT.OK | SWT.CANCEL);
    		msg.setText("Save As") ;
    		msg.setMessage(filePath + " already exists.\nDo you want to replace it?");
    		int choice = msg.open();
    		
    		if (choice == SWT.CANCEL)
    			return null ;
    	}

	    return filePath ;
	}
	
	public static String SelectFolderDialog(Composite parent, String initialFolder, String title, String message)
	{
		DirectoryDialog dialog = new DirectoryDialog(parent.getShell()) ;
		
		if (title != null)
			dialog.setText(title) ;
		if (message != null)
			dialog.setMessage(message) ;
		if (initialFolder != null)
			dialog.setFilterPath(initialFolder) ;
		
		String path = dialog.open() ;
		return path ;
	}
}
