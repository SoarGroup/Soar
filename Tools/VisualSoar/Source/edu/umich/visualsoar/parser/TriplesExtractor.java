package edu.umich.visualsoar.parser;
import java.util.*;
import edu.umich.visualsoar.util.*;

/**
 * This class extracts triples from the passed in soar production
 * it ignores the condition/action side information it also ignores
 * any relation besides equals, it is not sensitive to negations
 * @author Brad Jones
 */
 
public class TriplesExtractor {
//////////////////////////////////////////////////////////////////////////////
// Data Members
/////////////////////////////////////////////////////////////////////////////
	private int d_currentUnnamedVar = 0;
	private List d_triples = new LinkedList();
	private SoarProduction d_soarProduction;
	private Set d_variables = new TreeSet();
	private Set d_stateVariables = new TreeSet();
	private TripleFactory d_tripleFactory;
	private Map d_boundMap;
	
// Not Implemented
	private TriplesExtractor() {}
	
/////////////////////////////////////////////////////////////////////////////
// Constructors
/////////////////////////////////////////////////////////////////////////////
	public TriplesExtractor(SoarProduction soarProduction) {
		d_soarProduction = soarProduction;
		d_tripleFactory = new DefaultTripleFactory();
		extractTriples();
		extractVariables();
		extractStateVariables();
	}
		
/////////////////////////////////////////////////////////////////////////////
// Accessors
/////////////////////////////////////////////////////////////////////////////
	public Iterator triples() {
		return d_triples.iterator();
	}
	
	public Iterator variables() {
		return d_variables.iterator();
	}
	
	public Iterator stateVariables() {
		return d_stateVariables.iterator();
	}
	
	public int getStateVariableCount() {
		return d_stateVariables.size();
	}
	
	public Pair stateVariable() {
		Iterator i = d_stateVariables.iterator();
		if(i.hasNext())
			return (Pair)i.next();
		else
			return null;
		
	}
	
	public int getProductionStartLine() {
		return d_soarProduction.getStartLine();
	}
	
	public String getProductionName() {
		return d_soarProduction.getName();
	}
	
	public boolean isBound(String variable) {
		if(d_boundMap == null) {
			d_boundMap = new TreeMap();
			for(Iterator i = variables(); i.hasNext();) {
				Pair p = (Pair)i.next();
				String varName = p.getString();
				for(Iterator j = triples(); j.hasNext();) {
					CoverageTriple ct = (CoverageTriple)j.next();
					if(varName.equals(ct.getVariable().getString())  || 
					   varName.equals(ct.getAttribute().getString()) || 
					   varName.equals(ct.getValue().getString())) {
						if(ct.isChecking()) {
							d_boundMap.put(varName,Boolean.TRUE);
						}
						else {
							if(d_boundMap.get(varName) == null) {
								d_boundMap.put(varName,Boolean.FALSE);
							}
						}
					}
				}
			}
		}
		return ((Boolean)d_boundMap.get(variable)).booleanValue();
	}
	
	
/////////////////////////////////////////////////////////////////////////////
// Manipulators
/////////////////////////////////////////////////////////////////////////////	
	public void sortTriples(List errors) {
		if(d_stateVariables.size() != 1) {
			errors.add(d_soarProduction.getName() + "(" + d_soarProduction.getStartLine() + "): " 
					   + "productions with only one state variable can be checked.");
			d_triples = new LinkedList();
			d_variables.clear();
			d_stateVariables.clear();
			return;
		}
		List sorted = new LinkedList();
		edu.umich.visualsoar.util.Queue variables = new QueueAsLinkedList();
		variables.enqueue(d_stateVariables.iterator().next());
		while(!variables.isEmpty()) {
			String currentVar = (String)variables.dequeue();
			Iterator i = d_triples.iterator();
			while(i.hasNext()) {
				Triple t = (Triple)i.next();
				if (t.getVariable().equals(currentVar)) {
					sorted.add(t);
					i.remove(); 	
					if (TripleUtils.isVariable(t.getValue().getString()))
						variables.enqueue(t.getValue());
				}
			}
		}
		
		if (!d_triples.isEmpty()) {
			errors.add(d_soarProduction.getName() + "(" + d_soarProduction.getStartLine() 
						+ "): variable(s) not connected to state");
		}
		d_triples = sorted;
	}
	
	// Implementation Functions
	private void extractTriples() {
		// Extract Triples from the condition side
		Iterator i = d_soarProduction.getConditionSide().getConditions();
		while(i.hasNext())
			d_triples.addAll(extractTriples(((Condition)i.next()).getPositiveCondition()) );
			
		// Extract Triples from the action side
		i = d_soarProduction.getActionSide().getActions();
		while(i.hasNext()) {
			Action a = (Action)i.next();
			if(a.isVarAttrValMake()) 
				d_triples.addAll(extractTriples(a.getVarAttrValMake()));
		}		
	}
	
	private List extractTriples(PositiveCondition pc) {
		// If the this positive condition is a conjunctions then extract
		// all the positive conditions out of it and recursively
		// interpret those
		if(pc.isConjunction()) {
			List triples = new LinkedList();
			Iterator i = pc.getConjunction();
			while(i.hasNext()) 
				triples.addAll(extractTriples(((Condition)i.next()).getPositiveCondition()));
			return triples;
		}
		else {
		// Just extract the condition for one identifier
			return extractTriples(pc.getConditionForOneIdentifier());
		}
	}
	
	private List extractTriples(ConditionForOneIdentifier cfoi) {
		// This function is long and complicated so I'll explain it the best
		// that I can
		List triples = new LinkedList();
		// Get all the attribute Value tests
		Iterator i = cfoi.getAttributeValueTests();
		boolean hasState = cfoi.hasState();
		
		// For all the attribute value tests
		while(i.hasNext()) {
			Pair variable = cfoi.getVariable();
			List attributes = null;
			AttributeValueTest avt = (AttributeValueTest)i.next();
			
			// Get the attribute chain
			Iterator k = avt.getAttributeTests();
			while(k.hasNext()) {
				AttributeTest at = (AttributeTest)k.next();
				
				// First time switch
				if(attributes == null) {
					attributes = extract(at.getTest());
				}
				else {
				
				// Ok, they are doing the '.' thing so create a variable
				// value and march on down the line
					List newAttributes = extract(at.getTest());
					Pair newVariable = getNextUnnamedVar();
					Iterator j = attributes.iterator();
					while(j.hasNext()) {
						Pair attr = (Pair)j.next();
						triples.add(d_tripleFactory.createTriple(variable,attr,newVariable,hasState,true, true));
					}
					attributes = newAttributes;
					variable = newVariable;
					hasState = false;
				}
			}
			
			// In case they didn't have any attributes, put a variable one
			// in its place, (my understanding is that this is exactly what
			// soar does)
			if(attributes == null) {
				attributes = new LinkedList();
				attributes.add(getNextUnnamedVar());
			}
			
			// Ok get all the values that we are checking
			List values = null;
			Iterator j = avt.getValueTests();
			while(j.hasNext()) {
				ValueTest vt = (ValueTest)j.next();
				if(values == null)
					values = extract(vt.getTest());
				else
					values.addAll(extract(vt.getTest()));
			}
			
			// In case they didn't check for any values, put a variable in
			// there, my understanding is that soar does the exact same thing
			if(values == null) {
				values = new LinkedList();
				values.add(getNextUnnamedVar());
			}
			
			// Put the attributes and variables together with the 
			// variables into triples
			k = attributes.iterator();
			while(k.hasNext()) {
				Pair attr = (Pair)k.next();
				j = values.iterator();
				while(j.hasNext()) {
					Pair val = (Pair)j.next();
					triples.add(d_tripleFactory.createTriple(variable,attr,val,hasState,true, true));
				}
			}
		}
		return triples;	
	}
	
	private List extract(Test t) {
		if(t.isConjunctiveTest()) {
			List strings = new LinkedList();
			Iterator i = t.getConjunctiveTest().getSimpleTests();
			while(i.hasNext()) {
				strings.addAll(extract((SimpleTest)i.next()));
			}
			return strings;
		}
		else
			return extract(t.getSimpleTest());
	}
	
	private List extract(SimpleTest simpleTest) {
		if(simpleTest.isDisjunctionTest()) {
			List strings = new LinkedList();
			Iterator i = simpleTest.getDisjunctionTest().getConstants();
			while(i.hasNext()) {
				Constant c = (Constant)i.next();
				strings.add(c.toPair());
			}
			return strings;
		}
		else {
			SingleTest st = simpleTest.getRelationalTest().getSingleTest();
			List strings = new LinkedList();
			if(st.isConstant())
				strings.add(st.getConstant().toPair());
			else
				strings.add(st.getVariable());
			return strings;
		}
	}
	
	private List extractTriples(VarAttrValMake vavm) {
		List triples = new LinkedList();
		Iterator i = vavm.getAttributeValueMakes();
		while(i.hasNext()) {
			Pair variable = vavm.getVariable();
			Pair attributeMakes = null;
			AttributeValueMake avm = (AttributeValueMake)i.next();
			Iterator j = avm.getRHSValues();
			while(j.hasNext()) {
				if(attributeMakes == null)
					attributeMakes = extract((RHSValue)j.next());
				else {
					Pair newAttributeMakes = extract((RHSValue)j.next());
					Pair newVariable = getNextUnnamedVar();
					triples.add(d_tripleFactory.createTriple(variable,attributeMakes,newVariable,false,false, false));
					attributeMakes = newAttributeMakes;
					variable = newVariable;
				}
			}
			j = avm.getValueMakes();
			while(j.hasNext()) {
				ValueMake vm = (ValueMake)j.next();
				Pair value = extract(vm.getRHSValue());
				triples.add(d_tripleFactory.createTriple(variable,attributeMakes,value,false,false, false));
			}
		}
		return triples;
	}
	
	private Pair extract(RHSValue rhsValue) {
		if(rhsValue.isFunctionCall())
			return getNextUnnamedVar();
		if(rhsValue.isVariable())
			return rhsValue.getVariable();
		return rhsValue.getConstant().toPair();
	}
	
	private Pair getNextUnnamedVar() {
		return new Pair("< " + d_currentUnnamedVar++ + ">",-1);
	}
	
	private void extractVariables() {
		Iterator i = d_triples.iterator();
		while(i.hasNext()) {
			Triple t = (Triple)i.next();
			d_variables.add(t.getVariable());
			if(TripleUtils.isVariable(t.getAttribute().getString()))
				d_variables.add(t.getAttribute());
			if(TripleUtils.isVariable(t.getValue().getString()))
				d_variables.add(t.getValue());
		}
	}
	
	private void extractStateVariables() {
		Iterator i = d_triples.iterator();
		while(i.hasNext()) {
			Triple t = (Triple)i.next();
			if(t.hasState())
				d_stateVariables.add(t.getVariable());
		}
	}
}
