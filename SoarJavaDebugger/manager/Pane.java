/********************************************************************************************
*
* Pane.java
* 
* Description:	
* 
* Created on 	Feb 16, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package manager;

import modules.* ;
import java.util.*;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.events.*;

/************************************************************************
 * 
 * A pane represents a sub-window in the MainWindow.
 * 
 * A module is attached to a pane which gives it is content (e.g. a text window
 * for typing commands and receiving output or a tree control for viewing state).
 * 
 * Thus a pane is simply a place holder.  It can manage a single module
 * or a list of modules (through a tabbed window).
 * 
 ************************************************************************/
public class Pane
{
	// List of views attached to this pane
	// Each view represents a distinct module and only one is visible at a time (> 1 => we use some form of tab)
	private ArrayList	m_Views = new ArrayList() ;
	private Composite	m_Pane ;
	
	public Pane(Composite parent)
	{
		// We provide a border around the pane so we can find the edges to drag
		m_Pane = new Group(parent, SWT.SHADOW_ETCHED_IN) ;
		m_Pane.setLayout(new FillLayout(SWT.VERTICAL)) ;
	}
	
	public Composite getWindow()
	{
		return m_Pane ;
	}
	
	
}
