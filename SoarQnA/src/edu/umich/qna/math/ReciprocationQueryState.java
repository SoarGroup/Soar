package edu.umich.qna.math;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class ReciprocationQueryState extends UnaryMathQueryState {
	DivisionQueryState divQueryState;
	
	ReciprocationQueryState() {
		divQueryState = null;
	}
	
	@Override
	public boolean initialize(String querySource, Map<Object, List<Object>> queryParameters) {
		boolean returnVal = false;
		
		if (super.initialize(querySource, queryParameters)) {
			Map<Object, List<Object>> augmentedParameters = new HashMap<Object, List<Object>>();
			List<Object> op1 = new LinkedList<Object>();
			op1.add(new Integer(1));
			augmentedParameters.put("operand1", op1);
			augmentedParameters.put("operand2", queryParameters.get("operand1"));
			
			divQueryState = new DivisionQueryState();
			returnVal = divQueryState.initialize(querySource, augmentedParameters);
		}
		
		return returnVal;
	}
	
	@Override
	public Map<String, List<Object>> next() {
		Map<String, List<Object>> returnVal = null;
		
		if (!hasComputed && (divQueryState != null) && divQueryState.hasNext()) {
			hasComputed = true;
			returnVal = divQueryState.next();
		}
		
		return returnVal;
	}
}
