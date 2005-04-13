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
	
	private AbstractAction m_LoadSource = new AbstractAction("Load source file...") { public void actionPerformed(ActionEvent e) { loadSource(e) ; } } ;
	private AbstractAction m_LoadRete = new AbstractAction("Load production memory (rete)...") { public void actionPerformed(ActionEvent e) { loadRete(e) ; } } ;
	private AbstractAction m_SaveRete = new AbstractAction("Save production memory (rete)...") { public void actionPerformed(ActionEvent e) { saveRete(e) ; } } ;
	private AbstractAction m_LogNewFile 	 = new AbstractAction("Log output to new file...") { public void actionPerformed(ActionEvent e) { logNewFile(e) ; } } ;
	private AbstractAction m_LogExistingFile = new AbstractAction("Append log output to existing file...") { public void actionPerformed(ActionEvent e) { logAppendFile(e) ; } } ;
	private AbstractAction m_LogClose = new AbstractAction("Turn off logging") { public void actionPerformed(ActionEvent e) { logClose(e) ; } } ;
	private AbstractAction m_LogStatus = new AbstractAction("Logging status")   { public void actionPerformed(ActionEvent e) { logStatus(e) ; } } ;
	private AbstractAction m_Load 	 = new AbstractAction("Load layout...") { public void actionPerformed(ActionEvent e) { loadPerformed(e) ; } } ;
	private AbstractAction m_Save  	 = new AbstractAction("Save layout...") { public void actionPerformed(ActionEvent e) { savePerformed(e) ; } } ;
	private AbstractAction m_Default = new AbstractAction("Use Default layout") { public void actionPerformed(ActionEvent e) { useDefaultPerformed(e) ; } } ;
	private AbstractAction m_Exit 	 = new AbstractAction("Exit") 			{ public void actionPerformed(ActionEvent e) { exitPerformed(e) ; } } ;

	/** Create this menu */
	public static FileMenu createMenu(MainFrame frame, Document doc, String title, char mnemonicChar)
	{
		FileMenu menu = new FileMenu() ;
		menu.m_Document = doc ;
		menu.m_Frame    = frame ;
		
		menu.m_Menu = menu.makeMenu(frame.getMenuBar(), title, mnemonicChar) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title, char mnemonicChar)
	{
		BaseMenu menu = new BaseMenu(parent, title, mnemonicChar) ;
		
		menu.add(m_LoadSource) ;
		menu.addSeparator() ;
		menu.add(m_LoadRete) ;
		menu.add(m_SaveRete) ;
		menu.addSeparator() ;
		menu.add(m_LogNewFile) ;
		menu.add(m_LogExistingFile) ;
		menu.add(m_LogClose) ;
		menu.add(m_LogStatus) ;
		menu.addSeparator() ;
		menu.add(m_Load) ;
		menu.add(m_Save) ;
		menu.add(m_Default) ;
		menu.addSeparator() ;
		menu.add(m_Exit) ;
		
		return menu ;
	}
	
	/** Load a source file */
	private void loadSource(ActionEvent e)
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
		String filename = SaveLoad.SaveFileDialog(m_Frame.getWindow(), new String[] { "*.txt" }, new String[] { "Log file (*.txt)" } , m_Frame.getAppProperties(), "LogSave", "LogLoad") ;
		
		if (filename != null)
		{
			String sourceLine = m_Document.getSoarCommands().getLogNewCommand(filename) ;
			m_Frame.executeCommandPrimeView(sourceLine, true) ;
		}
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
	private void loadPerformed(ActionEvent e)
	{
		String filename = SaveLoad.LoadFileDialog(m_Frame.getWindow(), new String[] { "*.xml" }, new String[] { "Debugger Layout file (*.xml)" } , m_Frame.getAppProperties(), "SaveFile", "LoadFile") ;
		
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
	private void savePerformed(ActionEvent e)
	{
		String filename = SaveLoad.SaveFileDialog(m_Frame.getWindow(), new String[] { "*.xml" }, new String[] { "Debugger Layout file (*.xml)" }, m_Frame.getAppProperties(), "SaveFile", "LoadFile") ;

		if (filename != null)
			m_Frame.saveLayoutFile(filename) ;
	}

	/** Change the window layout back to the default */
	private void useDefaultPerformed(ActionEvent e)
	{
		MainFrame frame = m_Frame ;
		
		frame.useDefaultLayout() ;
	}

	/** Change the window layout back to the default */
	private void exitPerformed(ActionEvent e)
	{
		MainFrame frame = m_Frame ;

		frame.close() ;
	}
}


