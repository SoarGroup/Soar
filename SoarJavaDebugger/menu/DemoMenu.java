/********************************************************************************************
*
* DemoMenu.java
* 
* Description:	
* 
* Created on 	Mar 17, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package menu;

import java.io.File;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.widgets.Dialog;

import sml.Agent;
import sml.Kernel;

import debugger.MainFrame;
import doc.Document;

/************************************************************************
 * 
 * Menu that offers a list of demos to load
 * 
 ************************************************************************/
public class DemoMenu
{
	private BaseMenu	m_Menu ;
	
	private Document  m_Document = null ;
	private MainFrame m_Frame	= null ;
	private File	  m_DemoPath ;

	private AbstractAction m_TOH       		= new AbstractAction("Towers of Hanoi") 					{ public void actionPerformed(ActionEvent e) { toh(e) ; } } ;
	private AbstractAction m_TOH_Recursive 	= new AbstractAction("Towers of Hanoi Recursive") 			{ public void actionPerformed(ActionEvent e) { tohRecursive(e) ; } } ;
	private AbstractAction m_DemoPathItem  	= new AbstractAction("Set the path to the demos folder...")	{ public void actionPerformed(ActionEvent e) { setDemoPath(e) ; } } ;

	/** Create this menu */
	public static DemoMenu createMenu(MainFrame frame, Document doc, String title, char mnemonicChar)
	{
		DemoMenu menu = new DemoMenu() ;
		menu.m_Frame    = frame ;
		menu.m_Document = doc ;
		
		menu.m_Menu = menu.makeMenu(frame.getMenuBar(), title, mnemonicChar) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title, char mnemonicChar)
	{
		BaseMenu menu = new BaseMenu(parent, title, mnemonicChar) ;
		
		File path = new File("..") ;
		path = new File(path, "SoarIO") ;
		path = new File(path, "bin") ;
		path = new File(path, "demos") ;
		
		try
		{
			// Canonical path produces an absolute path but also removes
			// my ".." above, making the result cleaner.
			m_DemoPath = new File(path.getCanonicalPath()) ;
		}
		catch (java.io.IOException e)
		{
			m_DemoPath = new File(".") ;
		}
		
		BaseMenu toh = menu.addSubmenu("Towers of hanoi") ;
		toh.add(m_TOH) ;
		toh.add(m_TOH_Recursive) ;
		menu.addSeparator() ;
		menu.add(m_DemoPathItem) ;

		updateMenu() ;
		
		return menu ;		
	}
	
	/** Enable/disable the menu items depending on the current state of the kernel */
	public void updateMenu()
	{
		// All are always enabled for now
	}
	
	/** Load a specific demo.  We'll make this public so that buttons etc. can call here to load a demo */
	public void loadDemo(File filename)
	{
		File filePath = new File(m_DemoPath, filename.getPath()) ;
		String commandLine = m_Document.getSoarCommands().getSourceCommand() + " " + filePath.getPath() ;

		m_Frame.executeCommandPrimeView(commandLine, true) ;
		m_Frame.displayTextInPrimeView("\nType 'run' to execute the demo.") ;
	}

	private void toh(ActionEvent e)
	{
		loadDemo(new File("towers-of-hanoi","towers-of-hanoi.soar")) ;
	}

	private void tohRecursive(ActionEvent e)
	{
		loadDemo(new File("towers-of-hanoi","towers-of-hanoi-recursive.soar")) ;
	}

	private void setDemoPath(ActionEvent e)
	{
		// For now I'll use a file dialog rather than a directory dialog because
		// this allows me to set an original file path, which can be very helpful
		// (and on Windows at least the file dialog is much more familiar to the user).
		FileDialog fileDialog = new FileDialog(m_Frame.getShell(), 0) ;
		
		fileDialog.setFileName(m_DemoPath.getPath() + File.separator) ;
		fileDialog.setText("Select a file in the root of the demos folder") ;

		// Display the dialog
		String filePath = fileDialog.open() ;
	
		// Check if the user cancelled
		if (filePath == null)
			return ;
		
		// Extract the folder from the path
		File file = new File(filePath) ;
		m_DemoPath = new File(file.getParent()) ;
		
	}

}
