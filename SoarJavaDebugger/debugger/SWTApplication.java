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
		Display display = new Display() ;
		Shell shell = new Shell(display) ;

		RowLayout layout = new RowLayout(SWT.VERTICAL) ;
		layout.fill = true ;
		shell.setLayout(layout) ;
		
		Composite parent = shell ;
		
		new Button(parent, SWT.PUSH).setText("one") ;
		new Button(parent, SWT.PUSH).setText("two") ;
		new Button(parent, SWT.PUSH).setText("third button") ;
		new Button(parent, SWT.PUSH).setText("four") ;
		
		shell.open() ;

		while (!shell.isDisposed())
		{
			if (!display.readAndDispatch())
			{
				display.sleep() ;
			}
		}

		display.dispose() ;
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
