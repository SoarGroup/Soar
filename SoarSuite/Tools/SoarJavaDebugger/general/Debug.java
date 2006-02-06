/** 
 * Debug.java
 *
 * Description:		<describe the Debug class here>
 * @author			Doug
 * @version			
 */

package general;

import java.io.* ;
import java.net.* ;

public class Debug
{
	/** Path to folder containing the application's data.
	*** Typically, this is the same folder as the .jar file for the application */
	protected static java.io.File s_DataDirectory = null ;

	// Changing this to true/false makes it possible
	// to compile out code that tests it, because it's a constant.
	public static final boolean debug = true ;
	
	// Changing this to true/false turns on and off
	// more verbose chunks of output.
	public static final boolean verboseDebug = false ;

	/** Specifically controls output for the rule matcher */
	public static final boolean debugMatcher = true ;

	public static void Check(boolean condition)
	{
		if (!condition)
			throw new Error("Condition failed check") ;
	}
	
	public static void println(String message)
	{
		// If this flag is false, suppress all debug output.
		if (Debug.debug)
		{
			System.out.println(message) ;
		}
	}
	
	public static void print(String message)
	{
		// If this flag is false, suppress all debug output.
		if (Debug.debug)
		{
			System.out.print(message) ;
		}
	}
	
	/************************************************************************
    * Finds the folder that contains the application's jar file.
    * The method is to look up the location of this class.
    * Note--this is potentially invalid if the application is being
    * run over the net or if this is called inside an applet.
    * 
    * @return - Null if there is an error, otherwise the folder containing app's jar file.
    * 
    *************************************************************************/
    public static File FindDataDirectory()
    {
    	// Check to see if we've already cached the result.
    	if (Debug.s_DataDirectory != null)
    		return Debug.s_DataDirectory ;
    
    	// Look up the location of one of the classes in our app.
    	// We're using ourselves here (general.Debug)
        java.security.CodeSource source = general.Debug.class.getProtectionDomain().getCodeSource();

        if (source == null) return null;

        File dataDir;

        try
        {
            URL sourceURI = new URL(source.getLocation().toString());
            dataDir = new File(sourceURI.getFile());
        }
        catch (MalformedURLException e)
        {
            return null;
        }
        catch (IllegalArgumentException e)
        {
            return null;
        }

        if (!dataDir.isDirectory()) dataDir = dataDir.getParentFile();
        
        // Cache the result, so we can get this value quickly in future calls.
        s_DataDirectory = dataDir ;
        
        return dataDir;
    }

}


/* @(#)Debug.java */
