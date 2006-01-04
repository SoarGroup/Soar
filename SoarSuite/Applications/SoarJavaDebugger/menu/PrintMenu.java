/********************************************************************************************
*
* PrintMenu.java
* 
* Description:	
* 
* Created on 	Apr 12, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package menu;

import org.eclipse.swt.widgets.Menu;

import debugger.MainFrame;
import doc.Document;

/************************************************************************
 * 
 * The print menu
 * 
 ************************************************************************/
public class PrintMenu
{
	private BaseMenu  m_Menu ;

	private MainFrame m_Frame = null ;
	private Document  m_Document = null ;
	
	private AbstractAction m_PrintProductions = new AbstractAction("Print all &productions")  { public void actionPerformed(ActionEvent e) { printProductions(e) ; } } ;
	private AbstractAction m_PrintChunks      = new AbstractAction("Print all &chunks")  { public void actionPerformed(ActionEvent e) { printChunks(e) ; } } ;
	private AbstractAction m_PrintJustifications = new AbstractAction("Print all &justifications")  { public void actionPerformed(ActionEvent e) { printJustifications(e) ; } } ;
	private AbstractAction m_PrintStack = new AbstractAction("Print &goal/state stack")  { public void actionPerformed(ActionEvent e) { printStack(e) ; } } ;
	private AbstractAction m_PrintState = new AbstractAction("Print &state")  { public void actionPerformed(ActionEvent e) { printState(e) ; } } ;
	private AbstractAction m_PrintOperator = new AbstractAction("Print &operator")  { public void actionPerformed(ActionEvent e) { printOperator(e) ; } } ;
	private AbstractAction m_PrintTopState = new AbstractAction("Print &top state")  { public void actionPerformed(ActionEvent e) { printTopState(e) ; } } ;
	private AbstractAction m_PrintSuperState = new AbstractAction("Print s&uper state")  { public void actionPerformed(ActionEvent e) { printSuperState(e) ; } } ;

	/** Create this menu */
	public static PrintMenu createMenu(MainFrame frame, Document doc, String title)
	{
		PrintMenu menu = new PrintMenu() ;
		menu.m_Frame    = frame ;
		menu.m_Document = doc ;
		
		menu.m_Menu = menu.makeMenu(frame.getMenuBar(), title) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title)
	{
		BaseMenu menu = new BaseMenu(parent, title) ;
		
		menu.add(m_PrintStack) ;
		menu.add(m_PrintState) ;
		menu.add(m_PrintOperator) ;
		menu.add(m_PrintTopState) ;
		menu.add(m_PrintSuperState) ;
		menu.addSeparator() ;
		menu.add(m_PrintProductions) ;
		menu.add(m_PrintChunks) ;
		menu.add(m_PrintJustifications) ;
		
		return menu ;
	}

	private void printStack(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getPrintStackCommand() ;
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void printState(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getPrintStateCommand() ;
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void printOperator(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getPrintOperatorCommand() ;
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void printTopState(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getPrintTopStateCommand() ;
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void printSuperState(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getPrintSuperStateCommand() ;
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void printProductions(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getPrintProductionsCommand() ;
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void printJustifications(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getPrintJustificationsCommand() ;
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

	private void printChunks(ActionEvent e)
	{
		String sourceLine = m_Document.getSoarCommands().getPrintChunksCommand() ;
		m_Frame.executeCommandPrimeView(sourceLine, true) ;
	}

}
