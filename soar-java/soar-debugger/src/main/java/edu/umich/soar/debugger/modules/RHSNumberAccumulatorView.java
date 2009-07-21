package edu.umich.soar.debugger.modules;



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
import edu.umich.soar.debugger.MainFrame;
import edu.umich.soar.debugger.dialogs.PropertiesDialog;
import edu.umich.soar.debugger.dialogs.ReorderButtonsDialog;
import edu.umich.soar.debugger.doc.Document;
import edu.umich.soar.debugger.general.JavaElementXML;
import edu.umich.soar.debugger.helpers.FormDataHelper;
import edu.umich.soar.debugger.manager.Pane;

public class RHSNumberAccumulatorView extends RHSFunTextView
{
	public String getModuleBaseName() { return "rhs_number_accumulator" ; }
	
	@Override
	protected void updateNow() {
		if (clear) {
			clearDisplay();
			clear = false;
		}
		setTextSafely(Double.toString(totalValue));
	}

	double totalValue = 0;

	@Override
	public String rhsFunctionHandler(int eventID, Object data,
			String agentName, String functionName, String argument) {
		
		if (functionName.equals("--clear")) {
			clear = true;
			return debugMessages ? m_Name + ":" + functionName + ": set to clear" : "";
		}
		
		double value = 0;
		try {
			value = Double.parseDouble(argument);
		} catch (NumberFormatException e) {
			return m_Name + ":" + functionName + ": Unknown argument to " + rhsFunName;
		}
		
		totalValue += value;
		
		return debugMessages ? m_Name + ":" + functionName + ": Total value changed to: " + totalValue : "";
	}

	@Override
	public void clearDisplay() {
		totalValue = 0;
		super.clearDisplay();
	}
	
	@Override
	public void onInitSoar() {
		clearDisplay();
		updateNow();
	}
}
