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

import general.JavaElementXML;
import manager.MainWindow;
import manager.Pane;
import modules.AbstractView;
import debugger.MainFrame;
import dialogs.NewWindowDialog;

import org.eclipse.swt.*;

import sml.Kernel;

/************************************************************************
 * 
 * A set of commands that produce actions within the debugger
 * (e.g. closing a window or operating a menu item).
 * 
 * This allows us to script the debugger from within modules and
 * do pretty arbitrary code actions using a simple sort of scripting.
 * 
 ************************************************************************/
public class ScriptCommands
{
	protected MainFrame m_Frame ;
	protected Document	m_Document ;
	
	public ScriptCommands(MainFrame frame, Document doc)
	{
		m_Frame = frame ;
		m_Document = doc ;
	}
		
	// Remove an existing view: clear <framename> <viewname>
	protected Object executeClearView(String[] tokens)
	{
		String frameName = tokens[1] ;
		String viewName  = tokens[2] ;

		MainFrame frame = m_Document.getFrameByName(frameName) ;
		AbstractView view = frame.getView(viewName) ;

		view.clearDisplay() ;
		
		return null ;
	}
	
	// Make connection to remote kernel
	// remote <ip> <port> <agentname> [all arguments optional]
	protected Object executeRemoteConnect(String[] tokens)
	{
		String ip        = tokens.length >= 2 ? tokens[1] : null ;
		String portStr   = tokens.length >= 3 ? tokens[2] : Integer.toString(Kernel.GetDefaultPort()) ;
		String agentName = tokens.length >= 4 ? tokens[3] : null ;
		
		int port = Integer.parseInt(portStr) ;
		
		// If we already have a remote connection don't do it again.
		if (m_Document.isRemote() || m_Document.isStopping())
			return Boolean.FALSE ;
		
		try
		{
			m_Document.remoteConnect(ip, port, agentName) ;
			return Boolean.TRUE ;
		}
		catch (Exception ex)
		{
			m_Frame.ShowMessageBox("Error making remote connection: " + ex.getMessage()) ;
			return Boolean.FALSE ;
		}
	}
	
	// [copy | paste] <framename> <viewname>
	protected Object executeCopyPaste(String[] tokens)
	{
		String command   = tokens[0] ;
		String frameName = tokens[1] ;
		String viewName  = tokens[2] ;
					
		// To the user we are adding a view, but internally we're adding a pane
		// and then that pane will contain the view
		MainFrame frame = m_Document.getFrameByName(frameName) ;
		AbstractView view = frame.getView(viewName) ;

		if (command.equalsIgnoreCase("copy"))
			view.copy() ;
		else
			view.paste() ;
		
		return Boolean.TRUE ;
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
		AbstractView view = frame.getView(viewName) ;
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
		boolean storeContent = true ;	// Capture existing output and redisplay it
		JavaElementXML xml = frame.getMainWindow().convertToXML(storeContent) ;
		
		// Find the XML element for the pane we're modifying
		JavaElementXML existingPane = pane.getElementXML() ;

		JavaElementXML parent = existingPane.getParent() ;

		// If the parent window is a composite then it's children have
		// fixed locations and we treat them as a unit, so we move up
		// another level.
		// Later adjustment is to allow two fixed windows to be added next to each other
		// so in that case we don't navigate up the tree.
		if (! (newView.isFixedSizeView() && view.isFixedSizeView()) )
		{
			while (parent.getTagName().equals("composite"))
			{
				existingPane = parent ;
				parent = parent.getParent() ;
			}
		}
		
		// If we're adding to the left side we assume the orientation is vertical
		// If we're adding to the top we assume the orientation is horizontal
		boolean horiz = ((direction.equals(MainWindow.kAttachTopValue) || direction.equals(MainWindow.kAttachBottomValue))) ;
		
		// We create a new pane but with no parent window (so the pane object is not instantiated
		// into SWT windows).  We just want to use it to generate the new XML.
		Pane newPane = new Pane("no parent") ;
		newPane.setHorizontalOrientation(horiz) ;
		
		// Generate a name and make this view a child of the pane we're building
		newView.generateName(frame) ;
		newPane.addView(newView) ;

		// Create XML for the new sash form with appropriate children
		JavaElementXML newChild1 = existingPane ;
		JavaElementXML newChild2 = newPane.convertToXML(Pane.kTagName, false) ;

		JavaElementXML newParent = null ;
		
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

	// Add a new tab to a pane: addtab <framename> <viewname>
	protected Object executeAddTab(String[] tokens)
	{
		String frameName = tokens[1] ;
		String viewName  = tokens[2] ;
					
		// To the user we are adding a view, but internally we're adding a pane
		// and then that pane will contain the view
		MainFrame frame = m_Document.getFrameByName(frameName) ;
		AbstractView view = frame.getView(viewName) ;
		Pane pane = view.getPane() ;

		// Ask for the type of window to add
		Module module = NewWindowDialog.showDialog(frame, "Select new window", m_Document.getModuleList()) ;
		if (module == null)
			return null ;
		
		// Create an instance of the class.  This view won't be fully instantiated
		// but we'll use it to create a name and generate XML etc.
		AbstractView newView = module.createInstance() ;
		newView.generateName(frame) ;
		
		if (newView == null)
			return null ;
		
		if (!view.isFixedSizeView() && newView.isFixedSizeView())
		{
			frame.ShowMessageBox("I'm sorry.  You can't add a fixed size window (like a button bar) to a window containing a variable sized one (like a text window)") ;
			return null ;
		}

		if (view.isFixedSizeView() && !newView.isFixedSizeView())
		{
			frame.ShowMessageBox("I'm sorry.  You can't add a variable sized window (like a text window) to a window containing a fixed sized one (like a button bar)") ;
			return null ;
		}
		
		// The plan is to build the XML for the current set of windows.
		// Then add in the XML for the new window into the appropriate pane
		// and then rebuild everything from the modified XML.
		// We do this dance because SWT doesn't generally allow the parent of a window
		// to be changed after it was created.
		
		boolean storeContent = true ;	// Capture existing output so we can redisplay it
		JavaElementXML xml = frame.getMainWindow().convertToXML(storeContent) ;
		
		// Find the XML element for the pane we're modifying
		JavaElementXML existingPane = pane.getElementXML() ;
		//JavaElementXML parentXML = existingPane.getParent() ;

		// Force the pane to be a multi-view
		existingPane.addAttribute(Pane.kAttributeSingleView, "false") ;
		
		// Make the new view visible
		existingPane.addAttribute(Pane.kAttributeView, newView.getName()) ;

		// Add the new view as a child of this pane
		JavaElementXML newViewXML = newView.convertToXML(AbstractView.kTagView, false) ;
		existingPane.addChildElement(newViewXML) ;
				
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
	
	// Replace an existing view: replaceview <framename> <viewname>
	protected Object executeReplaceView(String[] tokens)
	{
		String frameName = tokens[1] ;
		String viewName  = tokens[2] ;
					
		// To the user we are adding a view, but internally we're adding a pane
		// and then that pane will contain the view
		MainFrame frame = m_Document.getFrameByName(frameName) ;
		AbstractView view = frame.getView(viewName) ;
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
		
		if (view.isFixedSizeView() && !newView.isFixedSizeView())
		{
			frame.ShowMessageBox("I'm sorry.  You can't replace a fixed size window (like a button bar) with a variable sized one (like a text window)") ;
			return null ;
		}

		if (!view.isFixedSizeView() && newView.isFixedSizeView())
		{
			frame.ShowMessageBox("I'm sorry.  You can't replace a variable sized window (like a text window) with a fixed sized one (like a button bar)") ;
			return null ;
		}

		// The plan is to build the XML for the current set of windows.
		// Then swap the part of the XML for the current pane with a new pane
		// based on the new window and then rebuild everything from the modified XML.
		// We do this dance because SWT doesn't generally allow the parent of a window
		// to be changed after it was created.
		
		boolean storeContent = true ;	// Capture existing output and redisplay it
		JavaElementXML xml = frame.getMainWindow().convertToXML(storeContent) ;
		
		// Find the XML element for the pane we're modifying
		JavaElementXML existingPane = pane.getElementXML() ;
		JavaElementXML parentXML = existingPane.getParent() ;
		
		// Generate a name and make this view a child of the pane we're building
		Pane newPane = new Pane("no parent") ;
		
		// Make the orientation match (some will ignore this)
		newPane.setHorizontalOrientation(pane.isHorizontalOrientation()) ;
		
		newView.generateName(frame) ;
		newPane.addView(newView) ;
		
		JavaElementXML newPaneXML = newPane.convertToXML(Pane.kTagName, false) ;
		
		// Change the XML tree to insert this new sash form.			
		boolean success = parentXML.replaceChild(existingPane, newPaneXML) ;

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
	
	// Move tabs in a tabbed view: movetabs <framename> <viewname> <top | bottom>
	protected Object executeMoveTabs(String[] tokens)
	{
		String frameName = tokens[1] ;
		String viewName  = tokens[2] ;
		String direction = tokens[3] ;
		
		// To the user we are adding a view, but internally we're adding a pane
		// and then that pane will contain the view
		MainFrame frame = m_Document.getFrameByName(frameName) ;
		AbstractView view = frame.getView(viewName) ;
		Pane pane = view.getPane() ;
		
		boolean storeContent = true ;	// Capture existing output and redisplay it
		JavaElementXML xml = frame.getMainWindow().convertToXML(storeContent) ;
		
		// Find the XML element for the pane we're modifying
		JavaElementXML existingPane = pane.getElementXML() ;
		
		// Set the tab at top value appropriately
		boolean top = (direction.equalsIgnoreCase("top")) ;
		existingPane.addAttribute(Pane.kAttributeTabAtTop, Boolean.toString(top)) ;

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
		AbstractView view = frame.getView(viewName) ;
		Pane pane = view.getPane() ;

		// Convert everything to XML
		boolean storeContent = true ;	// Capture existing output and redisplay it
		JavaElementXML xml = frame.getMainWindow().convertToXML(storeContent) ;

		// Find the XML element for the pane we're removing
		JavaElementXML removePane = pane.getElementXML() ;
				
		// If the pane contains multiple views we just need to delete our view
		if (!pane.isSingleView())
		{
			removePane.removeChild(view.getElementXML()) ;
			
			// If this takes us down to one child we need to mark the pane
			// as a single view
			if (removePane.getNumberChildren() == 1)
			{
				removePane.addAttribute(Pane.kAttributeSingleView, "true") ;
			}
		}
		else
		{
			// At this point we're removing the entire pane
			
			// If we're removing a fixed size view we want to just remove the parent
			// composite to leave just the other pane in the pair.
			// If we're removing a variable size view and its part of a composite
			// we delete the other (fixed size) pane as well which can lead to a chain.
			boolean removingFixedSize = view.isFixedSizeView() ;
			
			if (removingFixedSize)
			{
				// With a fixed size window we just promote the other side of the pair
				// up a level in the hierarchy.
				JavaElementXML parent = removePane.getParent() ;
				
				if (parent.getNumberChildren() != 2)
					throw new IllegalStateException("The parent of a fixed size window should be a composite pair") ;
				
				// Find the other pane of the pair
				JavaElementXML otherPane = parent.getChild(0) ;
				if (otherPane == removePane)
					otherPane = parent.getChild(1) ;
				
				// Promote the other pane to the position of the current parent
				JavaElementXML superParent = parent.getParent() ;
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
					JavaElementXML parent = removePane.getParent() ;
					
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
	
	// Remove an existing view: renameview <framename> <viewname>
	protected Object executeRenameView(String[] tokens)
	{
		String frameName = tokens[1] ;
		String viewName  = tokens[2] ;

		MainFrame frame = m_Document.getFrameByName(frameName) ;
		AbstractView view = frame.getView(viewName) ;
		Pane pane = view.getPane() ;

		boolean done = false ;
		
		while (!done)
		{
			String name = frame.ShowInputDialog("Rename window", "Enter the new name:", view.getName()) ;
	
			// Check if user cancelled
			if (name == null || name.length() == 0 || name.equals(view.getName()))
				return null ;

			// We can't allow spaces (throws off our script passing) or special characters (might throw off XML load/save)
			// If we really cared about these we could relax all of these limits, but it doesn't seem that critical.
			boolean valid = true ;
			for (int i = 0 ; i < name.length() ; i++)
			{
				char ch = name.charAt(i) ;
				
				if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || (ch == '_' || ch == '-' || ch == '<' || ch == '>')))
					valid = false ;
			}
			
			if (!valid)
			{
				frame.ShowMessageBox("Invalid characters in name", "Window names are limited to letters, numbers and '_', '-', '<', '>'") ;
				continue ;
			}
			
			if (frame.isViewNameInUse(name))
			{
				frame.ShowMessageBox("Name in use", "The window name has to be unique and " + name + " is already in use.") ;
				continue ;
			}
			
			// Change the name of this view
			view.changeName(name) ;
			pane.updateTabs() ;
			done = true ;
		}
		
		// Save immediately, so if the debugger crashes the changes are kept
		frame.saveCurrentLayoutFile() ;		

		return null ;
	}
	
	
	/** We allow the variables [thisframe] and [thisview] in the command.  Here we bind those variables */
	public String replaceVariables(MainFrame frame, AbstractView view, String command)
	{
		StringBuffer newCommand = new StringBuffer() ;
		String[] tokens = command.split(" ") ;

		for (int i = 0 ; i < tokens.length ; i++)
		{
			if (tokens[i] == "[thisframe]")
				tokens[i] = frame.getName() ;
			else if (tokens[i] == "[thisview]")
				tokens[i] = view.getName() ;
			
			newCommand.append(tokens[i]) ;
			
			if (i != tokens.length - 1)
				newCommand.append(" ") ;
		}
				
		return newCommand.toString() ;
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
		
		if (first.equals("loadsource"))
		{
			m_Frame.loadSource() ;
			return null ;
		}
		
		if (first.equals("askcd"))
		{
			m_Frame.getFileMenu().changeDirectory() ;
			return null ;
		}
		
		if (first.equals("removeview"))
		{
			return executeRemoveView(tokens) ;
		}
		
		if (first.equals("replaceview"))
		{
			return executeReplaceView(tokens) ;
		}

		if (first.equals("addview"))
		{
			return executeAddView(tokens) ;
		}
		
		if (first.equals("addtab"))
		{
			return executeAddTab(tokens) ;
		}
		
		if (first.equals("renameview"))
		{
			return executeRenameView(tokens) ;
		}
		
		if (first.equals("clear"))
		{
			return executeClearView(tokens) ;
		}
		
		if (first.equals("movetabs"))
		{
			return executeMoveTabs(tokens) ;
		}
		
		if (first.equals("remote"))
		{
			return executeRemoteConnect(tokens) ;
		}
		
		if (first.equals("copy") || first.equals("paste"))
		{
			return executeCopyPaste(tokens) ;
		}

		// properties <frame> <view>
		if (first.equals("properties"))
		{
			String frameName = tokens[1] ;
			String viewName  = tokens[2] ;

			MainFrame frame = m_Document.getFrameByName(frameName) ;
			AbstractView view = frame.getView(viewName) ;
			
			view.showProperties() ;
		}
		
		return null ;
	}
}
