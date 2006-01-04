/********************************************************************************************
*
* CommandsMenu.java
* 
* Description:	
* 
* Created on 	Apr 12, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package menu;

import debugger.* ;
import doc.Document;

import org.eclipse.swt.widgets.Menu;

/********************************************************************************************
* 
* The commands menu
* 
********************************************************************************************/
public class CommandsMenu
{
	private BaseMenu  m_Menu ;

	private MainFrame m_Frame = null ;
	private Document  m_Document = null ;
	
	private AbstractAction m_RestartAgent = new AbstractAction("Clear &working memory")  { public void actionPerformed(ActionEvent e) { initSoar(e) ; } } ;
	private AbstractAction m_ExciseAll    = new AbstractAction("Excise &all productions")  { public void actionPerformed(ActionEvent e) { exciseAll(e) ; } } ;
	private AbstractAction m_ExciseChunks = new AbstractAction("Excise &chunks")  { public void actionPerformed(ActionEvent e) { exciseChunks(e) ; } } ;
	private AbstractAction m_ExciseTask = new AbstractAction("Excise &task productions")  { public void actionPerformed(ActionEvent e) { exciseTask(e) ; } } ;
	private AbstractAction m_ExciseUser = new AbstractAction("Excise &user productions")  { public void actionPerformed(ActionEvent e) { exciseUser(e) ; } } ;
	private AbstractAction m_ExciseDefault = new AbstractAction("Excise &default productions")  { public void actionPerformed(ActionEvent e) { exciseDefault(e) ; } } ;

	/** Create this menu */
	public static CommandsMenu createMenu(MainFrame frame, Document doc, String title)
	{
		CommandsMenu menu = new CommandsMenu() ;
		menu.m_Frame    = frame ;
		menu.m_Document = doc ;
		
		menu.m_Menu = menu.makeMenu(frame.getMenuBar(), title) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title)
	{
		BaseMenu menu = new BaseMenu(parent, title) ;
		
		menu.add(m_RestartAgent) ;
		
		menu.addSeparator() ;	
//		BaseMenu excise = menu.addSubmenu("Excise productions") ;
		menu.add(m_ExciseAll) ;
		menu.add(m_ExciseChunks) ;
		menu.add(m_ExciseTask) ;
		menu.add(m_ExciseUser) ;
		menu.add(m_ExciseDefault) ;

		return menu ;
	}

	private void initSoar(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getInitSoarCommand() ;
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void exciseAll(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getExciseAllCommand() ;
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void exciseChunks(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getExciseChunksCommand();
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void exciseTask(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getExciseTaskCommand();
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void exciseUser(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getExciseUserCommand();
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void exciseDefault(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getExciseDefaultCommand();
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

}
