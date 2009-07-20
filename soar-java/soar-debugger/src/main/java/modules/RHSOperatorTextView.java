package modules;

import general.JavaElementXML;
import helpers.FormDataHelper;

import manager.Pane;
import modules.RHSObjectTextView.OrderedIdentifier;

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
	
	@Override
	protected void updateNow() {
		if (clear) {
			this.clearDisplay();
			clear = false;
		}
		

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
		
		if (this.idToOrdered.size() < 1) {
			return;
		}
		
		Agent agent = m_Frame.getAgentFocus() ;
		if (agent == null) {
			return;
		}
		
		// clear out old values
		{
			Iterator<OrderedIdentifier> iter = idToOrdered.values().iterator();
			while (iter.hasNext()) {
				OrderedIdentifier oid = iter.next();
				oid.resetValues();
			}
		}
		
		HashSet<String> reported = new HashSet<String>();		
		
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

					OrderedIdentifier oid = this.idToOrdered.get(match1);
					if (oid != null) {
						reported.add(new String(match1));
					}
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
					
					OrderedIdentifier oid = this.idToOrdered.get(match1);
					if (oid != null) {
						Double value = oid.getDoubleOrder();
						value += Double.parseDouble(match2);
						//System.out.println(oid + " --> " + value);
						
						oid.setDoubleOrder(value);
					}
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

		this.idToOrdered.keySet().retainAll(reported);
		this.sortedIdentifiers.clear();
		{
			Iterator<Map.Entry<String, OrderedIdentifier> > iter = this.idToOrdered.entrySet().iterator();
			while (iter.hasNext()) {
				this.sortedIdentifiers.add(iter.next().getValue());
			}
		}
		
		// iterate through sortedIdentifiers and print values
		StringBuilder output = new StringBuilder();
		{
			Iterator<OrderedIdentifier> iter = this.sortedIdentifiers.iterator();
			while (iter.hasNext()) {
				OrderedIdentifier oid = iter.next();
				
				output.append(oid.getIdentifier());
				output.append(" (");
				output.append(oid.getDoubleOrder());
				output.append(")\n");
				output.append(oid.getAttributes());

			}	
		}
		
		setTextSafely(output.toString());
	}
}
