package edu.umich.soar.qna.math;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class DivQueryState extends BinaryMathQueryState {

	public boolean initialize(String querySource, Map<Object, List<Object>> queryParameters) {
		boolean returnVal = false;
		
		if (super.initialize(querySource, queryParameters)) {
			if ((operand1 instanceof Long) && (operand2 instanceof Long)) {
				returnVal = true;
			} else {
				hasComputed = true;
			}
		}
		
		return returnVal;
	}

	public Map<String, List<Object>> next() {
		if (!hasComputed) {
			hasComputed = true;
			HashMap<String, List<Object>> returnVal = new HashMap<String, List<Object>>();
			List<Object> newList = new LinkedList<Object>();
			
			newList.add(new Long(((Long) operand1).longValue() / ((Long) operand2).longValue()));
			returnVal.put("result", newList);
			
			return returnVal;
		}
		
		return null;
	}

}
