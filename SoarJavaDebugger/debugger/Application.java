/** 
 * Application.java
 *
 * Title:			Debugger for Soar written in Java
 * Description:	
 * @author			Douglas Pearson, 2005 (www.threepenny.net)
 * @version			
 */

package debugger;
/*
import javax.swing.*;
import java.awt.*;
import javax.swing.plaf.*;
*/

import java.io.*;
import org.eclipse.*;
import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;

import doc.Document;

import sml.*;

public class Application {

	private Document m_Document = null ;
	
	public class TopLevelExceptionHandler
	{
		// Called when an otherwise untrapped exception occurs in the event thread.
		public void handle(Throwable thrown)
		{
			debugger.MainFrame frame = Document.getMainDocument().getFirstFrame() ;

			if (frame != null)
				frame.ShowMessageBox("Unexpected exception thrown", thrown.getStackTrace() + thrown.getMessage()) ;
		}
	}

	private void SetupExceptionHandler()
	{
		// BUGBUG: This isn't working.
		
		// This property is documented by Sun as a temporary workaround
		// for a feature missing from their APIs.
		// It allows me to install a global exception handler for the
		// main event thread.
		// System.setProperty("sun.awt.exception.handler","VisualEditor.TopLevelExceptionHandler");
	}
	
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
	
	public void SWTApp(boolean alwaysInstallLibs) throws Exception
	{
		// Step 1: Install the SWT (UI) libraries
		// BUGBUG: SWT uses platform specific libraries so we need to decide which platform we're on before
		// choosing which libraries to load.
		Install(new String[] {"javaw.exe.manifest", "swt-awt-win32-3062.dll", "swt-win32-3062.dll"} , "", alwaysInstallLibs) ;
		
		// Step 2: Install the SML (Soar) libraries
		// BUGBUG: Should choose the extension to match the platform we're on
		String extension = ".dll" ;		
		Install(new String[] { "KernelSML", "ElementXML", "Java_sml_ClientInterface" }, extension, alwaysInstallLibs) ;

		Display display = new Display() ;
		Shell shell = new Shell(display) ;
		
		// The document manages the Soar process
		m_Document = new Document() ;
		
		MainFrame frame = new MainFrame(shell, m_Document) ;
		frame.initComponents();
		frame.setVisible(true);
		
		// We wait until we have a frame up before starting the kernel
		// so it's just as if the user chose to do this manually
		// (saves some special case logic in the frame)
		Agent agent = m_Document.startLocalKernel(Kernel.GetDefaultPort()) ;
		frame.setAgentFocus(agent) ;	
		
		shell.open() ;
		
		m_Document.pumpMessagesTillClosed() ;

		display.dispose() ;
	}

	/************************************************************************
	*
	* Default constructor for the application -- creates the main frame and
	* set the look and feel.
	* 
	*************************************************************************/
	public Application(boolean alwaysInstallLibs) {
		try {
			SWTApp(alwaysInstallLibs) ;
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
		new Application(false);
	}
	
}
