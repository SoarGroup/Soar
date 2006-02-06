/********************************************************************************************
*
* SwtSplitPane.java
* 
* Description:	
* 
* Created on 	Feb 15, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package helpers;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.*;

/************************************************************************
 * 
 * A pane which owns a pair of child widgets and places a sash between them
 * thus creating something close to a JSplitPane for SWT.
 * 
 * We use it by creating the pane and then creating children 
 * 
 ************************************************************************/
public class SwtSplitPane
{
	private int m_Orientation = SWT.HORIZONTAL ;
	private Group		m_Pane ;
	private FormLayout	m_Layout ;
	private Listener 	m_SashListener;
	private Control  	m_Left ;
	private Control	 	m_Right ;
	private Sash	 	m_Sash ;
	private double 	 	m_Position ;	// 0.0 is left/top ; 1.0 is bottom/right

	private static int checkStyle (int style) {
		int mask = SWT.BORDER | SWT.LEFT_TO_RIGHT | SWT.RIGHT_TO_LEFT;
		return style & mask;
	}

	public SwtSplitPane(Composite parent, int style)
	{
		m_Pane = new Group(parent, 0) ;
		m_Layout = new FormLayout() ;
		m_Pane.setLayout(m_Layout) ;
		
		if ((style & SWT.VERTICAL) != 0){
			m_Orientation = SWT.VERTICAL;
		}
		
		m_Pane.addListener(SWT.Resize, new Listener() {
			public void handleEvent(Event e) {
				layoutControls() ;
			}
		});
		
		m_SashListener = new Listener() {
			public void handleEvent(Event e) {
				onDragSash(e);
			}
		};

		// The sash uses the inverse orientation
		m_Sash  = new Sash(m_Pane, m_Orientation == SWT.VERTICAL ? SWT.HORIZONTAL : SWT.VERTICAL) ;
		
		m_Left = null ;
		m_Right = null ;
		m_Sash.addListener(SWT.Selection, m_SashListener) ;
		m_Position = 0.5 ;
	}

	// I'm not deriving this class from an SWT class yet.
	// Instead I expose the control it represents.  Gives me more control while I'm learning.
	public Composite getControl()
	{
		return m_Pane ;
	}

	// Programmatically control the divider
	public void setDividerLocation(double proportion)
	{
		m_Position = proportion ;
		layoutControls() ;
	}
	
	public double getDividerLocation()
	{
		return m_Position ;
	}
	
	// The control should have m_Pane as their parent
	public void setControls(Control leftOrTop, Control rightOrBottom)
	{
		if (leftOrTop.getParent() != m_Pane)
			throw new IllegalArgumentException("We need the controls to be owned by this panel") ;
		if (rightOrBottom.getParent() != m_Pane)
			throw new IllegalArgumentException("We need the controls to be owned by this panel") ;

		
		m_Left  = leftOrTop ;
		m_Right = rightOrBottom ;
				
    	FormData leftData = new FormData();
    	FormData rightData = new FormData();

    	// Build form logic that places the sash in the center of the two
    	// controls and ensures that when it moves they will too.
    	if (m_Orientation == SWT.HORIZONTAL)
    	{
	      	leftData.left      = new FormAttachment(0);
	    	leftData.right     = new FormAttachment(m_Sash);
	    	leftData.top       = new FormAttachment(0);
	      	leftData.bottom    = new FormAttachment(100);
	             
	      	rightData.left   = new FormAttachment(m_Sash);
	    	rightData.right  = new FormAttachment(100);
	    	rightData.top    = new FormAttachment(0);
	    	rightData.bottom = new FormAttachment(100);
    	}
    	else
    	{
	      	leftData.left      = new FormAttachment(0);
	    	leftData.right     = new FormAttachment(100);
	    	leftData.top       = new FormAttachment(0);
	      	leftData.bottom    = new FormAttachment(m_Sash);
	             
	      	rightData.left   = new FormAttachment(0);
	    	rightData.right  = new FormAttachment(100);
	    	rightData.top    = new FormAttachment(m_Sash);
	    	rightData.bottom = new FormAttachment(100);    		
    	}
    	m_Left.setLayoutData(leftData);
    	m_Right.setLayoutData(rightData);
		
		layoutControls() ;
	}
	
	public void layoutControls()
	{
		// Check if we're fully formed yet
		if (m_Left == null || m_Right == null)
			return ;
		
	 	FormData sashData = new FormData();
	 	
	 	// We update the form data to move the sash's position
	 	// The other controls are bound to it so when it moves
	 	// they will resize.
		if (m_Orientation == SWT.HORIZONTAL)
		{
			sashData.top 	  = new FormAttachment(0) ;
           	sashData.bottom   = new FormAttachment(100) ;
           	sashData.left	  = new FormAttachment( (int)(m_Position * 100)) ;
		}
		else
		{
			sashData.left 	  = new FormAttachment(0) ;
           	sashData.right    = new FormAttachment(100) ;
           	sashData.top	  = new FormAttachment( (int)(m_Position * 100)) ;	
		}
		
       	m_Sash.setLayoutData(sashData) ;
       	
       	// Make the control update
       	m_Pane.layout() ;
	}
	
	void onDragSash(Event event)
	{
		Rectangle area = m_Pane.getClientArea() ;
		
		if (m_Orientation == SWT.HORIZONTAL)
		{
			m_Position = ((double)event.x) / (double)(area.width) ;
		}
		else
		{
			m_Position = ((double)event.y) / (double)(area.height) ;
		}

		// Make sure we're in range
		if (m_Position < 0.0) m_Position = 0.0 ;
		if (m_Position > 1.0) m_Position = 1.0 ;
		
		layoutControls() ;
	}

}

