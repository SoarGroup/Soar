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

public class RHSOperatorTextView extends RHSObjectTextView
{
	public String getModuleBaseName() { return "rhs_operator_text" ; }

	enum ParseState { START, ACCEPTABLES, BEFORE_NUM, NUM, DONE };
	
	static final Comparator<Double> kReverseOrder = new Comparator<Double>() {
		public int compare(Double x, Double y) {
			if (x < y) {
				return 1;
			} else if (x > y) {
				return -1;
			}
			return 0;
		}
	};
	
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
		String[] prefOutput = agent.ExecuteCommandLine("pref s1").split("\n");
		
		// get preference value for each operator
		ParseState state = ParseState.START;
		Matcher matcher;
		String match1;
		String match2;
		for (int index = 0; index < prefOutput.length; ++index) {
			switch (state) {
			case START:
				matcher = Pattern.compile("acceptables:$").matcher(prefOutput[index]);
				if (matcher.matches()) {
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
				matcher = Pattern.compile(".*indifferents:$").matcher(prefOutput[index]);
				if (matcher.matches()) {
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
		
		TreeMap<Double, ArrayList<String> > valueToOperators = new TreeMap<Double, ArrayList<String> >(kReverseOrder);
		{
			// foreach op/val in hash
			Set<Map.Entry<String, Double> > entrySet = operatorPreferences.entrySet();
			Iterator<Map.Entry<String, Double> > iter = entrySet.iterator();
			while (iter.hasNext()) {
				Map.Entry<String, Double> entry = iter.next();
				
				// if value in treemap
				if (valueToOperators.containsKey(entry.getValue())) {
					// add op to list
					valueToOperators.get(entry.getValue()).add(entry.getKey());
				} else {
					// new entry
					ArrayList<String> operators = new ArrayList<String>();
					operators.add(entry.getKey());
					valueToOperators.put(entry.getValue(), operators);
					
				}
			}
		}

		// iterate through treemap and print values
		StringBuilder output = new StringBuilder();
		{
			Set<Map.Entry<Double, ArrayList<String> > > entrySet = valueToOperators.entrySet();
			Iterator<Map.Entry<Double, ArrayList<String> > > iter = entrySet.iterator();
			
			// descending iterator is 1.6 specific so we use a backward comparator
			// and a regular comparator
			while (iter.hasNext()) {
				Map.Entry<Double, ArrayList<String> > entry = iter.next();
				
				Iterator<String> operators = entry.getValue().iterator();
				while (operators.hasNext()) {
					String operator = operators.next();

					output.append(operator);
					output.append(" (");
					output.append(entry.getKey());
					output.append(")\n");
					if (objectTextMap.containsKey(operator)) {
						output.append(objectTextMap.get(operator));
					}
				}
			}	
		}
		
		setTextSafely(output.toString());
	}
}
