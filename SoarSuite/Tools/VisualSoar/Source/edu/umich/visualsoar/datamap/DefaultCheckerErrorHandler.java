package edu.umich.visualsoar.datamap;

/**
 * This class is notified of errors by the datamap checker
 * @author Brad Jones
 */

public class DefaultCheckerErrorHandler extends DefaultMatcherErrorHandler implements CheckerErrorHandler {
	private String d_errorBegin;

///////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////
	public DefaultCheckerErrorHandler(String productionName,int startLine) {
		super(productionName,startLine);
		d_errorBegin = "" + d_productionName + "(" + startLine + "): ";
	}


//////////////////////////////////////////////////
// Modifiers
//////////////////////////////////////////////////
	public void variableNotMatched(String variable) {
		d_errors.add(d_errorBegin + "variable " + variable + " could not be matched in production");
	}
}
