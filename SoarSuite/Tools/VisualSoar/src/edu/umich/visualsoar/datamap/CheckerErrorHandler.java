package edu.umich.visualsoar.datamap;

/**
 This is an interface for error callbacks for the Checker
 @author Brad Jones
*/
public interface CheckerErrorHandler extends MatcherErrorHandler {

	/**
	 * This function is called when it has been determined that a
	 * variable has not been matched
	 * @param variable the variable that has not been matched
	*/
	void variableNotMatched(String variable);
}
