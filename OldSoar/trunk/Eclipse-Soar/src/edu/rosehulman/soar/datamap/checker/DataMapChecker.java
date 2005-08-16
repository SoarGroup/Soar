/**
 *
 * @file DataMapMatcher.java
 * @date Jun 1, 2004
 */
package edu.rosehulman.soar.datamap.checker;



import edu.rosehulman.soar.datamap.*;
import edu.rosehulman.soar.datamap.items.*;
import edu.umich.visualsoar.parser.*;

import org.eclipse.core.runtime.*;
import org.eclipse.core.resources.*;
import org.eclipse.ui.texteditor.MarkerUtilities;

import java.io.*;
import java.util.*;


/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class DataMapChecker {
	public static final String PROBLEM_MARKER = "edu.rosehulman.soar.DatamapProblem";
	
	/**
	 * Determines if the given source file matches the datamap.
	 * 
	 * @param source The Soar source file.
	 * @param dm The Datamap associated with it.
	 */
	public static void matches(IFile source, DataMap dm) {
		try {
			
			SoarParser parser =
				new SoarParser (new InputStreamReader(source.getContents()));
			
			Vector productions = parser.VisualSoarFile(); 
			
			matches(source, productions, dm);
			
			
		} catch (CoreException e) {
			e.printStackTrace();
		} catch (ParseException e) {
			System.out.println("parsing error!");
		}
	}
	
	/**
	 * Takes an already parsed source file productions contained therein
	 *  and determines if they match the datamap. This function find the 
	 *  datamap associated with this file on its own, and if there is none,
	 *  simply stops. 
	 * 
	 * @param source The source file.
	 * @param productions The productions resulting in the source file being parsed.
	 */
	public static void matches(IFile source, Vector productions) {
		DataMap dm = DataMap.getAssociatedDatamap(source);
		
		if (dm != null) {
			matches(source, productions, dm);
		}
	}
	
	/**
	 * Determines if an already parsed source file matches the datamap. This
	 *  is the function called by the other forms of <code>matches</code>.
	 *  The source file is marked in any place where it does not match up with 
	 *  the datamap.
	 * 
	 * @param source The source file.
	 * @param productions The productions contained in the source file.
	 * @param dm The DataMap associated with this source file.
	 */
	public static void matches(IFile source, Vector productions, DataMap dm) {
		try {
			DMItem start = dm.getAssociatedVertex(source);
			
			source.deleteMarkers(DataMapChecker.PROBLEM_MARKER, false, 0);
			
			Enumeration en = productions.elements();
				
			while (en.hasMoreElements()) {
				ArrayList names = new ArrayList();
				
				SoarProduction sp = (SoarProduction) en.nextElement();
				
				TriplesExtractor te = new TriplesExtractor(sp);
				
				Enumeration e = new EnumerationIteratorWrapper(te.triples());
					
					
				while(e.hasMoreElements()) {
					Triple currentTriple = (Triple)e.nextElement();
					
					String val = currentTriple.getValue().getString();	
					
					//This name is part of the path
					names.add(currentTriple.getAttribute().getString());
					
					if (val.matches("<\\s[0-9]*>")) {
						// This is part of a chain of attribute names.
						// Don't have much to do here, it would seem.
						// Just keep looping until we reach the else.
							
					} else {

						ArrayList nodes = dm.find(names, start);
						
						if (nodes.size() == 0) {
							// It doesn't exist! The horror!
							
							HashMap attr = new HashMap();
							MarkerUtilities.setLineNumber(attr, currentTriple.getLine());
							attr.put(IMarker.SEVERITY, new Integer(IMarker.SEVERITY_WARNING));
							String msg = makeName(names) + " does not exist in the datamap.";
							MarkerUtilities.setMessage(attr, msg);
							
							//MarkerUtilities.createMarker(source, attr, IMarker.PROBLEM);
							MarkerUtilities.createMarker(source, attr, DataMapChecker.PROBLEM_MARKER);
							
						} else {
							// Sure, maybe it exists. But does that satisfy you?
							boolean satisfied = false;
							
							for (int i=0; i<nodes.size(); ++i) {
								DMItem node = (DMItem) nodes.get(i);
								if (node.satisfies(currentTriple)) {
									satisfied = true;
								}
							}
							
							if (!satisfied) { // your money back!
								// Invalid value assignment
								HashMap attr = new HashMap();
								MarkerUtilities.setLineNumber(attr, currentTriple.getLine());
								attr.put(IMarker.SEVERITY, new Integer(IMarker.SEVERITY_WARNING));
								String msg = makeName(names) 
									+ " does not accept a value of "
									+ currentTriple.getValue().getString();
								MarkerUtilities.setMessage(attr, msg);
								
								//MarkerUtilities.createMarker(source, attr, IMarker.PROBLEM);
								MarkerUtilities.createMarker(source, attr, DataMapChecker.PROBLEM_MARKER);
								
							}
							
						} // else
						
						names.clear(); // clear out the path for the next attribute
						
					} // else
				} // while
					
				
			} // while
			
			
		} catch (CoreException e) {
			e.printStackTrace();
		}
	} // matches ( ... )
	
	
	/**
	 * Puts together the path list into a readable attribute name.
	 *  For example, an input of {"io", "input-link", "pie"} will
	 *  yield "io.input-link.pie"
	 *  
	 * @param names An attribute path
	 * @return The attribute's path with the periods back in place.
	 */
	private static String makeName(ArrayList names) {
		String ret = "";
		
		if (names.size() > 0) {
			ret += names.get(0);
								
			for (int i=1; i<names.size(); ++i) {
				ret += "." + names.get(i);
			} // for
		} // if
		
		return ret;
	}
	
	
	
	/*
	public List checkProduction(DataMap dm, DMIdentifier item, SoarProduction sp) {
		TriplesExtractor triplesExtractor = new TriplesExtractor(sp);
		
		DefaultCheckerErrorHandler dceh = new DefaultCheckerErrorHandler(sp.getName(),sp.getStartLine());
		
		
		Map varMap = matches(dm, item, triplesExtractor, dceh);
		if(varMap != null) {
			Set keySet = varMap.keySet();
			Iterator vars = keySet.iterator();
			while(vars.hasNext()) {
				String varKey = (String)vars.next();
				Set value = (Set)varMap.get(varKey);
				if(value.isEmpty()) 
					dceh.variableNotMatched(varKey); 
			}
		}
		
		return dceh.getErrors();
	}
	
	
	
	
	// taken from DataMapMatcher
	public static Map matches(DataMap dataMap, DMIdentifier startVertex,TriplesExtractor triplesExtractor,MatcherErrorHandler meh) {
		Map varMap = new HashMap();
		Iterator iter = triplesExtractor.variables();
		while(iter.hasNext()) {
			varMap.put(((Pair)iter.next()).getString(),new HashSet());
		}
		
		// Take care of the first Variable
		// Make sure there are the right number of state variables
		int result = triplesExtractor.getStateVariableCount();
		if(result == 0) {
			meh.noStateVariable();
			return null;
		}
		else if(triplesExtractor.getStateVariableCount() > 1) {
			meh.tooManyStateVariables();
			return null;
		}

		Pair stateVar = triplesExtractor.stateVariable();
		Set stateSet = (Set)varMap.get(stateVar.getString());
		stateSet.add(startVertex);
		
		Enumeration e = new EnumerationIteratorWrapper(triplesExtractor.triples());
		while(e.hasMoreElements()) {
			Triple currentTriple = (Triple)e.nextElement();
			if ( (currentTriple.getAttribute().getString().equals("operator"))  
				&& (TripleUtils.isFloat(currentTriple.getValue().getString())
				||  TripleUtils.isInteger(currentTriple.getValue().getString()) ) ) {
			  continue;
			}
			if (!addConstraint(dataMap,currentTriple,varMap)) {
			  meh.badConstraint(currentTriple);
			}
		}
		return varMap;
	}
	
	
	private static boolean addConstraint(DataMap dataMap,Triple triple, Map match) {
		Set varSet = (Set)match.get(triple.getVariable().getString());
		boolean matched = false;
		// for every possible start
		Enumeration e = new EnumerationIteratorWrapper(varSet.iterator());
		while(e.hasMoreElements()) {
			Object o = e.nextElement();

			// In case they try to use a attribute variable as 
			// soar identifier
			if(!(o instanceof SoarVertex))
				continue;
			SoarVertex currentSV = (SoarVertex)o;

			// Get all the edges from the start
			Enumeration edges = dataMap.emanatingEdges(currentSV);
			while(edges.hasMoreElements()) {
				NamedEdge currentEdge = (NamedEdge)edges.nextElement();
				if (currentEdge.satisfies(triple)) {
   
					// Used for the Datamap Searches for untested/uncreated elements
					 if(triple.isCondition()) {
						currentEdge.tested();
					 } else {
						currentEdge.created();
					 }

					if (!matched)
						matched = true;
					if (TripleUtils.isVariable(triple.getAttribute().getString())) {
						Set attrSet = (Set)match.get(triple.getAttribute().getString());
						attrSet.add(currentEdge.getName());
					}	
					if (TripleUtils.isVariable(triple.getValue().getString())) {
						Set valSet = (Set)match.get(triple.getValue().getString());
						valSet.add(currentEdge.V1());
					}
				}
			}
		}
		return matched;
	}
	
	/*private static boolean addConstraint(SoarWorkingMemoryModel dataMap,Triple triple, Map match) {
		Set varSet = (Set)match.get(triple.getVariable().getString());
		boolean matched = false;
		// for every possible start
		Enumeration e = new EnumerationIteratorWrapper(varSet.iterator());
		while(e.hasMoreElements()) {
			Object o = e.nextElement();

			// In case they try to use a attribute variable as 
			// soar identifier
			if(!(o instanceof SoarVertex))
				continue;
			SoarVertex currentSV = (SoarVertex)o;

			// Get all the edges from the start
			Enumeration edges = dataMap.emanatingEdges(currentSV);
			while(edges.hasMoreElements()) {
				NamedEdge currentEdge = (NamedEdge)edges.nextElement();
				if (currentEdge.satisfies(triple)) {
   
		  // Used for the Datamap Searches for untested/uncreated elements
			 if(triple.isCondition())
				currentEdge.tested();
			 else
				currentEdge.created();

					if (!matched)
						matched = true;
					if (TripleUtils.isVariable(triple.getAttribute().getString())) {
						Set attrSet = (Set)match.get(triple.getAttribute().getString());
						attrSet.add(currentEdge.getName());
					}	
					if (TripleUtils.isVariable(triple.getValue().getString())) {
						Set valSet = (Set)match.get(triple.getValue().getString());
						valSet.add(currentEdge.V1());
					}
				}
			}
		}
		return matched;
	}*/
	
	
	/*
	// taken from the original DataMapChecker
	public static void check(SoarWorkingMemoryModel dataMap,SoarIdentifierVertex startVertex,TriplesExtractor triplesExtractor,CheckerErrorHandler ceh) {
		Map varMap = DataMapMatcher.matches(dataMap,startVertex,triplesExtractor,ceh);
		if(varMap != null) {
			Set keySet = varMap.keySet();
			Iterator vars = keySet.iterator();
			while(vars.hasNext()) {
				String varKey = (String)vars.next();
				Set value = (Set)varMap.get(varKey);
				if(value.isEmpty()) 
					ceh.variableNotMatched(varKey); 
			}
		}
	} */
	
	
	/*public List checkProduction(SoarIdentifierVertex sv, SoarProduction sp) {
		TriplesExtractor triplesExtractor = new TriplesExtractor(sp);
		DefaultCheckerErrorHandler dceh = new DefaultCheckerErrorHandler(sp.getName(),sp.getStartLine());
		DataMapChecker.check(this,sv,triplesExtractor,dceh);
		return dceh.getErrors();
	} */
}
