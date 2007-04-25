/********************************************************************************************
*
* XmlTrace.java
* 
* Description:	
* 
* Created on 	May 24, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package helpers;

import modules.AbstractView;
import sml.Agent;
import sml.ClientTraceXML;
import sml.sml_Names;

/************************************************************************
 * 
 * Methods we can use to convert XML trace data into strings to display
 * 
 ************************************************************************/
public class XmlOutput
{
	public static final String kLineSeparator = AbstractView.kLineSeparator ;

	/** Number of spaces we indent conditions inside a production */
	public final static int kProductionIndent = 4 ;

	public final static int kDecisionDigits = 6 ;	// So colons match with print --stack in trace window

	/** Cache some indent strings that we'll need */
	public static String kProdIndent ;
	public static String kLineProdIndent ;

	/** We cache a series of strings made up of just spaces up to a certain size, so we can do rapid indenting through a lookup */
	protected static final int kCachedSpaces = 100 ;
	protected static final String[] kPadSpaces = new String[kCachedSpaces] ;
	
	/** This is a class constructor -- it runs once, the first time the class is used.  Don't mistake it for a normal, instance constructor */
	static
	{
		// Fill in the kPadSpaces array
		StringBuffer buffer = new StringBuffer() ;
		
		for (int i = 0 ; i < kCachedSpaces ; i++)
		{
			kPadSpaces[i] = buffer.toString() ;
			buffer.append(" ") ;
		}

		/** Cache some indent strings that we'll need */
		kProdIndent     = getSpaces(kProductionIndent) ;
		kLineProdIndent = kLineSeparator + kProdIndent ;

	}
	
	/** Returns a string of spaces of the given length (>= 0).  This is an efficient calculation */
	public static String getSpaces(int length)
	{
		if (length <= 0)
			return "" ;
		
		// We use a lookup from a table if the length is reasonable (< 100 right now)
		if (length < kPadSpaces.length)
			return kPadSpaces[length] ;
		
		// Otherwise we have to generate it which is slow
		StringBuffer buffer = new StringBuffer() ;
		buffer.append(kPadSpaces[kPadSpaces.length - 1]) ;
		
		// If we use this a lot we could speed it up by using a binary addition process
		// but I hope to never use it (except in a run-away stack situation).
		for (int i = 0 ; i < length - kPadSpaces.length ; i++)
		{
			buffer.append(" ") ;
		}
		
		return buffer.toString() ;
	}
	
	/** Add spaces to the length until reaches minLength */
	public static String padLeft(String orig, int minLength)
	{
		if (orig.length() >= minLength)
			return orig ;
				
		// Add the appropriate number of spaces.
		return getSpaces(minLength - orig.length()) + orig ;
	}
	
	/** Returns a string to indent to a certain stack depth (depth stored as a string) */
	public static String indent(String depthStr, int modifier, int indentStep)
	{
		if (depthStr == null)
			return "" ;
		
		int depth = Integer.parseInt(depthStr) + modifier ;
		
		int indentSize = depth * indentStep ;
		
		return getSpaces(indentSize) ;
	}
		
	public static String getStateText(Agent agent, ClientTraceXML xmlTrace, int indentSize)
	{
		// 3:    ==>S: S2 (operator no-change)
		StringBuffer text = new StringBuffer() ;				
		text.append(XmlOutput.padLeft(xmlTrace.GetDecisionCycleCount(), kDecisionDigits)) ;
		text.append(": ") ;
		text.append(XmlOutput.indent(xmlTrace.GetStackLevel(), -1, indentSize)) ;

		// Add an appropriate subgoal marker to match the indent size
		if (indentSize == 3)
			text.append("==>") ;
		else if (indentSize == 2)
			text.append("=>") ;
		else if (indentSize == 1)
			text.append(">") ;
		else if (indentSize > 3)
		{
			text.append(XmlOutput.getSpaces(indentSize - 3)) ;
			text.append("==>") ;
		}
		
		text.append("S: ") ;
		text.append(xmlTrace.GetStateID()) ;
		
		if (xmlTrace.GetImpasseObject() != null)
		{
			text.append(" (") ;
			text.append(xmlTrace.GetImpasseObject()) ;
			text.append(" ") ;
			text.append(xmlTrace.GetImpasseType()) ;
			text.append(")") ;
		}
		
		return text.toString() ;
	}
	
	public static String getOperatorText(Agent agent, ClientTraceXML xmlTrace, int indentSize)
	{
		 //2:    O: O8 (move-block)
		StringBuffer text = new StringBuffer() ;
		text.append(XmlOutput.padLeft(xmlTrace.GetDecisionCycleCount(), kDecisionDigits)) ;
		text.append(": ") ;
		text.append(XmlOutput.indent(xmlTrace.GetStackLevel(), 0, indentSize)) ;
		text.append("O: ") ;
		text.append(xmlTrace.GetOperatorID()) ;
		
		if (xmlTrace.GetOperatorName() != null)
		{
			text.append(" (") ;
			text.append(xmlTrace.GetOperatorName()) ;
			text.append(")") ;
		}

		return text.toString() ;
	}
	
	public static String getPhaseText(Agent agent, ClientTraceXML xmlTrace, String status)
	{
		StringBuffer text = new StringBuffer() ;
		
		String firingType = xmlTrace.GetFiringType() ;
		
		//String msg = xmlTrace.GenerateXMLString(true) ;
		//System.out.println(msg) ;
		
		text.append("--- ") ;
		text.append(xmlTrace.GetPhaseName()) ;
		text.append(" ") ;
		text.append("phase ") ;
		if (status != null)
			text.append(status) ;
		if (firingType != null)
		{
			text.append("(") ;
			text.append(firingType) ;
			text.append(") ") ;
		}
		text.append("---") ;
		
		return text.toString() ;
	}
	
	public static String getSubphaseText(Agent agent, ClientTraceXML xmlTrace)
	{
		StringBuffer text = new StringBuffer() ;

		String subphase = "unknown" ;
		
		if (xmlTrace.IsSubphaseNameFiringProductions())
			subphase = "Firing Productions" ;
		if (xmlTrace.IsSubphaseNameChangingWorkingMemory())
			subphase = "Change Working Memory" ;
		
		text.append("--- ") ;
		text.append(subphase) ;
		text.append(" (") ;
		text.append(xmlTrace.GetFiringType()) ;
		text.append(") ") ;
		text.append("---") ;
		
		return text.toString() ;
	}

/*
<production prod_name="my*prod" documentation="my doc string" 
type="[:chunk|:default|:justification ;# not reloadable]" 
declared-support="[:i-support|:o-support]">
   <conditions>
      <conjunctive-negation-condition>
         <condition test="[state|impasse]" id="<s1>" condition="^foo bar ^hello <world>"></condition>
         ...
      </conjunctive-negation-condition>
      <condition ...></condition>
      ...
   </conditions>
   <actions>
      <action id="<s1>" action="^foo2 bar2 ^what ever"></action>
      <action function="some function string"></action>
   </actions>
</production>
*/	
	public static String getConditionText(Agent agent, ClientTraceXML cond, boolean indent)
	{
		StringBuffer text = new StringBuffer() ;
		
		if (indent)
			text.append(kProdIndent) ;

		text.append("(") ;
		
	    //(state <s1> ^name water-jug ^desired <d1> ^jug <j1> ^jug <j2>)
		String test = cond.GetConditionTest() ;
		if (test != null)
		{
			text.append(test) ;
			text.append(" ") ;
		}
		
		String id = cond.GetConditionId() ;
		text.append(id) ;
		text.append(" ") ;
		
		String conditions = cond.GetCondition() ;
		text.append(conditions) ;

		text.append(")") ;

		return text.toString() ;
	}

	//<candidate name="O1" type="[sum|avg]" value="123.4"></candidate>
	//The text output looks something like:
	//Candidate O1:   Value (Avg) = 123.4
	public static String getNumericIndiffernceText(Agent agent, ClientTraceXML xmlTrace)
	{
		StringBuffer text = new StringBuffer() ;
		
		// Going to allow for any and all of these being null since this is more experimental
		// then the rest of the trace
		String candidate = xmlTrace.GetCandidateName() ;
		String type = xmlTrace.GetCandidateType() ;
		String value = xmlTrace.GetCandidateValue() ;
		String expvalue = xmlTrace.GetCandidateExpValue() ;
		
		if (candidate != null)
		{
			text.append("Candidate ") ;
			text.append(candidate) ;
			text.append(":   Value ") ;
			
			if (type != null)
			{
				text.append("(") ;
				text.append(type) ;
				text.append(")") ;
			}
			
			text.append(" = ") ;
			text.append(value) ;

			if (expvalue != null)
			{
				text.append(", (Exp) = ") ;
				text.append(expvalue) ;
			}

		}
		
		return text.toString() ;
	}
	
	/*
	 <backtrace prodname="evaluate-operator*elaborate*symbolic-evaluation*from-subgoal">
			<grounds>
				<wme attr="desired" id="E4" tag="281" value="D1"></wme>
				<wme attr="evaluation" id="O25" tag="282" value="E4"></wme>
				<wme attr="evaluation" id="S8" tag="277" value="E4"></wme>
				<wme attr="evaluation" id="O25" tag="282" value="E4"></wme>
				<wme attr="name" id="O25" tag="270" value="evaluate-operator"></wme>
				<wme attr="operator" id="S8" tag="276" value="O25"></wme>
			</grounds>
			<potentials></potentials>
			<locals>
				<wme attr="failure" id="S9" tag="329" value="D1"></wme>
				<wme attr="superstate" id="S9" tag="284" value="S8"></wme>
			</locals>
			<negated></negated>
			<nots></nots>
		</backtrace>
	 */
	
	public static String getBacktraceText(Agent agent, ClientTraceXML xml, String parentTagName)
	{
		StringBuffer text = new StringBuffer() ;

		if (xml.IsTagBacktrace())
		{
			String name = xml.GetProductionName() ;
			
			if (name != null)
			{	
				text.append(" ... backtrace through instantiation of ") ;
				text.append(name) ;
				text.append(kLineSeparator) ;
			
				if (xml.GetBacktraceAlreadyBacktraced() != null)
				{
					text.append("(We already backtraced through this instantiation.)") ;
					text.append(kLineSeparator) ;
				}
			}
			else
			{
				text.append("...no trace, can't backtrace") ;
				text.append(kLineSeparator) ;
			}
		} else if (xml.IsTagBacktraceResult())
		{
			text.append("For result preference ") ;	
		} else if (xml.IsTagLocal())
		{
			text.append("For local ") ;
		} else if (xml.IsTagProhibitPreference())
		{
			text.append("For prohibit preference ") ;
		} else if (xml.IsTagGrounds())
		{
			text.append("  -->Grounds:") ;
			text.append(kLineSeparator) ;			
		} else if (xml.IsTagPotentials())
		{
			text.append("  -->Potentials:") ;
			text.append(kLineSeparator) ;
		} else if (xml.IsTagLocals())
		{
			if (parentTagName.equals(sml_Names.getKTagBacktrace()))
			{
				text.append("  -->Locals:") ;
				text.append(kLineSeparator) ;							
			}
			else
			{
				text.append("*** Tracing Locals ***") ;
				text.append(kLineSeparator) ;
			}
		} else 	if (xml.IsTagWme())
		{
			String output = getWmeText(agent, xml) ;
			text.append("     ") ;
			text.append(output) ;
			text.append(kLineSeparator) ;
		}
		else if (xml.IsTagNegated())
		{
			text.append("  -->Negated:") ;
			
			String output = getConditionsText(agent, xml) ;
			text.append(output) ;
			text.append(kLineSeparator) ;
			
			// BADBAD: The conditions currently consume the list of children
			// so we have to return
			return text.toString();
		} else if (xml.IsTagNots())
		{
			text.append("  -->Nots:") ;
			text.append(kLineSeparator) ;
		} else if (xml.IsTagNot())
		{
			String symbol1 = xml.GetBacktraceSymbol1() ;
			String symbol2 = xml.GetBacktraceSymbol2() ;
			text.append("    ") ;
			text.append(symbol1) ;
			text.append(" <> ") ;
			text.append(symbol2) ;
		}
		else if (xml.IsTagPreference())
		{
			String output = getPreferenceText(agent, xml) ;
			text.append(output) ;
			text.append(kLineSeparator) ;
		} else if (xml.IsTagGroundedPotentials())
		{
			text.append("*** Tracing Grounded Potentials ***") ;
			text.append(kLineSeparator) ;
		} else if (xml.IsTagUngroundedPotentials())
		{
			text.append("*** Tracing Ungrounded Potentials ***") ;
			text.append(kLineSeparator) ;
		} else 
		{
			String tag = xml.GetTagName() ;
			System.out.println("Failed to handle " + tag) ;
		}
		
		int nChildren = xml.GetNumberChildren() ;
		String tag = xml.GetTagName() ;
		
		for (int i = 0 ; i < nChildren ; i++)
		{
			ClientTraceXML child = new ClientTraceXML() ;
			xml.GetChild(child, i) ;
			
			String subText = getBacktraceText(agent, child, tag) ;
			text.append(subText) ;
			
			child.delete() ;
		}
		
		return text.toString() ;
	}
	
	public static String getActionText(Agent agent, ClientTraceXML action)
	{
		StringBuffer text = new StringBuffer() ;

		text.append(kLineProdIndent) ;
		text.append("(") ;
		
	    // <action id="<s1>" action="^foo2 bar2 ^what ever"></action>
		// <action function="some function string"></action>
		String id = action.GetActionId() ;
		if (id != null)
		{
			text.append(id) ;
			text.append(" ") ;
		
			String actions = action.GetAction() ;
			text.append(actions) ;
		}
		else
		{
			String function = action.GetFunction() ;
			if (function != null)
			{
				text.append(function) ;
			}
		}
		
		text.append(")") ;

		return text.toString() ;
	}

	public static String getConditionsText(Agent agent, ClientTraceXML conditions)
	{
		StringBuffer text = new StringBuffer() ;
		
		for (int j = 0 ; j < conditions.GetNumberChildren() ; j++)
		{
			ClientTraceXML conds = new ClientTraceXML() ;
			conditions.GetChild(conds, j) ;
			
			if (conds.IsTagConjunctiveNegationCondition())
			{
				text.append(kLineSeparator) ;
				text.append(getSpaces(kProductionIndent-2)) ;
				text.append("-{") ;
				
				int nChildren = conds.GetNumberChildren() ;
				for (int k = 0 ; k < nChildren ; k++)
				{
					ClientTraceXML cond = new ClientTraceXML() ;
					conds.GetChild(cond, k) ;
					
					if (k != 0)
						text.append(kLineSeparator) ;
					
					String condition = getConditionText(agent, cond, k != 0) ;
					text.append(condition) ;
					
					cond.delete() ;
				}
				
				text.append("}") ;
			}
			else
			{
				text.append(kLineSeparator) ;
				String condition = getConditionText(agent, conds, true) ;
				text.append(condition) ;
			}
			
			conds.delete() ;
		}
		
		return text.toString() ;
	}
	
	public static String getProductionText(Agent agent, ClientTraceXML xmlProd)
	{
		StringBuffer text = new StringBuffer() ;
				
		//sp {chunk-2*d9*tie*2
		//   :chunk
		text.append("sp {") ;
		text.append(xmlProd.GetProductionName()) ;
		text.append(kLineProdIndent) ;

		String doc = xmlProd.GetProductionDoc() ;
		if (doc != null)
		{
			text.append(doc) ;
			text.append(kLineProdIndent) ;
		}
		
		String support = xmlProd.GetProductionDeclaredSupport() ;
		if (support != null)
		{
			text.append(support) ;
			text.append(kLineProdIndent) ;
		}
		
		text.append(xmlProd.GetProductionType()) ;
		
		for (int i = 0 ; i < xmlProd.GetNumberChildren() ; i++)
		{
			ClientTraceXML side = new ClientTraceXML() ;
			xmlProd.GetChild(side, i) ;
			
			if (side.IsTagConditions())
			{
				text.append(getConditionsText(agent, side)) ;
			}
			else if (side.IsTagActions())
			{
				text.append(kLineProdIndent) ;
				text.append("-->") ;
				
				for (int j = 0 ; j < side.GetNumberChildren() ; j++)
				{
					ClientTraceXML action = new ClientTraceXML() ;
					side.GetChild(action, j) ;

					String actionText = getActionText(agent, action) ;
					text.append(actionText) ;
					
					action.delete() ;
				}
			}
			
			side.delete() ;
		}
		
		// Close the production
		text.append(kLineSeparator) ;
		text.append("}") ;
		
		return text.toString() ;
	}
		
	public static String getPreferenceProductionText(Agent agent, ClientTraceXML xmlTrace)
	{
		return getPreferenceText(agent, xmlTrace) ;
	}
	
	public static String getPreferenceText(Agent agent, ClientTraceXML xmlTrace)
	{
		StringBuffer text = new StringBuffer() ;
		
		text.append("(") ;
		text.append(xmlTrace.GetPreferenceID()) ;
		text.append(" ^") ;
		text.append(xmlTrace.GetPreferenceAttribute()) ;
		text.append(" ") ;
		text.append(xmlTrace.GetPreferenceValue()) ;
		text.append(" ") ;
		text.append(xmlTrace.GetPreferenceType()) ;
		
		// For binary prefs we get a second object
		String referent = xmlTrace.GetPreferenceReferent() ;
		if (referent != null)
		{
			text.append(" ") ;
			text.append(referent) ;
		}
		
		String support = xmlTrace.GetPreferenceOSupported() ;
		if (support != null)
		{
			text.append(" ") ;
			text.append(support) ;
		}
		
		text.append(")") ;

		return text.toString() ;
	}
	
	public static String getWmeText(Agent agent, ClientTraceXML wme)
	{
		StringBuffer text = new StringBuffer() ;
		
		text.append("(") ;
		String timetag = wme.GetWmeTimeTag();
		if(timetag != null) {
			text.append(wme.GetWmeTimeTag()) ;
			text.append(": ") ;
		}
		
		text.append(wme.GetWmeID()) ;
		text.append(" ^") ;
		text.append(wme.GetWmeAttribute()) ;
		text.append(" ") ;
		text.append(wme.GetWmeValue()) ;
		
		String pref = wme.GetWmePreference() ;						
		if (pref != null)
		{
			text.append(" ") ;
			text.append(pref) ;
		}
		
		String support = wme.GetPreferenceOSupported() ;
		if (support != null)
		{
			text.append(" ") ;
			text.append(support) ;
		}
		
		text.append(")") ;
		
		return text.toString() ;
	}
	
	public static String getWmeChanges(Agent agent, ClientTraceXML xmlTrace, boolean adding)
	{
		StringBuffer text = new StringBuffer()  ;
		
		for (int i = 0 ; i < xmlTrace.GetNumberChildren() ; i++)
		{
			ClientTraceXML child = new ClientTraceXML() ;
			xmlTrace.GetChild(child, i) ;
			
			if (child.IsTagWme())
			{
				text.append(adding ? "=>WM: " : "<=WM: ") ;
				String output = getWmeText(agent, child) ;
				text.append(output) ;
			}
			
			child.delete() ;
		}
		
		return text.toString() ;

	}
	
	public static String getProductionFiring(Agent agent, ClientTraceXML xmlTrace, boolean firing)
	{
		StringBuffer text = new StringBuffer() ;
		
		// Firing/Retracting <production>
		// followed by timetag or full wme information for LHS
		for (int i = 0 ; i < xmlTrace.GetNumberChildren() ; i++)
		{
			ClientTraceXML child = new ClientTraceXML() ;
			xmlTrace.GetChild(child, i) ;
			if (child.IsTagProduction())
			{
				if (i > 0)
					text.append(kLineSeparator) ;
				
				text.append(firing ? "Firing " : "Retracting ") ;
				text.append(child.GetProductionName()) ;
				
				for (int j = 0 ; j < child.GetNumberChildren() ; j++)
				{
					if (j == 0)
						text.append(kLineSeparator) ;
					
					ClientTraceXML wme = new ClientTraceXML() ;
					child.GetChild(wme, j) ;
					
					if (wme.IsTagWme())
					{
						// See if we have full information or just a time tag
						String id = wme.GetWmeID() ;
						if (id != null)
						{
							if (j > 0)
								text.append(kLineSeparator) ;
							
							text.append("(") ;
							text.append(wme.GetWmeTimeTag()) ;
							text.append(": ") ;
							text.append(id) ;
							text.append(" ^") ;
							text.append(wme.GetWmeAttribute()) ;
							text.append(" ") ;
							text.append(wme.GetWmeValue()) ;
							
							String pref = wme.GetWmePreference() ;						
							if (pref != null)
							{
								text.append(" ") ;
								text.append(pref) ;
							}
							text.append(")") ;
						}
						else
						{
							text.append(wme.GetWmeTimeTag()) ;
							text.append(" ") ;
						}
					}
					
					wme.delete() ;
				}
			}
			child.delete() ;
		}
		
		return text.toString() ;
	}
	
}
