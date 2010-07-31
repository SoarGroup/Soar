package edu.umich.qna.math;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class DivQueryState extends BinaryMathQueryState {

	@Override
	public boolean initialize(String querySource, Map<Object, List<Object>> queryParameters) {
		boolean returnVal = false;
		
		if (super.initialize(querySource, queryParameters)) {
			if ((operand1 instanceof Integer) && (operand2 instanceof Integer)) {
				returnVal = true;
			} else {
				hasComputed = true;
			}
		}
		
		return returnVal;
	}

	@Override
	public Map<String, List<Object>> next() {
		if (!hasComputed) {
			hasComputed = true;
			HashMap<String, List<Object>> returnVal = new HashMap<String, List<Object>>();
			List<Object> newList = new LinkedList<Object>();
			
			newList.add(new Integer(((Integer) operand1).intValue() / ((Integer) operand2).intValue()));
			returnVal.put("result", newList);
			
			return returnVal;
		}
		
		return null;
	}

}
