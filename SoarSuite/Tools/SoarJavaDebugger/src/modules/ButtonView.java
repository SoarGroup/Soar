/********************************************************************************************
*
* ButtonPanel.java
* 
* Created on 	Nov 23, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import general.JavaElementXML;
import helpers.FormDataHelper;

import manager.Pane;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.events.*;

import java.util.*;
import debugger.MainFrame;
import dialogs.PropertiesDialog;
import dialogs.ReorderButtonsDialog;
import doc.Document;

/********************************************************************************************
* 
* Shows a series of buttons for issuing commands.
* 
********************************************************************************************/
public class ButtonView extends AbstractFixedView
{
	public static class ButtonInfo
	{
		public String	m_Name ;
		public String	m_Command ;
		public String	m_InternalCommand ;
		public Button	m_Button ;
	}
	
	/** A list of ButtonInfo objects */
	protected ArrayList m_ButtonList = new ArrayList() ;
	protected String 	m_LinkedViewName ;
	
	public ButtonView()
	{
	}
	
	// BADBAD: Should support a range of different sets of commands and allow the user
	// to pick which set to start from in some fashion.
	public void addDefaultCommands()
	{
		addButton("Help", "help") ;
		addButton("Init-soar", "init-soar") ;
		addButton("Step", "step") ;
		addButton("Run 1 -e", "run 1 --elaboration") ;
		addButton("Run", "run") ;
		addButton("Stop", "stop-soar") ;
		addButton("Clear", "clear") ;
		addButton("Matches", "matches") ;
		addButton("Print <s>", "print <s>") ;
		addButton("Print <ts>", "print <ts>") ;
		addButton("Print <o>", "print <o>") ;
		
		// This button uses an internally scripted command to drive the debugger itself to load a demo
		addButton("Towers of Hanoi", null, "demo towers-of-hanoi towers-of-hanoi.soar") ;
		addButton("Water Jug", null, "demo water-jug water-jug-look-ahead.soar") ;
	}
		
	/********************************************************************************************
	* 
	* This "base name" is used to generate a unique name for the window.
	* For example, returning a base name of "trace" would lead to windows named
	* "trace1", "trace2" etc.
	* 
	********************************************************************************************/
	public String getModuleBaseName() { return "buttons" ; }
	
	/********************************************************************************************
	* 
	* Create a new button with visible text of "name" that issues command "command" when the
	* user presses the button.  (A null command is valid and does nothing).
	* 
	* The caller can also (optionally) register an internal command.  That is a command that
	* does something inside the debugger based on a set of scripted commands that the debugger
	* itself supports.
	* 
	********************************************************************************************/
	public void addButton(String name, String command, String internalCommand)
	{
		ButtonInfo button = new ButtonInfo() ;
		button.m_Name    = name ;
		button.m_Command = command ;
		button.m_InternalCommand = internalCommand ;
		
		m_ButtonList.add(button) ;
	}

	/********************************************************************************************
	* 
	* Create a new button with visible text of "name" that issues command "command" when the
	* user presses the button.  (A null command is valid and does nothing).
	* 
	********************************************************************************************/
	public void addButton(String name, String command)
	{
		addButton(name, command, null) ;
	}
	
	 /*******************************************************************************************
	 * 
	 * The button pane can be linked to a specific view -- in which case commands are executed there.
	 * If it is not linked it defaults to using the prime view for output.
	 * We use the name of the view instead of the view itself so that we can save and load the
	 * connection and also when loading we may load this button pane before the view it is linked
	 * to and just storing the name avoids a problem of when to resolve the name into the view.
	 * 
	 ********************************************************************************************/
	public void setLinkedView(String viewName)
	{
		m_LinkedViewName = viewName ;
	}
		
	protected MenuItem addButtonViewItem(Menu menu, String text)
	{
		MenuItem item = new MenuItem (menu, SWT.PUSH);
		item.setText (text) ;
		
		return item ;
	}	

	protected ButtonInfo getButtonInfo(Button button)
	{
		for (int i = 0 ; i < this.m_ButtonList.size() ; i++)
		{
			ButtonInfo info = (ButtonInfo)m_ButtonList.get(i) ;
			if (info.m_Button == button)
				return info ;
		}
		
		return null ;
	}
	
	protected void removeButton(Button button)
	{
		ButtonInfo info = getButtonInfo(button) ;
		
		if (m_ButtonList.size() == 1)
		{
			m_Frame.ShowMessageBox("Can't remove the last button -- remove the window instead") ;
			return ;
		}
		
		m_ButtonList.remove(info) ;
		
		// Recreate the button panel
		createButtonPanel(m_Container.getParent()) ;
		
		// Save the new layout, so a debugger crash (when working on a new layout) doesn't lose the work.
		m_Frame.saveCurrentLayoutFile() ;
	}
	
	protected void editButton(Button button)
	{
		ButtonInfo info = getButtonInfo(button) ;
		
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[3] ;
				
		properties[0] = new PropertiesDialog.StringProperty("Label", info.m_Name) ;
		properties[1] = new PropertiesDialog.MultiLineStringProperty("Command to execute", info.m_Command) ;
		properties[2] = new PropertiesDialog.StringProperty("Internal command to execute (advanced)", info.m_InternalCommand) ;
		
		PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;

		info.m_Name 		   = ((PropertiesDialog.StringProperty)properties[0]).getValue() ;
		info.m_Command 		   = ((PropertiesDialog.MultiLineStringProperty)properties[1]).getValue() ;
		info.m_InternalCommand = ((PropertiesDialog.StringProperty)properties[2]).getValue() ;
		
		// Update the label
		button.setText(info.m_Name) ;
		
		// Save the new layout, so a debugger crash (when working on a new layout) doesn't lose the work.
		m_Frame.saveCurrentLayoutFile() ;
	}

	protected void addButton()
	{
		ButtonInfo info = new ButtonInfo() ;
		info.m_Name = "Name" ;
		
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[3] ;
		
		properties[0] = new PropertiesDialog.StringProperty("Label", info.m_Name) ;
		properties[1] = new PropertiesDialog.MultiLineStringProperty("Command to execute", info.m_Command) ;
		properties[2] = new PropertiesDialog.StringProperty("Internal command to execute (advanced)", info.m_InternalCommand) ;
		
		boolean ok = PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;
		
		info.m_Name 		   = ((PropertiesDialog.StringProperty)properties[0]).getValue() ;
		info.m_Command 		   = ((PropertiesDialog.MultiLineStringProperty)properties[1]).getValue() ;
		info.m_InternalCommand = ((PropertiesDialog.StringProperty)properties[2]).getValue() ;
		
		if (ok)
		{
			// Add the new button
			addButton(info.m_Name, info.m_Command, info.m_InternalCommand) ;
			
			// Recreate the button panel
			createButtonPanel(m_Container.getParent()) ;
			
			// Save the new layout, so a debugger crash (when working on a new layout) doesn't lose the work.
			m_Frame.saveCurrentLayoutFile() ;
		}
	}
	
	protected void reorderButtons()
	{
		ArrayList buttonList = ReorderButtonsDialog.showDialog(m_Frame.getWindow(), "Reorder the list of buttons", m_ButtonList) ;
		
		if (buttonList == null)
			return ;
		
		m_ButtonList = buttonList ;

		// Recreate the button panel
		createButtonPanel(m_Container.getParent()) ;
		
		// Save the new layout, so a debugger crash (when working on a new layout) doesn't lose the work.
		m_Frame.saveCurrentLayoutFile() ;
	}

	/************************************************************************
	* 
	* Given a context menu and a control, fill in the items you want to 
	* see in the menu.  The simplest is to just call "fillWindowMenu".
	* 
	* This call is made after the user has clicked to bring up the menu
	* so we can create a dymanic menu based on the current context.
	* 
	* You also have to call createContextMenu() to request a context menu
	* be attached to a specific control.
	* 
	*************************************************************************/
	protected void fillInContextMenu(Menu contextMenu, final Control control, int mouseX, int mouseY)
	{
		if (control instanceof Button)
		{
			// The user has clicked on a specific button, so add menu items related to that button
			
			MenuItem item = addButtonViewItem(contextMenu, "Edit button...") ;

			item.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) { editButton((Button)control) ; } } ) ;

			item = addButtonViewItem(contextMenu, "Reorder buttons...") ;
			
			item.addSelectionListener(new SelectionAdapter() { 
				public void widgetSelected(SelectionEvent e) { reorderButtons() ; }
			}) ;
			
			new MenuItem(contextMenu, SWT.SEPARATOR) ;
			item = addButtonViewItem(contextMenu, "Remove button") ;
			new MenuItem(contextMenu, SWT.SEPARATOR) ;

			item.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) { removeButton((Button)control) ; } } ) ;

			fillWindowMenu(contextMenu, true, false) ;
		}
		else
		{
			// The user has clicked on the button bar, but not on a specific button.
			
			MenuItem item = addButtonViewItem(contextMenu, "Add button...") ;

			item.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) { addButton() ; } } ) ;

			item = addButtonViewItem(contextMenu, "Reorder buttons...") ;
			
			item.addSelectionListener(new SelectionAdapter() { 
				public void widgetSelected(SelectionEvent e) { reorderButtons() ; }
			}) ;

			item = addButtonViewItem(contextMenu, "Edit/remove button...") ;

			// BADBAD: Later we might have a dialog to handle multiple changes, but for now we'll point them to the real stuff.
			item.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) { m_Frame.ShowMessageBox("Edit/remove buttons", "To edit or remove a button, right click on that button and select an option") ; } } ) ;

			new MenuItem(contextMenu, SWT.SEPARATOR) ;

			// In background container
			fillWindowMenu(contextMenu, false, false) ;
		}
	}

	/********************************************************************************************
	* 
	* Initialize this window and its children.
	* Should call setValues() at the start to complete initialization of the abstract view.
	* 
	********************************************************************************************/
	public void init(MainFrame frame, Document doc, Pane parentPane)
	{
		setValues(frame, doc, parentPane) ;
		
		if (m_ButtonList.size() == 0)
		{
			// Add some default buttons if none have been specified so we
			// can definitely see the window etc.
			addDefaultCommands() ;
		}
		
		createButtonPanel(m_Pane.getWindow()) ;
	}
	
	public void showProperties()
	{
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[1] ;

		// Create a list of all views out there which can accept output
		AbstractView[] views = this.m_Frame.getAllOutputViews() ;
		String[] descriptions = new String[views.length] ;
		
		for (int i = 0 ; i < views.length ; i++)
		{
			descriptions[i] = views[i].getName() ;
		}
		
		// Look up the view that we're currently linked to (if any)
		AbstractView currentView = this.m_Frame.getView(m_LinkedViewName) ;

		//Object initialValue, Object[] setValues, String[] setDescriptions 
		properties[0] = new PropertiesDialog.SetProperty("Window to use for output", currentView, views, descriptions) ;
		
		boolean ok = PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;

		if (ok)
		{
			AbstractView linkedView = (AbstractView)((PropertiesDialog.SetProperty)properties[0]).getValue() ;
			m_LinkedViewName = (linkedView == null) ? null : linkedView.getName() ;
		}		
	}

	/********************************************************************************************
	 * 
	 * We want the button views to wrap to use a second line when needed.
	 * However, there's a problem letting the parent composite know that it should give this
	 * view the space it needs to wrap.
	 * 
	 * The solution I'm using is to compute how much size the view wants to have
	 * and then manually setting the height/width value of the formData of the parent.
	 * 
	 * This is the only solution I've found so far.  It's quite likely there's a simple solution
	 * to this but finding even this one took a while.
	 * 
	********************************************************************************************/
	private void setPrefSize()
	{
		boolean horiz = getPane().isHorizontalOrientation() ;
		
		// See how much space our window currently has
		int height = m_Container.getBounds().height ;
		int width = m_Container.getBounds().width ;
		
		// Find out how much height/width we'd like to have by keeping one
		// value fixed and requested the preference for the other.
		Point size = horiz ? m_Container.computeSize(width, SWT.DEFAULT) : m_Container.computeSize(SWT.DEFAULT, height) ;
		
		int prefWidth = size.x ;
		int prefHeight = size.y ;

		// Check that our parent's parent is a composite used to glue two windows together.
		// This will always be the case unless we change how the MainWindow manages pairs of windows.
		Composite parent1 = m_Container.getParent();
		Composite parent2 = parent1.getParent() ;
		Object key = parent2.getData(manager.MainWindow.kAttachKey) ;
		
		if (key == null)
		{
			System.out.println("This button view doesn't appear to be hosted in an attached composite -- the window structure is off.") ;
			return ;
		}
		
		// Get the current form data object and make a copy of it.
		FormData existingData = (FormData)parent1.getLayoutData() ;
		if (existingData == null)
			return ;
		
		FormData newData = FormDataHelper.copyFormData(existingData) ;
		
		// Explicitly set the height/width we want to see
		// (which will allow enough space for the wrap to occur)
		if (horiz)
			newData.height = prefHeight ;
		else
			newData.width = prefWidth ;

		// Store the new layout data
		parent1.setLayoutData(newData) ;
		
		// Ask the parent to relay itself out to show the requested size
		parent2.layout(true) ;
	}
	
	protected void createButtonPanel(final Composite parent)
	{
		// Allow us to recreate the panel by calling this multiple times
		if (m_Container != null)
		{
			m_Container.dispose() ;
			m_Container = null ;
		}
		
		// The container lets us control the layout of the controls
		// within this window
		m_Container	   = new Composite(parent, SWT.NULL) ;
		
		if (getPane().isHorizontalOrientation())
		{
			RowLayout layout = new RowLayout(SWT.HORIZONTAL) ;
			layout.wrap = true ;
			layout.fill = true ;
			m_Container.setLayout(layout) ;				
		}
		else
		{
			RowLayout layout = new RowLayout(SWT.VERTICAL) ;
			layout.fill = true ;
			m_Container.setLayout(layout) ;		
		}
		
		m_Container.addControlListener(new ControlAdapter() {
			public void controlResized(ControlEvent e)
			{
				setPrefSize() ;
		} }) ;
				
		// Create and add buttons for each button info structure
		for (int i = 0 ; i < m_ButtonList.size() ; i++)
		{
			final ButtonInfo info = (ButtonInfo)m_ButtonList.get(i) ;
			
			final int pos = i ;
			Button button = new Button(m_Container, SWT.PUSH) ;
			button.setText(info.m_Name) ;
			
			// When the user presses the button we call our default handler and
			// optionally a handler registered with the button (to do a custom action)
			button.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) { buttonPressed(e, pos) ; } ;
			}) ;
			
			// Record a reference to the button, in case we need it.
			info.m_Button = button ;
		}
		
		// Create a context menu for m_Text.
		// It will be filled in via a call to fillInContextMenu when the menu is popped up
		// (this allows for dynamic content)
		createContextMenu(m_Container) ;

		// Create a context menu for each button -- allowing us to modify it etc.
		for (int i = 0 ; i < m_ButtonList.size() ; i++)
		{
			ButtonInfo info = (ButtonInfo)m_ButtonList.get(i) ;
			Button button = info.m_Button ;
			
			createContextMenu(button) ;
		}
		
		// Layout the parent again, because this set of windows has changed
		// This only matters if we're recreating the windows
		parent.layout(true) ;
	}
	
	protected AbstractView getLinkedView()
	{
		if (m_LinkedViewName == null)
			return null ;

		// Even when we have a name it's possible that the view it referred to
		// is no longer around, so this can still return null.
		return m_Frame.getView(m_LinkedViewName) ;
	}
	
	protected void buttonPressed(SelectionEvent e, int pos)
	{
		ButtonInfo button = (ButtonInfo)m_ButtonList.get(pos) ;
		String command = button.m_Command ;

		// Execute the command in the prime view for the debugger
		if (command != null && command.length() > 0)
		{
			AbstractView linkedView = getLinkedView() ;
			
			// We now allow multiple line commands, so process each in turn.
			String lines[] = command.split(AbstractView.kLineSeparator) ;
			
			for (int i = 0 ; i < lines.length ; i++)
			{
				if (lines[i].length() == 0)
					continue ;
				
				if (linkedView != null)
					linkedView.executeAgentCommand(lines[i], true) ;
				else
					m_Frame.executeCommandPrimeView(lines[i], true) ;			
			}
		}
		
		if (button.m_InternalCommand != null)
		{
			m_Frame.executeScriptCommand(this, button.m_InternalCommand, true) ;
		}
	}

	/************************************************************************
	* 
	* Converts this object into an XML representation.
	* 
	* For the button view there is no content beyond the list of buttons.
	* 
	*************************************************************************/
	public general.JavaElementXML convertToXML(String title, boolean storeContent)
	{
		JavaElementXML element = new JavaElementXML(title) ;
		
		// It's useful to record the class name to uniquely identify the type
		// of object being stored at this point in the XML tree.
		Class cl = this.getClass() ;
		element.addAttribute(JavaElementXML.kClassAttribute, cl.getName()) ;

		// Store this object's properties.
		element.addAttribute("Name", m_Name) ;
		element.addAttribute("ButtonCount", Integer.toString(m_ButtonList.size())) ;

		if (m_LinkedViewName != null)
			element.addAttribute("LinkedView", m_LinkedViewName) ;
		
		// Save information for each button in the panel
		for (int i = 0 ; i < m_ButtonList.size() ; i++)
		{
			ButtonInfo button = (ButtonInfo)m_ButtonList.get(i) ;
			
			JavaElementXML child = new JavaElementXML("Button") ;
			child.addAttribute("Name", button.m_Name) ;
			
			if (button.m_Command != null)
				child.addAttribute("Command", button.m_Command) ;
			
			if (button.m_InternalCommand != null)
				child.addAttribute("InternalCommand", button.m_InternalCommand) ;
			
			element.addChildElement(child) ;
		}
			
		return element ;
	}

	/************************************************************************
	* 
	* Create an instance of the class.  It does not have to be fully initialized
	* (it's the caller's responsibility to finish the initilization).
	* 
	*************************************************************************/
	public static ButtonView createInstance()
	{
		return new ButtonView() ;
	}

	/************************************************************************
	* 
	* Rebuild the object from an XML representation.
	* 
	* @param frame			The top level window that owns this window
	* @param doc			The document we're rebuilding
	* @param parent			The pane window that owns this view
	* @param element		The XML representation of this command
	* 
	*************************************************************************/
	public void loadFromXML(MainFrame frame, doc.Document doc, Pane parent, general.JavaElementXML element) throws Exception
	{
		setValues(frame, doc, parent) ;
		
		m_ButtonList.clear() ;
		
		m_Name   = element.getAttributeThrows("Name") ;
		int size = element.getAttributeIntThrows("ButtonCount") ;
		m_LinkedViewName = element.getAttribute("LinkedView") ;	// This one is optional
		
		for (int i = 0 ; i < size ; i++)
		{
			JavaElementXML child = element.getChild(i) ;
			
			ButtonInfo button = new ButtonInfo() ;
			button.m_Name    = child.getAttributeThrows("Name") ;
			button.m_Command = child.getAttribute("Command") ;
			button.m_InternalCommand = child.getAttribute("InternalCommand") ;
			
			m_ButtonList.add(button) ;
		}
		
		// Register that this module's name is in use
		frame.registerViewName(m_Name, this) ;

		// Actually create the window
		init(frame, doc, parent) ;
	}

}
