package modules;

import general.JavaElementXML;
import helpers.FormDataHelper;

import manager.Pane;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.custom.StyledText;
import org.eclipse.swt.events.*;

import sml.Agent;
import sml.Kernel;
import sml.smlAgentEventId;

import java.util.*;
import debugger.MainFrame;
import dialogs.PropertiesDialog;
import dialogs.ReorderButtonsDialog;
import doc.Document;

public class RHSObjectTextView extends RHSFunTextView implements Kernel.RhsFunctionInterface
{
	public RHSObjectTextView()
	{
	}
	
	public String getModuleBaseName() { return "rhs_object_text" ; }
	
	protected String priorityAttribute = new String();
	
	@Override
	protected void updateNow() {
		Agent agent = m_Frame.getAgentFocus() ;
		if (agent == null) {
			return;
		}
		
		StringBuilder output = new StringBuilder();
		
		// for each id
		Set<Map.Entry<String, String> > entrySet = objectTextMap.entrySet();
		Iterator<Map.Entry<String, String> > iter = entrySet.iterator();
		while (iter.hasNext()) {
			Map.Entry<String, String> entry = iter.next();
			
			// verify it exists
			String result = agent.ExecuteCommandLine("print " + entry.getKey());
			if (result.startsWith("There")) {
				// it doesn't exist, remove it
				iter.remove();
			} else {
				// write it in to the text
				if (priorityObject != null && priorityObject.equals(entry.getKey())) {
					StringBuilder priority = new StringBuilder();
					priority.append(entry.getKey());
					priority.append("\n");
					priority.append(entry.getValue());

					output.insert(0, priority);
				} else {
					output.append(entry.getKey());
					output.append("\n");
					output.append(entry.getValue());
				}
			}
		}
		
		setTextSafely(output.toString());
	}
	
	// identifier to associated text
	TreeMap<String, String> objectTextMap = new TreeMap<String, String>();
	String priorityObject = null;
	
	@Override
	public String rhsFunctionHandler(int eventID, Object data,
			String agentName, String functionName, String argument) {
		
		priorityObject = null;
		String[] commandLine = argument.split("\\s+");
		
		if (commandLine.length >= 1 && commandLine[0].equals("--clear")) {
			this.onInitSoar();
			return debugMessages ? m_Name + ":" + functionName + ": cleared" : null;
		}
		
		// make sure we have 2 args
		if (commandLine.length <= 1) {
			return m_Name + ":" + functionName + ": at least one argument required.";
		}
		
		// first arg is the event
		StringBuilder output = new StringBuilder();
		
		// the rest, if any, are attribute/value pairs
		for (int index = 1; index < commandLine.length; index += 2) {
			
			if (priorityAttribute.equals(commandLine[index])) {
				priorityObject = commandLine[0];
			}
			
			// TODO: make this indentation a property?
			output.append("  ");
			
			output.append(commandLine[index]);
			if (index + 1 < commandLine.length) {
				output.append(": ");
				output.append(commandLine[index + 1]);
			}
			output.append("\n");
		}

		objectTextMap.put(commandLine[0], output.toString());
		
		return debugMessages ? m_Name + ":" + functionName + ": Updated " + commandLine[0] : null;
	}
	
	@Override
	public void onInitSoar() {
		objectTextMap.clear();
		updateNow();
	}

	public void showProperties()
	{
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[5] ;
		
		properties[0] = new PropertiesDialog.IntProperty("Update automatically every n'th decision (0 => none)", m_UpdateEveryNthDecision) ;
		properties[1] = new PropertiesDialog.StringProperty("Name of RHS function to use to update this window", rhsFunName) ;
		properties[2] = new PropertiesDialog.StringProperty("Label text", labelText) ;
		properties[3] = new PropertiesDialog.BooleanProperty("Debug messages", debugMessages) ;
		properties[4] = new PropertiesDialog.StringProperty("Priority attribute (objects with this move to top of list)", priorityAttribute) ;
		
		boolean ok = PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;
		
		if (ok) {
			m_UpdateEveryNthDecision = ((PropertiesDialog.IntProperty)properties[0]).getValue() ;
			labelText = ((PropertiesDialog.StringProperty)properties[2]).getValue() ;
			setLabelText(labelText);
			debugMessages = ((PropertiesDialog.BooleanProperty)properties[3]).getValue() ;
			priorityAttribute = ((PropertiesDialog.StringProperty)properties[4]).getValue() ;

			if (this.getAgentFocus() != null)
			{
				// Make sure we're getting the events to match the new settings
				this.unregisterForAgentEvents(this.getAgentFocus()) ;
				this.registerForAgentEvents(this.getAgentFocus()) ;
			}
			
			String tempRHSFunName = ((PropertiesDialog.StringProperty)properties[1]).getValue() ;
			tempRHSFunName = tempRHSFunName.trim();
			
			// Make sure new one is different than old one and not zero length string
			if (tempRHSFunName.length() <= 0 || tempRHSFunName.equals(rhsFunName)) {
				return;
			}
			
			if (registeredRHSFunctions.contains(tempRHSFunName)) {
				// failed to register callback
				MessageBox errorDialog = new MessageBox(this.m_Frame.getShell(), SWT.ICON_ERROR | SWT.OK);
				errorDialog.setMessage("RHS function already registered: \"" + tempRHSFunName + "\", ignoring change.");
				errorDialog.open();
				return;
			}

			Agent agent = m_Frame.getAgentFocus() ;
			if (agent == null) {
				return;
			}

			// Try and register this, message and return if failure
			Kernel kernel = agent.GetKernel();
			int tempRHSCallback = kernel.AddRhsFunction(tempRHSFunName, this, null);
			
			// TODO: Verify that error check here is correct, and fix registerForAgentEvents
			// BUGBUG: remove true
			if (tempRHSCallback <= 0) {
				// failed to register callback
				MessageBox errorDialog = new MessageBox(this.m_Frame.getShell(), SWT.ICON_ERROR | SWT.OK);
				errorDialog.setMessage("Failed to change RHS function name \"" + tempRHSFunName + "\".");
				errorDialog.open();
				return;
			}
			
			// unregister old rhs fun
			boolean registerOK = true ;

			if (rhsCallback != -1)
				registerOK = kernel.RemoveRhsFunction(rhsCallback);
			
			rhsCallback = -1;

			// save new one
			rhsFunName = tempRHSFunName;
			registeredRHSFunctions.add(tempRHSFunName);
			rhsCallback = tempRHSCallback;
			
			if (!registerOK)
				throw new IllegalStateException("Problem unregistering for events") ;
		
		} // Careful, returns in the previous block!
	}
	
	/************************************************************************
	* 
	* Converts this object into an XML representation.
	* 
	*************************************************************************/
	@Override
	public JavaElementXML convertToXML(String title, boolean storeContent) {
		JavaElementXML element = new JavaElementXML(title) ;
		
		// It's useful to record the class name to uniquely identify the type
		// of object being stored at this point in the XML tree.
		Class cl = this.getClass() ;
		element.addAttribute(JavaElementXML.kClassAttribute, cl.getName()) ;

		if (m_Name == null)
			throw new IllegalStateException("We've created a view with no name -- very bad") ;
		
		// Store this object's properties.
		element.addAttribute("Name", m_Name) ;
		element.addAttribute("UpdateOnStop", Boolean.toString(m_UpdateOnStop)) ;
		element.addAttribute("UpdateEveryNthDecision", Integer.toString(m_UpdateEveryNthDecision)) ;
		element.addAttribute("RHSFunctionName", rhsFunName) ;
		element.addAttribute("LabelText", labelText) ;
		element.addAttribute("DebugMessages", Boolean.toString(debugMessages)) ;
		element.addAttribute("PriorityAttribute", priorityAttribute) ;
				
		if (storeContent)
			storeContent(element) ;

		element.addChildElement(this.m_Logger.convertToXML("Logger")) ;
		
		return element ;
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
	@Override
	public void loadFromXML(MainFrame frame, Document doc, Pane parent,
			JavaElementXML element) throws Exception {
		setValues(frame, doc, parent) ;

		m_Name			   	= element.getAttribute("Name") ;
		m_UpdateOnStop	   	= element.getAttributeBooleanThrows("UpdateOnStop") ;
		m_UpdateEveryNthDecision = element.getAttributeIntThrows("UpdateEveryNthDecision") ;
		String tempRHSFunName = element.getAttribute("RHSFunctionName");
		labelText 			= element.getAttribute("LabelText");
		debugMessages		= element.getAttributeBooleanThrows("DebugMessages");
		priorityAttribute 	= element.getAttribute("PriorityAttribute");
		
		if (tempRHSFunName == null) {
			tempRHSFunName = new String();
		}
		
		if (registeredRHSFunctions.contains(tempRHSFunName)) {
			MessageBox errorDialog = new MessageBox(this.m_Frame.getShell(), SWT.ICON_ERROR | SWT.OK);
			errorDialog.setMessage("RHS function already registered: \"" + tempRHSFunName + "\", ignoring change.");
			errorDialog.open();
			
			tempRHSFunName = new String();
		}
		
		rhsFunName = tempRHSFunName;
		
		if (labelText == null) {
			labelText = new String();
		}
		
		if (priorityAttribute == null) {
			priorityAttribute = new String();
		}
		
		JavaElementXML log = element.findChildByName("Logger") ;
		if (log != null)
			this.m_Logger.loadFromXML(doc, log) ;

		// Register that this module's name is in use
		frame.registerViewName(m_Name, this) ;
		
		// Actually create the window
		init(frame, doc, parent) ;

		// Restore the text we saved (if we chose to save it)
		restoreContent(element) ;
	}
}
