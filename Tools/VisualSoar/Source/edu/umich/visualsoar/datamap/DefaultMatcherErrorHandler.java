package edu.umich.visualsoar.datamap;
import java.util.*;

/**
 * This class implements the default behavior that corresponds to the MatcherErrorHandler
 * @author Brad Jones
 */

public class DefaultMatcherErrorHandler implements MatcherErrorHandler {
//////////////////////////////////////////////////////////
// Data Members
//////////////////////////////////////////////////////////
	protected String d_productionName;
	protected List d_errors = new LinkedList();
	protected int startLine;

// Not Implemented
	private DefaultMatcherErrorHandler() {}
//////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////
	public DefaultMatcherErrorHandler(String productionName,int productionStartLine) {
		d_productionName = productionName;
		startLine = productionStartLine;
	}
	
//////////////////////////////////////////////////////////
// Accessors
//////////////////////////////////////////////////////////
	public List getErrors() {
		return d_errors;
	}
	
//////////////////////////////////////////////////////////
// Modifiers
//////////////////////////////////////////////////////////
	public void badConstraint(edu.umich.visualsoar.parser.Triple triple) {
		String errorBegin = "" + d_productionName + "(" + triple.getLine() + "): ";
		d_errors.add(errorBegin + "could not match constraint " + triple + " in production");
	}

  public void generatedIdentifier(edu.umich.visualsoar.parser.Triple triple, String element) {
    String errorBegin = "" + d_productionName + "(" + triple.getLine() + "): ";
    d_errors.add(errorBegin + "Added Identifier '" + element + "' to the datamap to match constraint " + triple);
  }

  public void generatedInteger(edu.umich.visualsoar.parser.Triple triple, String element) {
    String errorBegin = "" + d_productionName + "(" + triple.getLine() + "): ";
    d_errors.add(errorBegin + "Added Integer '" + element + "' to the datamap to match constraint " + triple);
  }

  public void generatedFloat(edu.umich.visualsoar.parser.Triple triple, String element) {
    String errorBegin = "" + d_productionName + "(" + triple.getLine() + "): ";
    d_errors.add(errorBegin + "Added Float '" + element + "' to the datamap to match constraint " + triple);
  }

  public void generatedEnumeration(edu.umich.visualsoar.parser.Triple triple, String element) {
    String errorBegin = "" + d_productionName + "(" + triple.getLine() + "): ";
    d_errors.add(errorBegin + "Added Enumeration '" + element + "' to the datamap to match constraint " + triple);
  }

  public void generatedAddToEnumeration(edu.umich.visualsoar.parser.Triple triple, String attribute, String value) {
    String errorBegin = "" + d_productionName + "(" + triple.getLine() + "): ";
    d_errors.add(errorBegin + "Added value '" + value + "' to the enumeration '" + attribute + "' to match constraint " + triple);
  }

	public void noStateVariable() {
		String errorBegin = "" + d_productionName + "(" + startLine +"): ";
		d_errors.add(errorBegin + "no state variable in production");
	}
	
	public void tooManyStateVariables() {
		String errorBegin = "" + d_productionName + "(" + startLine +"): ";
		d_errors.add(errorBegin + "too many state variables in production");
	
	}

}
