/********************************************************************************************
*
* LayoutMenu.java
* 
* Description:	
* 
* Created on 	Apr 12, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package edu.umich.soar.debugger.menu;

import org.eclipse.swt.widgets.Menu;

import edu.umich.soar.debugger.MainFrame;
import edu.umich.soar.debugger.doc.Document;

/************************************************************************
 * 
 * The layout menu
 * 
 ************************************************************************/
public class LayoutMenu
{
	private MainFrame m_Frame = null ;

	private AbstractAction m_UseDefaultLayout = new AbstractAction("Use &default window layout")  { public void actionPerformed(ActionEvent e) { useDefaultPerformed(e) ; } } ;
	private AbstractAction m_UseTreeLayout = new AbstractAction("Use &tree view layout")  { public void actionPerformed(ActionEvent e) { useDefaultPerformed(e) ; } } ;
	private AbstractAction m_UseTextLayout = new AbstractAction("Use te&xt view layout")  { public void actionPerformed(ActionEvent e) { useTextLayout(e) ; } } ;
	private AbstractAction m_Load 	 = new AbstractAction("&Load window layout...") { public void actionPerformed(ActionEvent e) { load(e) ; } } ;
	private AbstractAction m_Save  	 = new AbstractAction("&Save window layout...") { public void actionPerformed(ActionEvent e) { save(e) ; } } ;

	/** Create this menu */
	public static LayoutMenu createMenu(MainFrame frame, Document doc, String title)
	{
		LayoutMenu menu = new LayoutMenu() ;
		menu.m_Frame    = frame ;
		
		menu.makeMenu(frame.getMenuBar(), title) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title)
	{
		BaseMenu menu = new BaseMenu(parent, title) ;
		
		menu.add(m_UseDefaultLayout) ;
		menu.add(m_UseTreeLayout) ;
		menu.add(m_UseTextLayout) ;
		menu.addSeparator() ;
		menu.add(m_Load) ;
		menu.add(m_Save) ;
		
		return menu ;
	}

	private void load(ActionEvent e)
	{
		m_Frame.getFileMenu().loadPerformed(e) ;
	}
	
	private void save(ActionEvent e)
	{
		m_Frame.getFileMenu().savePerformed(e) ;
	}
	
	/** Change the window layout back to the default */
	private void useDefaultPerformed(ActionEvent e)
	{
		m_Frame.useDefaultLayout() ;
	}
	
	private void useTextLayout(ActionEvent e)
	{
		m_Frame.loadLayoutFile("default-text-" + Document.kVersion + ".dlf", true) ;
	}
}
