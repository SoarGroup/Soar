package edu.umich.soar.qna.math;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class NegationQueryState extends UnaryMathQueryState {	
	MultiplicationQueryState multQueryState;
	
	NegationQueryState() {
		multQueryState = null;
	}
	
	public boolean initialize(String querySource, Map<Object, List<Object>> queryParameters) {
		boolean returnVal = false;
		
		if (super.initialize(querySource, queryParameters)) {
			Map<Object, List<Object>> augmentedParameters = new HashMap<Object, List<Object>>(queryParameters);
			List<Object> op2 = new LinkedList<Object>();
			op2.add(new Long(-1));
			augmentedParameters.put("operand2", op2);
			
			multQueryState = new MultiplicationQueryState();
			returnVal = multQueryState.initialize(querySource, augmentedParameters);
		}
		
		return returnVal;
	}
	
	public Map<String, List<Object>> next() {
		Map<String, List<Object>> returnVal = null;
		
		if (!hasComputed && (multQueryState != null) && multQueryState.hasNext()) {
			hasComputed = true;
			returnVal = multQueryState.next();
		}
		
		return returnVal;
	}
}
