package modules;

import general.JavaElementXML;
import helpers.FormDataHelper;

import manager.Pane;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.custom.StyledText;
import org.eclipse.swt.events.*;

import sml.Agent;
import sml.ClientAnalyzedXML;
import sml.Kernel;
import sml.smlAgentEventId;

import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import debugger.MainFrame;
import dialogs.PropertiesDialog;
import dialogs.ReorderButtonsDialog;
import doc.Document;

public class RHSOperatorTextView extends RHSObjectTextView implements Kernel.RhsFunctionInterface
{
	public RHSOperatorTextView()
	{
	}
	
	public String getModuleBaseName() { return "rhs_operator_text" ; }

	class OperatorValue implements Comparable<OperatorValue> {
		
		public OperatorValue()
		{
		}
		
		OperatorValue(OperatorValue ov) {
			this.operator = new String(ov.operator);
			this.value = new Double(ov.value);
		}
		
		public String operator;
		public Double value;
		
		// This is backwards because descending iterator is a java 1.6 feature
		public int compareTo(OperatorValue arg0) {
			if (this.value > arg0.value) {
				return -1;
			} else if (this.value < arg0.value) {
				return 1;
			}
			return 0;
		}
	}

	enum ParseState { START, ACCEPTABLES, BEFORE_NUM, NUM, DONE };
	
	@Override
	protected void updateNow() {

//		String test1 = "  O154 (verify) =0 :I";
//		String test2 = "  O154 (verify) =0.244948 :I";
//		String test3 = "  O153 (clear-attended-flags) =-0.0745079 :I";
//		String test4 = "  O1 (two) + :I ";
//		Matcher m;
//		m = Pattern.compile("^\\s+(O\\d+).+=(.+)\\s.*$").matcher(test1);
//		if (m.matches()) {
//			System.out.println(m.group(2));
//		}
//		m = Pattern.compile("^\\s+(O\\d+).+=(.+)\\s.*$").matcher(test2);
//		if (m.matches()) {
//			System.out.println(m.group(2));
//		}
//		m = Pattern.compile("^\\s+(O\\d+).+=(.+)\\s.*$").matcher(test3);
//		if (m.matches()) {
//			System.out.println(m.group(2));
//		}
//		m = Pattern.compile("^\\s+(O\\d+)\\s+.*").matcher(test4);
//		if (m.matches()) {
//			System.out.println(m.group(1));
//		}
//		System.out.println("end");
//		
//		if (true) return;
		
		Agent agent = m_Frame.getAgentFocus() ;
		if (agent == null) {
			return;
		}
		
		HashMap<String, Double> operatorPreferences = new HashMap<String, Double>();
		
		// get preferences result, split on newlines
		String[] prefOutput = agent.ExecuteCommandLine("pref").split("\n");
		
		// get preference value for each operator
		ParseState state = ParseState.START;
		Matcher matcher;
		String match1;
		String match2;
		for (int index = 0; index < prefOutput.length; ++index) {
			switch (state) {
			case START:
				if (prefOutput[index].contains("acceptables")) {
					state = ParseState.ACCEPTABLES;
				}
				break;
			
			case ACCEPTABLES:
				matcher = Pattern.compile("^\\s+(O\\d+)\\s+.*").matcher(prefOutput[index]);
				if (matcher.matches()) {
					match1 = matcher.group(1);
					operatorPreferences.put(new String(match1), new Double(0));
				} else {
					state = ParseState.BEFORE_NUM;
				}
				break;
				
			case BEFORE_NUM:
				if (prefOutput[index].contains("indifferents")) {
					state = ParseState.NUM;
				}
				break;
			
			case NUM:
				matcher = Pattern.compile("^\\s+(O\\d+).+=(-?\\d+\\.?\\d*)\\s.*$").matcher(prefOutput[index]);
				if (matcher.matches()) {
					match1 = matcher.group(1);
					match2 = matcher.group(2);
					Double value = operatorPreferences.get(match1);
					value += Double.parseDouble(match2);
					operatorPreferences.put(new String(match1), value);
				} else {
					state = ParseState.DONE;
				}
				break;
			
			default:
				assert false;
			}
			
			if (state == ParseState.DONE) {
				break;
			}
		}
		
		TreeSet<OperatorValue> orderedOperators = new TreeSet<OperatorValue>();
		OperatorValue operatorValue = new OperatorValue();
		{
			Iterator<String> iter = operatorPreferences.keySet().iterator();
			while (iter.hasNext()) {
				operatorValue.operator = iter.next();
				operatorValue.value = operatorPreferences.get(operatorValue.operator);
				orderedOperators.add(new OperatorValue(operatorValue));
			}
		}
		
		// iterate through container and print values
		StringBuilder output = new StringBuilder();
		{
			// descending iterator is 1.6 specific so we use a backward comparator
			// and a regular comparator
			Iterator<OperatorValue> iter = orderedOperators.iterator();
			while (iter.hasNext()) {
				operatorValue = iter.next();

				output.append(operatorValue.operator);
				output.append(" (");
				output.append(operatorValue.value);
				output.append(")\n");
				output.append(objectTextMap.get(operatorValue.operator));
			}	
		}
		
		setTextSafely(output.toString());
	}
}
