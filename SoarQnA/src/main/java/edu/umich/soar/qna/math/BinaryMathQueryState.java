package edu.umich.soar.qna.math;

import java.util.List;
import java.util.Map;

import edu.umich.soar.qna.ComputationalQueryState;

public abstract class BinaryMathQueryState extends ComputationalQueryState {
	Object operand1;
	Object operand2;

	public boolean initialize(String querySource, Map<Object, List<Object>> queryParameters) {
		boolean returnVal = false;
		
		if ((queryParameters.size() == 2) && (queryParameters.containsKey("operand1")) && (queryParameters.containsKey("operand2"))) {
			List<Object> tempList;
			
			tempList = queryParameters.get("operand1");
			if ((tempList.size() == 1) && (tempList.iterator().next() instanceof Number)) {
				operand1 = tempList.iterator().next();
				
				tempList = queryParameters.get("operand2");
				if ((tempList.size() == 1) && (tempList.iterator().next() instanceof Number)) {
					operand2 = tempList.iterator().next();
					
					returnVal = true;
					hasComputed = false;
				}
			}
		}
		
		return returnVal;
	}
}
