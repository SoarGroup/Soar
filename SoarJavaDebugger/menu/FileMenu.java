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
		
		menu.add(m_Load) ;
		menu.add(m_Save) ;
		menu.add(m_Default) ;
		menu.addSeparator() ;
		menu.add(m_Exit) ;
		
		return menu ;
	}

	/** Load a new window layout */
	private void loadPerformed(ActionEvent e)
	{
		String filename = SaveLoad.LoadFileDialog(m_Frame.getWindow(), new String[] { "*.xml" }, new String[] { "Debugger Layout file (*.xml)" } , m_Frame.getAppProperties(), "SaveFile", "LoadFile") ;
		
		if (filename != null)
			m_Frame.loadLayoutFile(filename, true) ;
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


