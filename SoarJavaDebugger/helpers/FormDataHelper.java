/********************************************************************************************
*
* FormDataHelper.java
* 
* Description:	
* 
* Created on 	Feb 16, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package helpers;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;

/************************************************************************
 * 
 * Create common FormData objects in one line.
 * 
 * (SWT may make this easy but I've not found out how yet).
 * 
 ************************************************************************/
public class FormDataHelper
{
	// Anchor to all sides
	static public FormData anchorFull(int offset)
	{
		FormData data = new FormData() ;
		
		data.left = new FormAttachment(0, offset) ;
		data.right = new FormAttachment(100, -offset) ;
		data.top = new FormAttachment(0, offset) ;
		data.bottom = new FormAttachment(100, -offset) ;
		
		return data ;
	}

	// Anchor to the bottom of the form
	static public FormData anchorBottom(int offset)
	{
		FormData data = new FormData() ;
		
		data.left = new FormAttachment(0, offset) ;
		data.right = new FormAttachment(100, -offset) ;
		data.bottom = new FormAttachment(100, -offset) ;
		
		return data ;
	}

	// Anchor to the top of the form
	static public FormData anchorTop(int offset)
	{
		FormData data = new FormData() ;
		
		data.left = new FormAttachment(0, offset) ;
		data.right = new FormAttachment(100, -offset) ;
		data.top = new FormAttachment(0, offset) ;
		
		return data ;
	}

	// Anchor to the top-left of the form
	static public FormData anchorTopLeft(int offset)
	{
		FormData data = new FormData() ;
		
		data.left = new FormAttachment(0, offset) ;
		data.top  = new FormAttachment(0, offset) ;
		
		return data ;
	}

	// Anchor to the top-left of the form
	static public FormData anchorBottomLeft(int offset)
	{
		FormData data = new FormData() ;
		
		data.left    = new FormAttachment(0, offset) ;
		data.bottom  = new FormAttachment(100, -offset) ;
		
		return data ;
	}

	// Anchor to the top-left of the form
	static public FormData anchorBottomRight(int offset)
	{
		FormData data = new FormData() ;
		
		data.right  = new FormAttachment(100, -offset) ;
		data.bottom = new FormAttachment(100, -offset) ;
		
		return data ;
	}

}
