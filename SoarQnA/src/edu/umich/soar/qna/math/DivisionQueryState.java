package edu.umich.soar.qna.math;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class DivisionQueryState extends BinaryMathQueryState {
	@Override
	public Map<String, List<Object>> next() {
		if (!hasComputed) {
			hasComputed = true;
			HashMap<String, List<Object>> returnVal = new HashMap<String, List<Object>>();
			List<Object> newList = new LinkedList<Object>();
			
			if ((operand1 instanceof Integer) && (operand2 instanceof Integer) && (((Integer) operand1).intValue() % ((Integer) operand2).intValue() == 0)) {
				newList.add(new Integer(((Integer) operand1).intValue()/((Integer) operand2).intValue()));
			} else {
				newList.add(new Double(((Number) operand1).doubleValue()/((Number) operand2).doubleValue()));
			}
			returnVal.put("result", newList);
			
			return returnVal;
		}
		
		return null;
	}
}
