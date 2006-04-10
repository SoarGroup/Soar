/********************************************************************************************
*
* BaseMenu.java
* 
* Description:	
* 
* Created on 	Feb 15, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package menu;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.widgets.*;

/************************************************************************
 * 
 * A wrapper around the particular Menu class we're using.
 * 
 * I'm including this to allow me to switch over from Swing Menus to SWT Menus
 * more easily without re-writing the existing code.  Abstracting this logic
 * is probably a good idea anyway.
 *
 * 
 ************************************************************************/
public class BaseMenu
{
	// The SWT menu object
	private Menu m_Menu ;
	private MenuItem m_Header ;

	public BaseMenu(Menu bar, String title, final MenuUpdater updater)
	{
		m_Header = new MenuItem(bar, SWT.CASCADE) ;
		m_Header.setText(title) ;
		
		m_Menu = new Menu(bar.getShell(), SWT.DROP_DOWN) ;
		m_Header.setMenu(m_Menu) ;
				
		m_Header.addArmListener(new ArmListener() {
			public void widgetArmed(ArmEvent e) {
				
				// Give the menu a chance to change the state of the action objects.
				// We could extend this to be done on each action object in turn as well.
				if (updater != null)
					updater.updateItems() ;
				
				for (int i = 0 ; i < m_Menu.getItemCount() ; i++)
				{
					MenuItem item = m_Menu.getItem(i) ;
					AbstractAction action = (AbstractAction)item.getData() ;
					
					if (action != null)
						item.setEnabled(action.isEnabled()) ;
				}
			}
		} );
	}

	public BaseMenu(Menu bar, String title)
	{
		this(bar, title, null) ;
	}
	
	public Menu getMenu() { return m_Menu ; }
	
	public MenuItem addSeparator()
	{
		MenuItem item = new MenuItem(m_Menu, SWT.SEPARATOR) ;
		return item ;
	}
	
	public BaseMenu addSubmenu(String name)
	{
	   BaseMenu subMenu = new BaseMenu(m_Menu, name) ;
	   return subMenu ;
	}
	
	public MenuItem add(final AbstractAction action)
	{
		MenuItem item = new MenuItem(m_Menu, SWT.PUSH) ;
		item.setText(action.getLabel()) ;
		item.setData(action) ;
		
		// When the SWT selection occurs we fire that through the
		// actionPerformed method of the action object
		// (This provides a generic way to call this sort of action
		//  so we could also add the actions to a toolbar and not
		//  repeat all of the logic)
		item.addSelectionListener(new SelectionAdapter () {
			public void widgetSelected(SelectionEvent e) {
				action.actionPerformed(null) ;
			}
		});
		
		return item ;
	}
	
	public MenuItem add(AbstractAction action, int accelerator)
	{
		MenuItem item = add(action) ;
		item.setAccelerator(accelerator) ;
		return item ;
	}

	public MenuItem addCheckedItem(final AbstractAction action, boolean checkedInitially)
	{
		action.setChecked(checkedInitially, false) ;
		
		final MenuItem item = new MenuItem(m_Menu, SWT.CHECK) ;
		item.setText(action.getLabel()) ;
		item.setData(action) ;
		item.setSelection(checkedInitially) ;
		
		action.setMenuItem(item) ;
		
		// When the SWT selection occurs we fire that through the
		// actionPerformed method of the action object
		// (This provides a generic way to call this sort of action
		//  so we could also add the actions to a toolbar and not
		//  repeat all of the logic)
		item.addSelectionListener(new SelectionAdapter () {
			public void widgetSelected(SelectionEvent e) {
				action.setChecked(!action.isChecked(), false) ;
				item.setSelection(action.isChecked()) ;
				action.actionPerformed(null) ;
			}
		});
		
		return item ;
	}
}
