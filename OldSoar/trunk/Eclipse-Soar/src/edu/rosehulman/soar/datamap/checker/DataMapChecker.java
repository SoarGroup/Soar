/**
 *
 * @file DataMapMatcher.java
 * @date Jun 1, 2004
 */
package edu.rosehulman.soar.datamap.checker;



import edu.rosehulman.soar.datamap.*;
import edu.umich.visualsoar.parser.*;

import org.eclipse.core.runtime.*;
import org.eclipse.core.resources.*;


import java.io.*;
import java.util.*;


/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class DataMapChecker {
	public static void matches(IFile source, DataMap dm) {
		ArrayList errors = new ArrayList();
		
		

		
		
		try {
			
			SoarParser parser =
				new SoarParser (new InputStreamReader(source.getContents()));
			
			Vector productions = parser.VisualSoarFile(); 
			
			Enumeration enum = productions.elements();
			
			System.out.println("Parsed!");
			
			while (enum.hasMoreElements()) {
				Enumeration e;
				SoarProduction sp = (SoarProduction) enum.nextElement();
				
				System.out.println();
				System.out.println(sp.getName() + ":" +  sp.getProductionType());
				
				
				TriplesExtractor te = new TriplesExtractor(sp);
				
				System.out.println(te.getStateVariableCount());
				System.out.println(te.stateVariable().getString());
				
				
				e = new EnumerationIteratorWrapper(te.triples());
				while(e.hasMoreElements()) {
					Triple currentTriple = (Triple)e.nextElement();
					System.out.println("attribute: " + currentTriple.getAttribute().getString());
					System.out.println("val: " + currentTriple.getValue().getString());
					System.out.println("var: " + currentTriple.getVariable().getString());
				
				}
				
				
			}
			
			
			/*public void checkProductions(OperatorNode on,Vector productions, java.util.List errors) {

				// Find the state that these productions should be checked against
				SoarIdentifierVertex siv = on.getStateIdVertex();
				if(siv == null)
					siv = WorkingMemory.getTopstate();
				Enumeration e = productions.elements();

				while(e.hasMoreElements()) {
					SoarProduction sp = (SoarProduction)e.nextElement();
					errors.addAll(WorkingMemory.checkProduction(siv,sp));
				}
			} */
			
			
		} catch (CoreException e) {
			e.printStackTrace();
		} catch (ParseException e) {
			System.out.println("parsing error!");
				
		}
		
		
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
