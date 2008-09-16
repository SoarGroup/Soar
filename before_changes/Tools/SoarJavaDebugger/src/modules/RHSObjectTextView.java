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
	
	@Override
	protected void updateNow() {
		if (clear) {
			this.clearDisplay();
			clear = false;
		}
		
		Agent agent = m_Frame.getAgentFocus() ;
		if (agent == null) {
			return;
		}
		
		//System.out.println("--> Update ");
		StringBuilder output = new StringBuilder();
		
		// for each id
		Iterator<OrderedIdentifier> iter = sortedIdentifiers.iterator();
		while (iter.hasNext()) {
			OrderedIdentifier orderedIdentifier = iter.next();
			
			// verify it exists
			String result = agent.ExecuteCommandLine("print " + orderedIdentifier.getIdentifier());
			if (result.startsWith("There")) {
				// it doesn't exist, remove it
				iter.remove();
			} else {
				// it may just be in parens:
				if (result.contains("(" + orderedIdentifier.getIdentifier().toUpperCase() + ")")) {
					// if it is, don't print:
					
					//-----
					//As you'll recall, we remove old objects from the object window when
					//their ids can no longer be printed.  Unfortunately, there appears to be
					//a "bug" in Soar (I use that term loosely because I don't really
					//understand what's going on).  Sometimes when the id is no longer
					//connected to the state, they can still be printed (but show up as
					//empty).  For example, I might get this:
					//
					//print e27
					//(E27)
					//
					//We should detect this case and treat it the same as if it couldn't be
					//printed.  So if, when we do the print check, we just get the id in
					//parentheses, then it should be removed.  I can imagine someone actually
					//having legitimate ids like this that they want to show in the window,
					//but I think we should design this for the other 99.999% of cases :)
					//-----

					// it doesn't exist, remove it
					iter.remove();
					
				} else {
					output.append(orderedIdentifier.getIdentifier());
					output.append("\n");
					output.append(orderedIdentifier.getAttributes());
				}
			}
		}
		
		setTextSafely(output.toString());
	}

	public class OrderedIdentifier implements Comparable<OrderedIdentifier> {

		public OrderedIdentifier(String id) {
			this.id = id;
		}
		
		public String getIdentifier() {
			return this.id;
		}

		protected String id;
		protected int order = 0;
		protected String attributes = new String();
		protected double doubleOrder = 0;
		
		public void resetValues() {
			order = 0;
			doubleOrder = 0;
		}

		public int compareTo(OrderedIdentifier other) {
			if (this.order == other.order) {
				if (this.doubleOrder == other.doubleOrder) {
					return this.id.compareTo(other.id);
				}
				if (this.doubleOrder < other.doubleOrder) {
					return 1;
				} else if (this.doubleOrder > other.doubleOrder) {
					return -1;
				}
				return this.id.compareTo(other.id);
			}
//			System.out.println("*** comparing " + this.id + " to " + other.id);
			return other.order - this.order;
		}
		
		@Override
		public boolean equals(Object o) {
			OrderedIdentifier other;
			try {
				other = (OrderedIdentifier)o;
			} catch (ClassCastException e) {
				return super.equals(o);
			}
			return this.id.equals(other.id);
		}

		public String getAttributes() {
			return this.attributes;
		}

		public void setAttributes(String attributes) {
			assert attributes != null;
			this.attributes = attributes;
		}

		public int getOrder() {
			return this.order;
		}

		public void setOrder(int order) {
			this.order = order;
		}

		public double getDoubleOrder() {
			return doubleOrder;
		}

		public void setDoubleOrder(double doubleOrder) {
			this.doubleOrder = doubleOrder;
		}
		
		@Override
		public String toString() {
			return this.id + " (" + this.order + ", " + this.doubleOrder + ")";
		}
	}
	
	// Identifier to metadata map
	HashMap<String, OrderedIdentifier> idToOrdered = new HashMap<String, OrderedIdentifier>();
	// The sorted objects
	TreeSet<OrderedIdentifier> sortedIdentifiers = new TreeSet<OrderedIdentifier>();
	
	@Override
	public String rhsFunctionHandler(int eventID, Object data,
			String agentName, String functionName, String argument) {
		
		String[] commandLine = argument.split("\\s+");
		
		if (commandLine.length >= 1 && commandLine[0].equals("--clear")) {
			clear = true;
			return debugMessages ? m_Name + ":" + functionName + ": set to clear" : "";
		}
		
		// make sure we have 2 args
		if (commandLine.length <= 1) {
			return m_Name + ":" + functionName + ": at least two arguments required.";
		}
		
		// check for order command
		if (commandLine[1].equals("--order")) {
			// make sure we have a third arg
			if (commandLine.length < 3) {
				return m_Name + ":" + functionName + ": --order requires an argument";
			}
			
			int value = 0;
			try {
				value = Integer.parseInt(commandLine[2]);
			} catch (NumberFormatException e) {
				return m_Name + ":" + functionName + ": --order requires an integer argument";
			}
			
			OrderedIdentifier orderedIdentifier = idToOrdered.get(commandLine[0]);
			if (orderedIdentifier == null) {
				orderedIdentifier = new OrderedIdentifier(commandLine[0]);
				//System.out.println("Creating new oid " + orderedIdentifier);
			} else {
				sortedIdentifiers.remove(orderedIdentifier);
				//System.out.println("Found oid " + orderedIdentifier);
			}
			
			orderedIdentifier.setOrder(value);

			//System.out.println("Saving oid " + orderedIdentifier);
			idToOrdered.put(commandLine[0], orderedIdentifier);
			sortedIdentifiers.add(orderedIdentifier);
			
			return debugMessages ? m_Name + ":" + functionName + ": Updated order for " + commandLine[0] : "";
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
		
		OrderedIdentifier orderedIdentifier = idToOrdered.get(commandLine[0]);
		if (orderedIdentifier == null) {
			orderedIdentifier = new OrderedIdentifier(commandLine[0]);
			//System.out.println("Creating new oid " + orderedIdentifier);
		} else {
			sortedIdentifiers.remove(orderedIdentifier);
			//System.out.println("Found oid " + orderedIdentifier);
		}
		orderedIdentifier.setAttributes(output.toString());
		
		//System.out.println("Saving oid " + orderedIdentifier);
		idToOrdered.put(commandLine[0], orderedIdentifier);
		sortedIdentifiers.add(orderedIdentifier);
		
		return debugMessages ? m_Name + ":" + functionName + ": Updated " + commandLine[0] : "";
	}
	
	@Override
	public void clearDisplay() {
		idToOrdered.clear();
		sortedIdentifiers.clear();
		super.clearDisplay();
	}
	
	@Override
	public void onInitSoar() {
		clearDisplay();
		updateNow();
	}
}
