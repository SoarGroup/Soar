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
	private AbstractAction m_Waterjug_Lookahead = new AbstractAction("Water Jug Lookahead")				{ public void actionPerformed(ActionEvent e) { waterjugLookahead(e) ; } } ;
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
		
		// Look up the user's saved value (if there is one)
		String savedPath = m_Frame.getAppStringProperty("DemoMenu.DemoPath") ;
		File path = null ;

		if (savedPath != null)
		{
			path = new File(savedPath) ;
		}
		else
		{
			// Default path is "..\SoarIO\bin\demos"
			path = new File("..") ;
			path = new File(path, "SoarIO") ;
			path = new File(path, "bin") ;
			path = new File(path, "demos") ;
		}
 		
		try
		{
			// Canonical path produces an absolute path but also removes
			// my ".." above, making the result cleaner.
			// (It can throw because the canonical process can fail)
			m_DemoPath = new File(path.getCanonicalPath()) ;
		}
		catch (java.io.IOException e)
		{
			m_DemoPath = new File(".") ;
		}
		
		BaseMenu toh = menu.addSubmenu("Towers of hanoi") ;
		toh.add(m_TOH) ;
		toh.add(m_TOH_Recursive) ;
		BaseMenu water = menu.addSubmenu("Water Jug") ;
		water.add(m_Waterjug_Lookahead) ;
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
	
	protected void loadDemo(File filename)
	{
		// Default parameter -- echo the source command
		loadDemo(filename, true) ;
	}
	
	/** Load a specific demo.  We'll make this public so that buttons etc. can call here to load a demo */
	public void loadDemo(File filename, boolean echoCommand)
	{
		// Start by excising any existing productions
		String exciseLine = m_Document.getSoarCommands().getExciseAllCommand() ;
		m_Frame.executeCommandPrimeView(exciseLine, false) ;
		
		File filePath = new File(m_DemoPath, filename.getPath()) ;
		String commandLine = m_Document.getSoarCommands().getSourceCommand(filePath.getPath()) ;

		m_Frame.executeCommandPrimeView(commandLine, echoCommand) ;
		
		if (echoCommand && m_Frame.getAgentFocus() != null)
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
	
	private void waterjugLookahead(ActionEvent e)
	{
		loadDemo(new File("water-jug", "water-jug-look-ahead.soar")) ;
	}

	private void setDemoPath(ActionEvent e)
	{
		DirectoryDialog folderDialog = new DirectoryDialog(m_Frame.getShell(), 0) ;
		folderDialog.setText("Set Demos path") ;
		folderDialog.setMessage("Select the root of the demos folder") ;
		folderDialog.setFilterPath(m_DemoPath.getPath() + File.separator) ;

		// Display the dialog
		String filePath = folderDialog.open() ;
	
		// Check if the user cancelled
		if (filePath == null)
			return ;
		
		// Record the new path
		m_DemoPath = new File(filePath) ;
		m_Frame.setAppProperty("DemoMenu.DemoPath", m_DemoPath.getPath()) ;
	}

}
