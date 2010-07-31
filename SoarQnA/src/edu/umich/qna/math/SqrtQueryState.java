package edu.umich.qna.math;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class SqrtQueryState extends UnaryMathQueryState {

	@Override
	public Map<String, List<Object>> next() {
		if (!hasComputed) {
			hasComputed = true;
			HashMap<String, List<Object>> returnVal = new HashMap<String, List<Object>>();
			List<Object> newList = new LinkedList<Object>();
			
			newList.add(new Double(Math.sqrt(((Number) operand1).doubleValue())));
			returnVal.put("result", newList);
			
			return returnVal;
		}
		
		return null;
	}

}
