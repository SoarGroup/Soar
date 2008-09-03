/********************************************************************************************
*
* FileExt.java
* 
* Created on 	Nov 20, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package general;

/********************************************************************************************
* 
* Methods for working with file extensions.
* 
********************************************************************************************/
public class FileExt
{
	/** Only static methods in the class, so prevent construction */
	private FileExt()
	{
	}

	// Returns the extension in lower case (without a leading ".')
	public static String GetLowerFileExtension(String filePath)
	{
		// Find where the extension starts
		int lastDot = filePath.lastIndexOf('.') ;
		
		// Return the empty string if there's no extension
		if (lastDot == -1 || lastDot == filePath.length() - 1)
			return "" ;
		
		// Get the extension in lower case
		String ext = filePath.substring(lastDot + 1).toLowerCase() ;
		
		return ext ;
	}
	
	// Make the file path have the given extension
	// The extension can start with or without a "."
	public static String SetFileExtension(String filePath, String newExt)
	{
		// Check for adding a null extension
		if (newExt == null || newExt.length() == 0)
			return filePath ;
	
		// Find where the extension starts
		int lastDot = filePath.lastIndexOf('.') ;
		
		if (lastDot == -1)
			lastDot = filePath.length() ;
		
		// Get everything up to the existing extension, if there is one
		String basePath = filePath.substring(0, lastDot) ;
		
		// Add the dot if necessary
		if (newExt.charAt(0) != '.')
			basePath += "." ;
		
		// Add the new extension
		basePath += newExt ;
		
		return basePath ;
	}
}

