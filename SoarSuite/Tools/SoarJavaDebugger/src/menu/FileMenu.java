/********************************************************************************************
*
* FileMenu.java
* 
* Created on 	Nov 20, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package menu;

import general.* ;
import debugger.* ;
import doc.* ;

import org.eclipse.swt.widgets.Menu;

import sml.ClientAnalyzedXML;
import sml.sml_Names;

/********************************************************************************************
* 
* The file menu
* 
********************************************************************************************/
public class FileMenu
{
	private BaseMenu	m_Menu ;
	
	private MainFrame m_Frame = null ;
	private Document  m_Document = null ;
	
	private AbstractAction m_LoadSource = new AbstractAction("Load &source file...") { public void actionPerformed(ActionEvent e) { loadSource() ; } } ;
	private AbstractAction m_ChangeDirectory = new AbstractAction("&Change current folder...") { public void actionPerformed(ActionEvent e) { changeDirectory() ; } } ;
	private AbstractAction m_LoadRete = new AbstractAction("&Load production memory (rete)...") { public void actionPerformed(ActionEvent e) { loadRete(e) ; } } ;
	private AbstractAction m_SaveRete = new AbstractAction("Save production memory (&rete)...") { public void actionPerformed(ActionEvent e) { saveRete(e) ; } } ;
	private AbstractAction m_LogNewFile 	 = new AbstractAction("Log &output to file...") { public void actionPerformed(ActionEvent e) { logNewFile(e) ; } } ;
	//private AbstractAction m_LogExistingFile = new AbstractAction("&Append log output to existing file...") { public void actionPerformed(ActionEvent e) { logAppendFile(e) ; } } ;
	//private AbstractAction m_LogClose = new AbstractAction("Turn &off logging") { public void actionPerformed(ActionEvent e) { logClose(e) ; } } ;
	//private AbstractAction m_LogStatus = new AbstractAction("Logging &status")   { public void actionPerformed(ActionEvent e) { logStatus(e) ; } } ;
	private AbstractAction m_Load 	 = new AbstractAction("Load &window layout...") { public void actionPerformed(ActionEvent e) { loadPerformed(e) ; } } ;
	private AbstractAction m_Save  	 = new AbstractAction("Save w&indow layout...") { public void actionPerformed(ActionEvent e) { savePerformed(e) ; } } ;
	private AbstractAction m_Exit 	 = new AbstractAction("E&xit") 			{ public void actionPerformed(ActionEvent e) { exitPerformed(e) ; } } ;

	/** Create this menu */
	public static FileMenu createMenu(MainFrame frame, Document doc, String title)
	{
		FileMenu menu = new FileMenu() ;
		menu.m_Document = doc ;
		menu.m_Frame    = frame ;
		
		menu.m_Menu = menu.makeMenu(frame.getMenuBar(), title) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title)
	{
		BaseMenu menu = new BaseMenu(parent, title) ;
		
		menu.add(m_LoadSource) ;
		menu.add(m_ChangeDirectory) ;
		menu.addSeparator() ;
		menu.add(m_LoadRete) ;
		menu.add(m_SaveRete) ;
		menu.addSeparator() ;
		menu.add(m_LogNewFile) ;
		//menu.add(m_LogExistingFile) ;
		//menu.add(m_LogClose) ;
		//menu.add(m_LogStatus) ;
		menu.addSeparator() ;
		menu.add(m_Load) ;
		menu.add(m_Save) ;
		menu.addSeparator() ;
		menu.add(m_Exit) ;
		
		return menu ;
	}

	public String getCurrentDirectory()
	{
		if (!m_Document.isConnected())
			return null ;
		
		String pwd = m_Document.getSoarCommands().getWorkingDirectoryCommand() ;
		
		ClientAnalyzedXML response = new ClientAnalyzedXML() ;
		boolean ok = m_Frame.executeCommandXML(pwd, response) ;

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
	
	/** Dialog to select user select a new current working directory */
	public void changeDirectory()
	{
		String cwd = getCurrentDirectory() ;
		
		String newPath = SaveLoad.SelectFolderDialog(m_Frame.getWindow(), cwd, null, "Select the current working directory") ;
		
		if (newPath != null)
		{
			String sourceLine = m_Document.getSoarCommands().getChangeDirectoryCommand(newPath) ;
			m_Frame.executeCommandPrimeView(sourceLine, true) ;
		}
	}

	/** Load a source file */
	public void loadSource()
	{
		String filename = SaveLoad.LoadFileDialog(m_Frame.getWindow(), new String[] { "*.soar" }, new String[] { "Soar source file (*.soar)" } , m_Frame.getAppProperties(), "SourceSave", "SourceLoad") ;
		
		if (filename != null)
		{
			String sourceLine = m_Document.getSoarCommands().getSourceCommand(filename) ;
			m_Frame.executeCommandPrimeView(sourceLine, true) ;
		}
	}

	/** Load a rete file */
	private void loadRete(ActionEvent e)
	{
		String filename = SaveLoad.LoadFileDialog(m_Frame.getWindow(), new String[] { "*.soarx" }, new String[] { "Soar production memory (rete) file (*.soarx)" } , m_Frame.getAppProperties(), "SourceSave", "SourceLoad") ;
		
		if (filename != null)
		{
			// Have to do init-soar before we can load rete
			String initSoar = m_Document.getSoarCommands().getInitSoarCommand() ;
			m_Frame.executeCommandPrimeView(initSoar, true) ;

			// Now load the rete
			String sourceLine = m_Document.getSoarCommands().getLoadReteCommand(filename) ;
			m_Frame.executeCommandPrimeView(sourceLine, true) ;
		}
	}

	/** Save a rete file */
	private void saveRete(ActionEvent e)
	{
		String filename = SaveLoad.SaveFileDialog(m_Frame.getWindow(), new String[] { "*.soarx" }, new String[] { "Soar production memory (rete) file (*.soarx)" } , m_Frame.getAppProperties(), "SourceSave", "SourceLoad") ;
		
		if (filename != null)
		{
			String sourceLine = m_Document.getSoarCommands().getSaveReteCommand(filename) ;
			m_Frame.executeCommandPrimeView(sourceLine, true) ;
		}
	}

	/** Log to a new file */
	private void logNewFile(ActionEvent e)
	{
		m_Frame.ShowMessageBox("Log to file", "To log output from a window to a file, right click on the window and select 'Log this window...' from the context menu") ;
		/*
		String filename = SaveLoad.SaveFileDialog(m_Frame.getWindow(), new String[] { "*.txt" }, new String[] { "Log file (*.txt)" } , m_Frame.getAppProperties(), "LogSave", "LogLoad") ;
		
		if (filename != null)
		{
			String sourceLine = m_Document.getSoarCommands().getLogNewCommand(filename) ;
			m_Frame.executeCommandPrimeView(sourceLine, true) ;
		}
		*/
	}

	/** Append log to an existing file */
	private void logAppendFile(ActionEvent e)
	{
		// We treat this as a load because we'll append to it (i.e. you must choose an existing file)
		String filename = SaveLoad.LoadFileDialog(m_Frame.getWindow(), new String[] { "*.txt" }, new String[] { "Log file (*.txt)" } , m_Frame.getAppProperties(), "LogSave", "LogLoad") ;
		
		if (filename != null)
		{
			String sourceLine = m_Document.getSoarCommands().getLogAppendCommand(filename) ;
			m_Frame.executeCommandPrimeView(sourceLine, true) ;
		}
	}

	/** Close log file */
	private void logClose(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getLogCloseCommand() ;
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	/** Report log status */
	private void logStatus(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getLogStatusCommand() ;
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	/** Load a new window layout */
	public void loadPerformed(ActionEvent e)
	{
		String filename = SaveLoad.LoadFileDialog(m_Frame.getWindow(), new String[] { "*.dlf" }, new String[] { "Debugger Layout file (*.dlf)" } , m_Frame.getAppProperties(), "SaveFile", "LoadFile") ;
		
		if (filename != null)
		{
			boolean ok = m_Frame.loadLayoutFile(filename, true) ;

			// If we fail to load the layout file go back to a default layout
			// That in turn will try to load a layout file.  If that fails we go
			// back to an internal layout (in code) which cannot fail.
			if (!ok)
			{
				m_Frame.useDefaultLayout() ;
			}
		}
	}

	/** Save the current window layout */
	public void savePerformed(ActionEvent e)
	{
		String filename = SaveLoad.SaveFileDialog(m_Frame.getWindow(), new String[] { "*.dlf" }, new String[] { "Debugger Layout file (*.dlf)" }, m_Frame.getAppProperties(), "SaveFile", "LoadFile") ;

		if (filename != null)
			m_Frame.saveLayoutFile(filename) ;
	}

	/** Change the window layout back to the default */
	private void exitPerformed(ActionEvent e)
	{
		MainFrame frame = m_Frame ;

		frame.close() ;
	}
}


