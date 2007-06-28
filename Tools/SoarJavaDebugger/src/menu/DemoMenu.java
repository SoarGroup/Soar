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
import sml.sml_Names;

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

	private AbstractAction m_TOH       		= new AbstractAction("Towers of Hanoi")
				{ public void actionPerformed(ActionEvent e) { loadDemo(new File("towers-of-hanoi","towers-of-hanoi.soar")) ; } } ;
	private AbstractAction m_TOH_Recursive  = new AbstractAction("Towers of Hanoi Recursive")
				{ public void actionPerformed(ActionEvent e) { loadDemo(new File("towers-of-hanoi","towers-of-hanoi-recursive.soar")) ; } } ;

	private AbstractAction m_Waterjug_Lookahead = new AbstractAction("Water Jug Lookahead")	
				{ public void actionPerformed(ActionEvent e) { loadDemo(new File("water-jug","water-jug-look-ahead.soar")) ; } } ;
	private AbstractAction m_Waterjug = new AbstractAction("Water Jug")	
				{ public void actionPerformed(ActionEvent e) { loadDemo(new File("water-jug","water-jug.soar")) ; } } ;

	private AbstractAction m_BlocksWorld    = new AbstractAction("Blocks World") 
				{ public void actionPerformed(ActionEvent e) { loadDemo(new File("blocks-world", "blocks-world.soar")) ; } } ;
	private AbstractAction m_BlocksWorldOpSub = new AbstractAction("Blocks World Operator Subgoaling") 
				{ public void actionPerformed(ActionEvent e) { loadDemo(new File("blocks-world", "blocks-world-operator-subgoaling.soar")) ; } } ;
	private AbstractAction m_BlocksWorldLookahead = new AbstractAction("Blocks World Lookahead") 
				{ public void actionPerformed(ActionEvent e) { loadDemo(new File("blocks-world", "blocks-world-look-ahead.soar")) ; } } ;

	private AbstractAction m_EightPuzzle    = new AbstractAction("Eight puzzle") 
				{ public void actionPerformed(ActionEvent e) { loadDemo(new File("eight-puzzle", "eight-puzzle.soar")) ; } } ;
	private AbstractAction m_FifteenPuzzle = new AbstractAction("Fifteen puzzle") 
				{ public void actionPerformed(ActionEvent e) { loadDemo(new File("eight-puzzle", "fifteen-puzzle.soar")) ; } } ;

	private AbstractAction m_Mac    = new AbstractAction("Missionaries and Cannibals") 
				{ public void actionPerformed(ActionEvent e) { loadDemo(new File("mac", "mac.soar")) ; } } ;
	private AbstractAction m_MacPlanning = new AbstractAction("Missionaries Planning") 
				{ public void actionPerformed(ActionEvent e) { loadDemo(new File("mac", "mac-planning.soar")) ; } } ;
	private AbstractAction m_Arithmetic    = new AbstractAction("Arithmetic") 
				{ public void actionPerformed(ActionEvent e) { loadDemo(new File("arithmetic", "arithmetic.soar")) ; } } ;
			
//    private AbstractAction m_DemoPathItem  	= new AbstractAction("Set the library path (contains the demos)...")	{ public void actionPerformed(ActionEvent e) { setLibraryPath() ; } } ;

	/** Create this menu */
	public static DemoMenu createMenu(MainFrame frame, Document doc, String title)
	{
		DemoMenu menu = new DemoMenu() ;
		menu.m_Frame    = frame ;
		menu.m_Document = doc ;
		
		menu.m_Menu = menu.makeMenu(frame.getMenuBar(), title) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title)
	{
		BaseMenu menu = new BaseMenu(parent, title) ;
		
		BaseMenu blocks = menu.addSubmenu("Blocks World") ;
		blocks.add(m_BlocksWorld) ;
		blocks.add(m_BlocksWorldOpSub) ;
		blocks.add(m_BlocksWorldLookahead) ;
		
		BaseMenu eight = menu.addSubmenu("Eight puzzle") ;
		eight.add(m_EightPuzzle) ;
		eight.add(m_FifteenPuzzle) ;
		
		BaseMenu mac = menu.addSubmenu("Missionaries") ;
		mac.add(m_Mac) ;
		mac.add(m_MacPlanning) ;
		
		BaseMenu toh = menu.addSubmenu("Towers of hanoi") ;
		toh.add(m_TOH) ;
		toh.add(m_TOH_Recursive) ;

		BaseMenu water = menu.addSubmenu("Water Jug") ;
		water.add(m_Waterjug) ;
		water.add(m_Waterjug_Lookahead) ;

		menu.add(m_Arithmetic) ;
		
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
		m_Frame.executeCommandPrimeView(exciseLine, true) ;
		
		// Then do an init-soar
		String initSoar = m_Document.getSoarCommands().getInitSoarCommand() ;
		m_Frame.executeCommandPrimeView(initSoar, true) ;
		
		File filePath = null ;

		String libraryPath = getLibraryLocation() ;
		File demoPath = new File(libraryPath, "Demos") ;
		filePath = new File(demoPath, filename.getPath()) ;
		
		String commandLine = m_Document.getSoarCommands().getSourceCommand(filePath.getPath()) ;

		m_Frame.executeCommandPrimeView(commandLine, echoCommand) ;
		
		if (echoCommand && m_Frame.getAgentFocus() != null)
			m_Frame.displayTextInPrimeView("\nType 'run' to execute the demo.") ;
	}
	
	public String getLibraryLocation()
	{
		if (!m_Document.isConnected())
			return "" ;
		
		String getLocation = m_Document.getSoarCommands().getLibraryLocationCommand() ;
		
		sml.ClientAnalyzedXML response = new sml.ClientAnalyzedXML() ;
		boolean ok = m_Frame.executeCommandXML(getLocation, response) ;

		// Check if the command failed
		if (!ok)
		{
			response.delete() ;
			return "" ;
		}
		
		// Debug code to look at the result of the command in XML
		//String check = response.GenerateXMLString(true) ;
		String location = response.GetArgString(sml_Names.getKParamDirectory()) ;
		
		// BADBAD: Need to strip quotes - they shouldn't really be added
		if (location != null && location.length() > 2 && location.charAt(0) == '"')
		{
			// Chop leading and trailing quote
			location = location.substring(1, location.length() - 1) ;
		}
		
		// We do explicit clean up so we can check for memory leaks when debugger exits.
		// (See executeCommandXMLcomment for more).
		response.delete() ;
		
		return location ;
	}
	
	/*
	private boolean setLibraryPath()
	{
		if (!m_Document.isConnected())
		{
			m_Frame.ShowMessageBox("No kernel running", "Need to have Soar running before changing the library path (its stored inside the kernel)") ;
			return false ;
		}

		String newPath = m_Frame.getKernelMenu().setKernelLocation() ;
		if (newPath == null)
			return false ;

		// The library path is the parent of the kernel DLL (at least with our current structure)
		File path = new File(newPath) ;
		String libPath = path.getParent() ;

		if (libPath == null)
			return false ;

		String setLocation = m_Document.getSoarCommands().setLibraryLocationCommand(libPath) ;
		m_Frame.executeCommandPrimeView(setLocation, true) ;
		
		return true ;
	}
	*/
}
