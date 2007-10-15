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

public class EmoTotalRewardView extends RHSFunTextView
{
	public EmoTotalRewardView()
	{
	}
	
	public String getModuleBaseName() { return "emo_total_reward" ; }
	
	@Override
	protected void updateNow() {
		setTextSafely(Double.toString(totalRewardValue));
	}

	double totalRewardValue = 0;
	
	@Override
	public String rhsFunctionHandler(int eventID, Object data,
			String agentName, String functionName, String argument) {
		
		if (functionName.equals("reward")) {
			double reward = 0;
			try {
				reward = Double.parseDouble(argument);
			} catch (NumberFormatException e) {
				return "Unknown argument to " + functionName;
			}
			
			totalRewardValue += reward;
			
			return "Total reward changed to: " + totalRewardValue;
			
		} else {
			assert false;
		}
		
		return "Unknown rhs function received.";
	}
	
	@Override
	public void onInitSoar() {
		totalRewardValue = 0;
		setTextSafely("0");
	}
}
