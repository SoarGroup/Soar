/********************************************************************************************
*
* SWTApplication.java
* 
* Description:	
* 
* Created on 	Mar 2, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package debugger;

import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.*;
import org.eclipse.swt.custom.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.program.*;
import org.eclipse.swt.graphics.*;

import java.io.* ;

import sml.Agent;
import sml.Kernel;
import doc.Document;

/************************************************************************
 * 
 * SWT based application (we used to have a Swing version too)
 * 
 ************************************************************************/
public class SWTApplication
{
	private Document m_Document = null ;

	static Display display;
	static Shell shell;
	static CoolBar coolBar;
	static Menu chevronMenu = null;
	
	// This is just a little place for me to easily test SWT concepts
	public void startTest() throws Exception
	{
		display = new Display ();
		shell = new Shell (display);
		shell.setLayout(new GridLayout());
		coolBar = new CoolBar(shell, SWT.FLAT | SWT.BORDER);
		coolBar.setLayoutData(new GridData(GridData.FILL_BOTH));
		ToolBar toolBar = new ToolBar(coolBar, SWT.FLAT | SWT.WRAP);
		int minWidth = 0;
		for (int j = 0; j < 5; j++) {
			int width = 0;
			ToolItem item = new ToolItem(toolBar, SWT.PUSH);
			item.setText("B" + j);
			width = item.getWidth();
			/* find the width of the widest tool */
			if (width > minWidth) minWidth = width;
		}
		CoolItem coolItem = new CoolItem(coolBar, SWT.DROP_DOWN);
		coolItem.setControl(toolBar);
		Point size = toolBar.computeSize(SWT.DEFAULT, SWT.DEFAULT);
		Point coolSize = coolItem.computeSize (size.x, size.y);
		coolItem.setMinimumSize(minWidth, coolSize.y);
		coolItem.setPreferredSize(coolSize);
		coolItem.setSize(coolSize);
		coolItem.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent event) {
				if (event.detail == SWT.ARROW) {
					CoolItem item = (CoolItem) event.widget;
					Rectangle itemBounds = item.getBounds ();
					Point pt = coolBar.toDisplay(new Point(itemBounds.x, itemBounds.y));
					itemBounds.x = pt.x;
					itemBounds.y = pt.y;
					ToolBar toolBar = (ToolBar) item.getControl ();
					ToolItem[] tools = toolBar.getItems ();
					
					int i = 0;
					while (i < tools.length) {
						Rectangle toolBounds = tools[i].getBounds ();
						pt = toolBar.toDisplay(new Point(toolBounds.x, toolBounds.y));
						toolBounds.x = pt.x;
						toolBounds.y = pt.y;
						
						/* Figure out the visible portion of the tool by looking at the
						 * intersection of the tool bounds with the cool item bounds. */
				  		Rectangle intersection = itemBounds.intersection (toolBounds);
				  		
						/* If the tool is not completely within the cool item bounds, then it
						 * is partially hidden, and all remaining tools are completely hidden. */
				  		if (!intersection.equals (toolBounds)) break;
				  		i++;
					}
					
					/* Create a menu with items for each of the completely hidden buttons. */
					if (chevronMenu != null) chevronMenu.dispose();
					chevronMenu = new Menu (coolBar);
					for (int j = i; j < tools.length; j++) {
						MenuItem menuItem = new MenuItem (chevronMenu, SWT.PUSH);
						menuItem.setText (tools[j].getText());
					}
					
					/* Drop down the menu below the chevron, with the left edges aligned. */
					pt = coolBar.toDisplay(new Point(event.x, event.y));
					chevronMenu.setLocation (pt.x, pt.y);
					chevronMenu.setVisible (true);
				}
			}
		});
		
		shell.pack();
		shell.open ();
		while (!shell.isDisposed ()) {
			if (!display.readAndDispatch ()) display.sleep ();
		}
		display.dispose ();
	}
	
	public void startApp() throws Exception
	{
		Display display = new Display() ;
		Shell shell = new Shell(display) ;
		
		// The document manages the Soar process
		m_Document = new Document() ;
		
		MainFrame frame = new MainFrame(shell, m_Document) ;
		frame.initComponents();
		
		// We wait until we have a frame up before starting the kernel
		// so it's just as if the user chose to do this manually
		// (saves some special case logic in the frame)
		Agent agent = m_Document.startLocalKernel(Kernel.GetDefaultPort()) ;
		frame.setAgentFocus(agent) ;	
		
		shell.open() ;
		
		m_Document.pumpMessagesTillClosed(display) ;

		display.dispose() ;
	}

}
