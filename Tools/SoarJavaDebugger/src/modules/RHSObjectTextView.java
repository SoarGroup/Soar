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
	
	@Override
	protected void updateNow() {
		Agent agent = m_Frame.getAgentFocus() ;
		if (agent == null) {
			return;
		}
		
		StringBuilder output = new StringBuilder();
		
		// for each id
		Set<String> keys = objectTextMap.keySet();
		String[] sortedKeys = keys.toArray(new String[0]);
		Arrays.sort(sortedKeys);
		
		for (int index = 0; index < sortedKeys.length; ++index) {
			
			// verify it exists
			String result = agent.ExecuteCommandLine("print " + sortedKeys[index]);
			if (!result.startsWith("There")) {
				// write it in to the text
				output.append(sortedKeys[index]);
				output.append("\n");
				output.append(objectTextMap.get(sortedKeys[index]));
				
			} else {
				// it doesn't exist, remove it
				objectTextMap.remove(sortedKeys[index]);
			}
		}
		setTextSafely(output.toString());
	}
	
	// identifier to associated text
	HashMap<String, String> objectTextMap = new HashMap<String, String>();
	
	@Override
	public String rhsFunctionHandler(int eventID, Object data,
			String agentName, String functionName, String argument) {
		
		if (!functionName.equals(rhsFunName)) {
			return "Unknown rhs function received in window " + getName() + ".";
		}
		
		String[] args = argument.split("\\s+");
		
		// make sure we have 2 args
		if (args.length <= 1) {
			return "RHS function " + rhsFunName + " requires at least one argument.";
		}
		
		// first arg is the event
		StringBuilder output = new StringBuilder();
		
		// the rest, if any, are attribute/value pairs
		for (int index = 1; index < args.length; index += 2) {
			// TODO: make this indentation a property?
			output.append("  ");
			
			output.append(args[index]);
			if (index + 1 < args.length) {
				output.append(": ");
				output.append(args[index + 1]);
			}
			output.append("\n");
		}

		objectTextMap.put(args[0], output.toString());
		
		return "Updated " + args[0];
	}
	
	@Override
	public void onInitSoar() {
		objectTextMap.clear();
		updateNow();
	}
}
