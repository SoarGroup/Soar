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
				output.append(entry.getKey());
				output.append("\n");
				output.append(entry.getValue());
			}
		}
		
		setTextSafely(output.toString());
	}
	
	// identifier to associated text
	TreeMap<String, String> objectTextMap = new TreeMap<String, String>();
	
	@Override
	public String rhsFunctionHandler(int eventID, Object data,
			String agentName, String functionName, String argument) {
		
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
}
