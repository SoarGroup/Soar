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
import org.eclipse.swt.custom.TableEditor;
import org.eclipse.swt.events.*;
import org.eclipse.swt.program.*;
import org.eclipse.swt.graphics.*;

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
	    Display display = new Display ();
	    Shell shell = new Shell (display);
	    Label label = new Label (shell, SWT.NONE);
	    label.setText ("Can't find icon for .txt");
	    Image image = null;
	    Program p = Program.findProgram (".txt");
	    if (p != null) {
	        ImageData data = p.getImageData ();
	        if (data != null) {
	            image = new Image (display, data);
	            label.setImage (image);
	        }
	    }
	    Color white = new Color(display, 255,255,255);
	    Tree tree = new Tree(shell, SWT.NONE);
	    tree.setBackground(white);
	    if (image != null) {	    	
	    	TreeItem item = new TreeItem(tree, SWT.NONE);
	        item.setBackground(white);
	        item.setImage(image);
	        item.setText("?");
	    }
	    label.setBackground(white);
	    label.pack ();
	    tree.pack();
	    shell.pack ();
	    shell.open ();
	    while (!shell.isDisposed()) {
	        if (!display.readAndDispatch ()) display.sleep ();
	    }
	    white.dispose();
	    if (image != null) image.dispose ();
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
