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
import doc.Document;

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
	
	private AbstractAction m_ChooseFont = new AbstractAction("Choose text font") { public void actionPerformed(ActionEvent e) { chooseFontPerformed(e) ; } } ;

	/** Create this menu */
	public static EditMenu createMenu(MainFrame frame, Document doc, String title, char mnemonicChar)
	{
		EditMenu menu = new EditMenu() ;
		menu.m_Frame    = frame ;
		menu.m_Document = doc ;
		
		menu.m_Menu = menu.makeMenu(frame.getMenuBar(), title, mnemonicChar) ;
		
		return menu ;
	}
	
	private BaseMenu makeMenu(Menu parent, String title, char mnemonicChar)
	{
		BaseMenu menu = new BaseMenu(parent, title, mnemonicChar) ;
		
		menu.add(m_ChooseFont) ;
		
		return menu ;
	}
	
	private void chooseFontPerformed(ActionEvent e)
	{
		FontData data = m_Frame.ShowFontDialog() ;
		
		if (data != null)
			m_Frame.setTextFont(data) ;
	}
}
