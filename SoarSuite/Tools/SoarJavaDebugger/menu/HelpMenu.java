/********************************************************************************************
*
* HelpMenu.java
* 
* Description:	Help menu
* 
* Created on 	Apr 10, 2006
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package menu;

import general.OSName;
import general.StartBrowser;

import java.io.File;
import java.net.URL;

import org.eclipse.swt.widgets.Menu;

import sml.Kernel;

import debugger.MainFrame;
import doc.Document;

public class HelpMenu {
	private BaseMenu	m_Menu ;
	
	private Document  m_Document = null ;
	private MainFrame m_Frame	= null ;

	private AbstractAction m_About   	= new AbstractAction("About Soar's Debugger") 	{ public void actionPerformed(ActionEvent e) { about() ; } } ;
	private AbstractAction m_Homepage	= new AbstractAction("Online - Soar Home page") 		{ public void actionPerformed(ActionEvent e) { open("http://sitemaker.umich.edu/soar") ; } } ;
	private AbstractAction m_Wiki 	 	= new AbstractAction("Online - Soar Wiki") 				{ public void actionPerformed(ActionEvent e) { open("http://winter.eecs.umich.edu/soarwiki/Main_Page") ; } } ;
	private AbstractAction m_CLI 	 	= new AbstractAction("Online - Soar Command Line Help") { public void actionPerformed(ActionEvent e) { open("http://winter.eecs.umich.edu/soarwiki/Soar_Command_Line_Interface") ; } } ;
	private AbstractAction m_FAQ 	 	= new AbstractAction("Online - Soar FAQ") 				{ public void actionPerformed(ActionEvent e) { open("http://acs.ist.psu.edu/projects/soar-faq/soar-faq.html") ; } } ;
	private AbstractAction m_Tutorial1 	= new AbstractAction("Soar Tutorial - Part 1") 	{ public void actionPerformed(ActionEvent e) { local("../Documentation/Soar Tutorial Part 1.pdf") ; } } ;
	private AbstractAction m_Tutorial2 	= new AbstractAction("Soar Tutorial - Part 2") 	{ public void actionPerformed(ActionEvent e) { local("../Documentation/Soar Tutorial Part 2.pdf") ; } } ;
	private AbstractAction m_Tutorial3 	= new AbstractAction("Soar Tutorial - Part 3") 	{ public void actionPerformed(ActionEvent e) { local("../Documentation/Soar Tutorial Part 3.pdf") ; } } ;
	private AbstractAction m_Tutorial4 	= new AbstractAction("Soar Tutorial - Part 4") 	{ public void actionPerformed(ActionEvent e) { local("../Documentation/Soar Tutorial Part 4.pdf") ; } } ;
	private AbstractAction m_Tutorial5 	= new AbstractAction("Soar Tutorial - Part 5") 	{ public void actionPerformed(ActionEvent e) { local("../Documentation/Soar Tutorial Part 5.pdf") ; } } ;
	private AbstractAction m_Tutorial6 	= new AbstractAction("Soar Tutorial - Part 6") 	{ public void actionPerformed(ActionEvent e) { local("../Documentation/Soar Tutorial Part 6.pdf") ; } } ;
	private AbstractAction m_DebuggerHelp = new AbstractAction("Intro to the Debugger") { public void actionPerformed(ActionEvent e) { local("../Documentation/Intro to the Soar Debugger in Java.pdf") ; } } ;
	private AbstractAction m_SMLHelp      = new AbstractAction("General Intro to Soar Markup Language (SML)") { public void actionPerformed(ActionEvent e) { local("../Documentation/SML Quick Start Guide.pdf") ; } } ;	
	
	/** Create this menu */
	public static HelpMenu createMenu(MainFrame frame, Document doc, String title)
	{
		HelpMenu menu = new HelpMenu() ;
		menu.m_Frame    = frame ;
		menu.m_Document = doc ;
		
		menu.m_Menu = menu.makeMenu(frame.getMenuBar(), title) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title)
	{
		BaseMenu menu = new BaseMenu(parent, title) ;

		menu.add(m_Homepage) ;
		menu.add(m_Wiki) ;
		menu.add(m_CLI) ;
		menu.add(m_FAQ) ;
		menu.addSeparator() ;
		
		menu.add(m_DebuggerHelp) ;
		menu.add(m_SMLHelp) ;
		menu.addSeparator() ;

		menu.add(m_Tutorial1) ;
		menu.add(m_Tutorial2) ;
		menu.add(m_Tutorial3) ;
		menu.add(m_Tutorial4) ;
		menu.add(m_Tutorial5) ;
		menu.add(m_Tutorial6) ;
		menu.addSeparator() ;
		
		menu.add(m_About) ;
		
		return menu ;
	}
	
	/** Enable/disable the menu items depending on the current state of the kernel */
	public void updateMenu()
	{
		// All are always enabled for now
	}
	
	private void about()
	{
		String newLine = OSName.kSystemLineSeparator ;

		// This version is based on the version of the client interface compiled into the debugger
		// and is always available.
		String aboutText = newLine + "\tSoar Debugger version: \t" + Kernel.GetSoarClientVersion() + newLine ;

		// This version is based on the runtime
		aboutText += "\tSoar Kernel version: \t" ;
		
		String kernelVersion = m_Document.getKernelVersion() ;
		
		if (kernelVersion != null)
			aboutText += kernelVersion ;
		else
			aboutText += "no kernel is running." ;
		
		// This version is for the communication library
		aboutText += newLine + "\tSML version: \t\t" + Kernel.GetSMLVersion() + newLine ;

		aboutText += newLine + "\tDeveloped by: " + newLine + "\t\tDouglas Pearson" + newLine + "\t\tBob Marinier" + newLine + "\t\tKaren Coulter" + newLine + "\t\tJon Voigt" + newLine + "\t\tand others." + newLine ;
		
		aboutText += newLine + "For help with the debugger or other aspects of Soar " + Kernel.GetSoarClientVersion() + newLine + "application development send email to soar-sml-list@umich.edu" + newLine + newLine ;
		aboutText += "For help with writing Soar systems (production rules) or for theoretical discussions " + newLine + "of the Soar cognitive architecture send email to soar-group@lists.sourceforge.net" + newLine ;
		
		m_Frame.ShowMessageBox("About Soar's Debugger", aboutText) ;
	}
	
	private void open(String url)
	{
		try
		{
			StartBrowser.openURL(url) ;
		}
		catch (Exception e)
		{
			m_Frame.ShowMessageBox("Error launching URL " + url, e.getLocalizedMessage()) ;
		}
	}
	
	private void local(String relativeUrl)
	{
		String path = m_Frame.getLibraryLocation() ;
		
		if (path == null || path == "")
		{
			m_Frame.ShowMessageBox("Error launching URL " + relativeUrl, "Need to be running a Soar kernel so we can find the files") ;
			return ;
		}
		
		try
		{
			File file = new File(path, relativeUrl) ;
			
			if (!file.exists())
			{
				m_Frame.ShowMessageBox("Could not find this file", file.toString()) ;
				return ;
			}
			
			URL url = file.toURL() ;
			String urlString = url.toString() ;
			open(urlString) ;
		}
		catch (Exception e)
		{
			m_Frame.ShowMessageBox("Error launching URL " + relativeUrl, e.getLocalizedMessage()) ;			
		}
	}
}
