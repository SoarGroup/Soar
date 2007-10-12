/********************************************************************************************
*
* TextFieldAdapter.java
* 
* Created on 	Nov 9, 2003
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
* Makes attaching a document adapter to a text field simpler, when you just care that
* it's changed, not how it has changed.
* 
********************************************************************************************/
public abstract class TextFieldListener implements DocumentListener
{	
	public abstract void textUpdate(DocumentEvent e) ;
	
	public void insertUpdate(DocumentEvent e)
	{
		textUpdate(e) ;
	}
	
	public void removeUpdate(DocumentEvent e)
	{
		textUpdate(e) ;
	}
	
	public void changedUpdate(DocumentEvent e)
	{
		textUpdate(e) ;
	}
}
