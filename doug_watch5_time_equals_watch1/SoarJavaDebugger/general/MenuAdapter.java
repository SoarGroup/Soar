/********************************************************************************************
*
* MenuAdapter.java
* 
* Created on 	Nov 7, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package general;

import javax.swing.event.*;

/********************************************************************************************
* 
* Simple adapter class for menu events.
* 
********************************************************************************************/
public class MenuAdapter implements MenuListener
{
	public void menuCanceled(MenuEvent e) {}
	public void menuDeselected(MenuEvent e) {}
	public void menuSelected(MenuEvent e) {}
}
