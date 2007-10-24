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
			return debugMessages ? m_Name + ":" + functionName + ": cleared" : "";
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
		
		return debugMessages ? m_Name + ":" + functionName + ": Updated " + commandLine[0] : "";
	}
	
	@Override
	public void onInitSoar() {
		objectTextMap.clear();
		updateNow();
	}

	private int propertiesStartingIndex;
	
	@Override
	protected void initProperties(ArrayList<PropertiesDialog.Property> properties) {
		super.initProperties(properties);

		propertiesStartingIndex = properties.size();
		
		properties.add(new PropertiesDialog.StringProperty("Priority attribute (objects with this move to top of list)", priorityAttribute));
	}
	
	@Override
	protected void processProperties(ArrayList<PropertiesDialog.Property> properties) {
		super.processProperties(properties);

		priorityAttribute = ((PropertiesDialog.StringProperty)properties.get(propertiesStartingIndex)).getValue() ;
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
		m_UpdateOnStop	   	= element.getAttributeBooleanDefault("UpdateOnStop", true) ;
		m_UpdateEveryNthDecision = element.getAttributeIntDefault("UpdateEveryNthDecision", 0) ;
		rhsFunName			= element.getAttribute("RHSFunctionName");
		labelText 			= element.getAttribute("LabelText");
		debugMessages		= element.getAttributeBooleanDefault("DebugMessages", true);
		priorityAttribute 	= element.getAttribute("PriorityAttribute");
		
		if (rhsFunName == null) {
			rhsFunName = new String();
		}
		
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
