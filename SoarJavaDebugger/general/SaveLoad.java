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

import general.* ;
import javax.swing.*;
import java.io.*;
import java.awt.*;

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
	public static String LoadFileDialog(Component parent, String fileExt, String fileDescription, AppProperties props, String fileSaveName, String fileLoadName)
	{
		String currentDirSave = fileSaveName + ".CurrentDir" ;
		String currentDirLoad = fileLoadName + ".CurrentDir" ;
		
		// Retrieve the initial directory to use.
		String initialDir = props.getProperty(currentDirLoad) ;
	
		// If there's no FileLoad entry, try the FileSave entry
		if (initialDir == null)
			initialDir = props.getProperty(currentDirSave) ;
			
	    JFileChooser chooser = new JFileChooser();

		// Specify the file extensions we want
	    GeneralFileFilter filter = new GeneralFileFilter(fileExt, fileDescription);
	    chooser.setFileFilter(filter);
	    
	    // Open up on the same folder we last used, if possible.
	    if (initialDir != null)
		    chooser.setCurrentDirectory(new File(initialDir)) ;
		    
		// Ask the user to select a file to open.
	    int returnVal = chooser.showOpenDialog(parent);
	    
	    // Store the current directory for future use.
	    // Do this even if the user canceled--they will probably quickly return here again.
	    props.setProperty(currentDirLoad,chooser.getCurrentDirectory().getAbsolutePath()) ;
	    
	    if(returnVal == JFileChooser.APPROVE_OPTION)
	    {
	    	// Get the user's choice of file.
	    	String filePath = chooser.getSelectedFile().getAbsolutePath() ;
			
			return filePath ;
	    }
	    
	    return null ;
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
	public static String SaveFileDialog(Component parent, String fileExt, String fileDescription, AppProperties props, String fileSaveName, String fileLoadName)
	{
		String currentDirSave = fileSaveName + ".CurrentDir" ;
		String currentDirLoad = fileLoadName + ".CurrentDir" ;

		// Retrieve the initial directory to use.
		String initialDir = props.getProperty(currentDirSave) ;
	
		// If there's no FileSave entry, try the FileLoad entry
		if (initialDir == null)
			initialDir = props.getProperty(currentDirLoad) ;
	
	    JFileChooser chooser = new JFileChooser();

		// Specify the file extensions we want
	    GeneralFileFilter filter = new GeneralFileFilter(fileExt, fileDescription);
	    chooser.setFileFilter(filter);
	    
	    // Open up on the same folder we last used, if possible.
	    if (initialDir != null)
		    chooser.setCurrentDirectory(new File(initialDir)) ;
		    
		// Ask the user to select a file to save.
	    int returnVal = chooser.showSaveDialog(parent);
	    
	    // Store the current directory for future use.
	    // Do this even if the user canceled--they will probably quickly return here again.
	    props.setProperty(currentDirSave,chooser.getCurrentDirectory().getAbsolutePath()) ;
	    
	    if(returnVal == JFileChooser.APPROVE_OPTION)
	    {
	    	// Get the user's choice of file.
	    	String filePath = chooser.getSelectedFile().getAbsolutePath() ;
	    	
	    	// Find out if the user entered an extension
	    	// If not, we will use a default one.
	    	if (FileExt.GetLowerFileExtension(filePath) == "")
	    	{
		    	// Get the user's preferred extension
		    	String ext = null ;
		    	javax.swing.filechooser.FileFilter selectedFilter = chooser.getFileFilter() ;
		    	if (selectedFilter instanceof GeneralFileFilter)
		    	{
		    		ext = ((GeneralFileFilter)selectedFilter).getPrimaryExtension() ;
		    	}
		    	
		    	// Provide a default if the user chose "*.*"
		    	if (ext == null)
		    		ext = fileExt ;
		    	
		    	// Change/add the file extension as necessary
		    	filePath = FileExt.SetFileExtension(filePath, ext) ;
	    	}
	    	
	    	return filePath ;
	    }
	    
	    return null ;
	}
}
