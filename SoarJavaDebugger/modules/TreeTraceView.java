/********************************************************************************************
*
* TreeTraceView.java
* 
* Description:	Represents trace output using a tree, so we can capture trace information
* 				at one level of detail and display a lesser amount of detail.
* 
* Created on 	Mar 29, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import java.io.File;

import general.ElementXML;
import helpers.CommandHistory;
import manager.Pane;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.DisposeEvent;
import org.eclipse.swt.events.DisposeListener;
import org.eclipse.swt.events.KeyAdapter;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.MouseAdapter;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.widgets.*;

import sml.Agent;
import debugger.MainFrame;
import doc.Document;

/************************************************************************
 * 
 * Represents trace output using a tree, so we can capture trace information
 * at one level of detail and display a lesser amount of detail.
 * 
 ************************************************************************/
public class TreeTraceView extends ComboCommandBase
{
	protected Tree m_Tree ;

	/********************************************************************************************
	* 
	* Create the window that will display the output
	* 
	********************************************************************************************/
	protected void createDisplayControl(Composite parent)
	{
		m_Tree = new Tree(parent, SWT.BORDER) ;
		
		Tree tree = m_Tree ;
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
	}

	/********************************************************************************************
	* 
	* This "base name" is used to generate a unique name for the window.
	* For example, returning a base name of "trace" would lead to windows named
	* "trace1", "trace2" etc.
	* 
	********************************************************************************************/
	public String getModuleBaseName()
	{
		return "treetrace" ;
	}

	/** The control we're using to display the output in this case **/
	protected Control getDisplayControl()
	{
		return m_Tree ;
	}

	/************************************************************************
	* 
	* Add the text to the view (needs to be done in a thread safe way)
	* 
	*************************************************************************/
	protected void appendText(String text)
	{
		
	}

	/************************************************************************
	* 
	* Clear the display control.
	* 
	*************************************************************************/
	protected void clearDisplay()
	{
		
	}
	
	protected void storeContent(ElementXML element)
	{
		
	}

	protected void restoreContent(ElementXML element)
	{
		
	}

	/********************************************************************************************
	 * 
	 * Display a dialog that allows the user to adjust properties for this window
	 * e.g. choosing whether to clear the window everytime a new command executes or not.
	 * 
	********************************************************************************************/
	public void showProperties()
	{
	}
}
