/********************************************************************************************
*
* DebuggerCommands.java
* 
* Description:	
* 
* Created on 	Mar 19, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package doc;

import general.ElementXML;
import manager.MainWindow;
import manager.Pane;
import modules.AbstractView;
import modules.TraceView;
import debugger.MainFrame;
import dialogs.NewWindowDialog;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;

/************************************************************************
 * 
 * A set of commands that produce actions within the debugger
 * (e.g. closing a window or operating a menu item).
 * 
 * This allows us to script the debugger from within modules and
 * do pretty arbitrary code actions using a simple sort of scripting.
 * 
 ************************************************************************/
public class DebuggerCommands
{
	protected MainFrame m_Frame ;
	protected Document	m_Document ;
	
	public DebuggerCommands(MainFrame frame, Document doc)
	{
		m_Frame = frame ;
		m_Document = doc ;
	}
	
	// Add a new view: addview <framename> <viewname> Right|Left|Top|Bottom
	protected Object executeAddView(String[] tokens)
	{
		String frameName = tokens[1] ;
		String viewName  = tokens[2] ;
		String direction = tokens[3] ;
					
		// To the user we are adding a view, but internally we're adding a pane
		// and then that pane will contain the view
		MainFrame frame = m_Document.getFrameByName(frameName) ;
		AbstractView view = frame.getNameRegister().getView(viewName) ;
		Pane pane = view.getPane() ;

		// Ask for the type of window to add
		Module module = NewWindowDialog.showDialog(frame, "Select new window", m_Document.getModuleList()) ;
		if (module == null)
			return null ;
		
		// Create an instance of the class.  This view won't be fully instantiated
		// but we'll use it to create a name and generate XML etc.
		AbstractView newView = module.createInstance() ;
		
		if (newView == null)
			return null ;

		// Fixed sized views are hosted in composite windows
		// Variable sized views (the norm) are hosted in sash forms
		boolean addSash = !newView.isFixedSizeView() ;

		// The plan is to convert the current set of windows to XML
		// then modify that tree structure and then load the XML back in.
		// (We do this because SWT requires us to rebuild all of the windows from
		//  the point of the change down, so we may as well rebuild everything since we
		//  already have all of that code and it's both fairly complex and heavily used).
		ElementXML xml = frame.getMainWindow().convertToXML() ;
		
		// Find the XML element for the pane we're modifying
		ElementXML existingPane = pane.getElementXML() ;

		ElementXML parent = existingPane.getParent() ;

		// If the parent window is a composite then it's children have
		// fixed locations and we treat them as a unit, so we move up
		// another level.
		while (parent.getTagName().equals("composite"))
		{
			existingPane = parent ;
			parent = parent.getParent() ;
		}

		// If we're adding to the left side we assume the orientation is vertical
		// If we're adding to the top we assume the orientation is horizontal
		boolean horiz = ((direction.equals(MainWindow.kAttachTopValue) || direction.equals(MainWindow.kAttachBottomValue))) ;

		// We create a new pane but with no parent window (so the pane object is not instantiated
		// into SWT windows).  We just want to use it to generate the new XML.
		Pane newPane = new Pane(true) ;
		newPane.setHorizontalOrientation(horiz) ;
		
		// Generate a name and make this view a child of the pane we're building
		newView.generateName(frame) ;
		newPane.addView(newView) ;

		// Create XML for the new sash form with appropriate children
		ElementXML newChild1 = existingPane ;
		ElementXML newChild2 = newPane.convertToXML(Pane.kTagName) ;

		ElementXML newParent = null ;
		
		if (addSash)
		{
			// Create a new XML subtree for a new sash form
			int orientation = (direction.equals(MainWindow.kAttachLeftValue) || direction.equals(MainWindow.kAttachRightValue)) ? SWT.HORIZONTAL : SWT.VERTICAL ;				
			int[] weights = new int[] { 50, 50 } ;
			newParent = frame.getMainWindow().buildXMLForSashForm(orientation, weights) ;

			// The order to add the children depends on which side
			// we're adding to.
			if (direction.equals(MainWindow.kAttachRightValue) || direction.equals(MainWindow.kAttachBottomValue))
			{
				newParent.addChildElement(newChild1) ;
				newParent.addChildElement(newChild2) ;
			}
			else
			{
				newParent.addChildElement(newChild2) ;
				newParent.addChildElement(newChild1) ;				
			}
		}
		else
		{
			// Create a new XML subtree for a composite pair
			newParent = frame.getMainWindow().buildXMLforComposite(direction) ;
			
			newParent.addChildElement(newChild1) ;
			newParent.addChildElement(newChild2) ;
		}
		
		
		// Change the XML tree to insert this new sash form.			
		boolean success = parent.replaceChild(existingPane, newParent) ;
		
		if (!success)
			throw new IllegalStateException("Error in replacing panes") ;
			
		try
		{
			// Rebuild the entire layout from the new XML structure.
			frame.getMainWindow().loadFromXML(xml) ;
			
			// Save immediately, so if the debugger crashes the changes are kept
			frame.saveCurrentLayoutFile() ;		
			
		} catch (Exception e)
		{
			// Fatal error
			e.printStackTrace();
		}
		
		return null ;		
	}
	
	// Remove an existing view: removeview <framename> <viewname>
	protected Object executeRemoveView(String[] tokens)
	{
		String frameName = tokens[1] ;
		String viewName  = tokens[2] ;

		MainFrame frame = m_Document.getFrameByName(frameName) ;
		AbstractView view = frame.getNameRegister().getView(viewName) ;
		Pane pane = view.getPane() ;

		// Convert everything to XML
		ElementXML xml = frame.getMainWindow().convertToXML() ;

		// Find the XML element for the pane we're removing
		ElementXML removePane = pane.getElementXML() ;
		
		// If we're removing a fixed size view we want to just remove the parent
		// composite to leave just the other pane in the pair.
		// If we're removing a variable size view and its part of a composite
		// we delete the other (fixed size) pane as well which can lead to a chain.
		boolean removingFixedSize = view.isFixedSizeView() ;
		
		if (removingFixedSize)
		{
			// With a fixed size window we just promote the other side of the pair
			// up a level in the hierarchy.
			ElementXML parent = removePane.getParent() ;
			
			if (parent.getNumberChildren() != 2)
				throw new IllegalStateException("The parent of a fixed size window should be a composite pair") ;
			
			// Find the other pane of the pair
			ElementXML otherPane = parent.getChild(0) ;
			if (otherPane == removePane)
				otherPane = parent.getChild(1) ;
			
			// Promote the other pane to the position of the current parent
			ElementXML superParent = parent.getParent() ;
			superParent.replaceChild(parent, otherPane) ;
		}
		else
		{
			// With a variable sized window, if we're removing it from a pair
			// (with a fixed sized window) we need to remove the pair.
			// This can lead to a chain reaction up the hierarchy.
			boolean done = false ;

			while (!done)
			{
				ElementXML parent = removePane.getParent() ;
				
				// Can't remove the children of the top window as the top
				// window is just a container that hosts everything else
				// (i.e. it only has one child)
				if (parent == null)
				{
					frame.ShowMessageBox("Can't remove the last window") ;
					return null ;
				}
				
				parent.removeChild(removePane) ;

				// If the parent is a composite, remove it and its twin for now
				if (parent.getTagName().equals("composite"))
				{
					removePane = parent ;
				}
				else if (parent.getNumberChildren() > 0)
				{
					// Note: For sashform's this actually leaves the XML in an invalid state with
					// incorrect weight information (to match the wrong number of children).
					// Fixed that by allowing the load logic to recover from that state and just
					// default to equal weights and the result seems fine.
					done = true ;
				}
				
				removePane = parent ;
			}
		}
		
		try
		{
			// Rebuild the entire layout from the new XML structure.
			frame.getMainWindow().loadFromXML(xml) ;
			
			// Save immediately, so if the debugger crashes the changes are kept
			frame.saveCurrentLayoutFile() ;
			
		} catch (Exception e)
		{
			// Fatal error
			e.printStackTrace();
		}

		return null ;		
	}
	
	// Return value is currently null, but we'll allow for it to return values in the future.
	public Object executeCommand(String command, boolean echoCommand)
	{
		if (command == null)
			return null ;
		
		String[] tokens = command.split(" ") ;
		
		if (tokens.length == 0)
			return null ;
		
		String first = tokens[0] ;

		// Load a demo: "demo <folder-relative-to-m_DemoPath> <demo.soar>"
		if (first.equals("demo"))
		{
			m_Frame.loadDemo(new java.io.File(tokens[1], tokens[2]), echoCommand) ;
			return null ;
		}
		
		if (first.equals("removeview"))
		{
			return executeRemoveView(tokens) ;
		}

		if (first.equals("addview"))
		{
			return executeAddView(tokens) ;
		}
		
		// properties <frame> <view>
		if (first.equals("properties"))
		{
			String frameName = tokens[1] ;
			String viewName  = tokens[2] ;

			MainFrame frame = m_Document.getFrameByName(frameName) ;
			AbstractView view = frame.getNameRegister().getView(viewName) ;
			
			view.showProperties() ;
		}
		
		return null ;
	}
}
