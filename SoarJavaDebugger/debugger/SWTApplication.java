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

	// This is just a little place for me to easily test SWT concepts
	public void startTest() throws Exception
	{
		final Display display = new Display ();
		final Shell shell = new Shell (display);
		shell.setText ("Lazy Tree");
		shell.setLayout (new FillLayout ());
		final Tree tree = new Tree (shell, SWT.BORDER);
		File [] roots = File.listRoots ();
		for (int i=0; i<roots.length; i++) {
			TreeItem root = new TreeItem (tree, 0);
			root.setText (roots [i].toString ());
			root.setData (roots [i]);
			new TreeItem (root, 0);
		}
		tree.addListener (SWT.Expand, new Listener () {
			public void handleEvent (final Event event) {
				final TreeItem root = (TreeItem) event.item;
				TreeItem [] items = root.getItems ();
				for (int i= 0; i<items.length; i++) {
					if (items [i].getData () != null) return;
					items [i].dispose ();
				}
				File file = (File) root.getData ();
				File [] files = file.listFiles ();
				if (files == null) return;
				for (int i= 0; i<files.length; i++) {
					TreeItem item = new TreeItem (root, 0);
					item.setText (files [i].getName ());
					item.setData (files [i]);
					if (files [i].isDirectory()) {
						new TreeItem (item, 0);
					}
				}
			}
		});
		Point size = tree.computeSize (300, SWT.DEFAULT);
		int width = Math.max (300, size.x);
		int height = Math.max (300, size.y);
		shell.setSize (shell.computeSize (width, height));
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
