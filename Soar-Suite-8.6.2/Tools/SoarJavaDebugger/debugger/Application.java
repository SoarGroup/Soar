/** 
 * Application.java
 *
 * Title:			Debugger for Soar written in Java
 * Description:	
 * @author			Douglas Pearson, 2005 (www.threepenny.net)
 * @version			
 */

package debugger;

import java.io.*;

public class Application {
	
	/************************************************************************
	*
	* Look for the DLLs we need.  If we don't find them then try to unpack them
	* from our JAR file onto the hard drive so we can load them.
	* 
	* We may choose to run this everytime we launch or just the first time, from
	* a menu etc.
	* 
	* @param installAlways -- overwrite the existing files.  Ensures we have libs to match this debugger
	*************************************************************************/
	private void Install(String[] libraries, String extension, boolean installAlways) throws IOException
	{	
		for (int i = 0 ; i < libraries.length ; i++)
		{
			// We just work relative to the current directory because that's how
			// load library will work.
			File library = new File(libraries[i] + extension) ;

			if (library.exists() && !installAlways)
			{
				System.out.println(library + " already exists so not installing from the JAR file") ;
				continue ;
			}
			
			// Get the DLL from inside the JAR file
			// It should be placed at the root of the JAR (not in a subfolder)
			String jarpath = "/" + library.getPath() ;
			InputStream is = this.getClass().getResourceAsStream(jarpath) ;
			
			if (is == null)
			{
				System.out.println("Failed to find " + jarpath + " in the JAR file") ;
				continue ;
			}
			
			// Make sure we can delete the library.  This is actually here to cover the
			// case where we're running in Eclipse without a JAR file.  The getResourceAsStream()
			// call can end up loading the same library that we're trying to save to and we
			// end up with a blank file.  Explicitly trying to delete it first ensures that
			// we're not reading the same file that we're writing.
			if (library.exists() && !library.delete())
			{
				System.out.println("Failed to remove the existing layout file " + library) ;
				continue ;
			}
			
			// Create the new file on disk
			FileOutputStream os = new FileOutputStream(library) ;
			
			// Copy the file onto disk
			byte bytes[] = new byte[2048];
			int read;
			while (true)
			{
				read = is.read( bytes) ;
				
				// EOF
				if ( read == -1 ) break;
				
				os.write( bytes, 0, read);
			}

			is.close() ;
			os.close() ;
			
			System.out.println("Installed " + library + " onto the local disk from JAR file") ;
		}
	}
	
	/************************************************************************
	*
	* Default constructor for the application -- creates the main frame and
	* set the look and feel.
	* 
	*************************************************************************/
	public Application(String[] args, boolean alwaysInstallLibs) {
		try {
			// Step 1: Install the SWT (UI) libraries
			// SWT uses platform specific libraries so we need to decide which platform we're on before
			// choosing which libraries to load.
			//Install(new String[] { "swt.jar", "javaw.exe.manifest", "swt-awt-win32-3062.dll", "swt-win32-3062.dll"} , "", alwaysInstallLibs) ;
			
			// Step 2: Install the SML (Soar) libraries
			// NOTE: Should choose the extension to match the platform we're on
			//String extension = ".dll" ;		
			//Install(new String[] { "SoarKernelSML", "ElementXML", "Java_sml_ClientInterface" }, extension, alwaysInstallLibs) ;
			
			// Step 3: Insall the default layout file
			Install(new String[] { "default-layout.dlf", "default-text.dlf" } , "", true) ;

			// Start the SWT version of the application (we used to have a Swing version too)
			SWTApplication swtApp = new SWTApplication() ;
			
			swtApp.startApp(args) ;
			
			System.exit(0) ;
		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	/*************************************************************************
	*
	* The application's main entry point.
	* 
	* @param args			Command line arguments.  Currently ignored.
	* 
	*************************************************************************/
	static public void main(String[] args) {
		new Application(args, false);
	}
	
}
