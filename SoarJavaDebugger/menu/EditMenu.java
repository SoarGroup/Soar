/********************************************************************************************
*
* EditMenu.java
* 
* Created on 	Nov 9, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package menu;

import general.* ;
import debugger.* ;
import dialogs.SearchDialog;
import doc.Document;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.graphics.FontData ;

/********************************************************************************************
* 
* The edit menu
* 
********************************************************************************************/
public class EditMenu
{
	private BaseMenu  m_Menu ;

	private MainFrame m_Frame = null ;
	private Document  m_Document = null ;
	private SearchDialog m_SearchDialog = null ;
	
	private AbstractAction m_Search = new AbstractAction("&Find...\tCtrl+F")  		  { public void actionPerformed(ActionEvent e) { searchPrime() ; } } ;
	private AbstractAction m_ChooseFont = new AbstractAction("&Choose text font...")  { public void actionPerformed(ActionEvent e) { chooseFontPerformed(e) ; } } ;
	private AbstractAction m_DefaultFont = new AbstractAction("Use &default font") 	  { public void actionPerformed(ActionEvent e) { useDefaultFont(e) ; } } ;

	/** Create this menu */
	public static EditMenu createMenu(MainFrame frame, Document doc, String title)
	{
		EditMenu menu = new EditMenu() ;
		menu.m_Frame    = frame ;
		menu.m_Document = doc ;
		
		menu.m_Menu = menu.makeMenu(frame.getMenuBar(), title) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title)
	{
		BaseMenu menu = new BaseMenu(parent, title) ;
		
		menu.add(m_Search, SWT.CTRL + 'F') ;
		menu.addSeparator() ;
		menu.add(m_ChooseFont) ;
		menu.add(m_DefaultFont) ;
		
		return menu ;
	}

	public void searchPrime()
	{
		if (m_SearchDialog != null && !m_SearchDialog.isDisposed())
		{
			// If the search dialog is up and we get another request
			// to bring it up (probably a Ctrl-F typed) then do a find.
			m_SearchDialog.find() ;
			return ;
		}
		
		m_SearchDialog = SearchDialog.showDialog(m_Frame, "Search for text", m_Frame.getPrimeView()) ;
	}
	
	private void useDefaultFont(ActionEvent e)
	{
  		m_Frame.setTextFont(m_Frame.kDefaultFontData) ;  		
	}

	private void chooseFontPerformed(ActionEvent e)
	{
		FontData data = m_Frame.ShowFontDialog() ;
		
		if (data != null)
			m_Frame.setTextFont(data) ;
	}
}
