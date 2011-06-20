package edu.umich.soar.qna.dice;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import edu.umich.dice.LiarsDice.Predicate;
import edu.umich.soar.qna.ComputationalQueryState;

public class ComputeProbabilityQueryState extends ComputationalQueryState {
	
	private static final String COMMAND_DICE = "number-of-dice";
	private static final String COMMAND_SIDES = "number-of-faces";
	private static final String COMMAND_COUNT = "count";
	private static final String COMMAND_PREDICATE = "predicate";
	
	private Integer myDice = null;
	private Integer mySides = null;
	private Integer myCount = null;
	private Predicate myPred = null;
	private Double myResult = null;

	public boolean initialize(String querySource,
			Map<Object, List<Object>> queryParameters) {
		boolean returnVal = false;
		
		if (queryParameters.size() == 4) {
			if (queryParameters.containsKey(COMMAND_DICE) && queryParameters.get(COMMAND_DICE).size()==1 &&
				queryParameters.containsKey(COMMAND_SIDES) && queryParameters.get(COMMAND_SIDES).size()==1 &&
				queryParameters.containsKey(COMMAND_COUNT) && queryParameters.get(COMMAND_COUNT).size()==1 &&
				queryParameters.containsKey(COMMAND_PREDICATE) && queryParameters.get(COMMAND_PREDICATE).size()==1) {
				
				if ((queryParameters.get(COMMAND_DICE).iterator().next() instanceof Long) &&
					(queryParameters.get(COMMAND_SIDES).iterator().next() instanceof Long) &&
					(queryParameters.get(COMMAND_COUNT).iterator().next() instanceof Long) &&
					(queryParameters.get(COMMAND_PREDICATE).iterator().next() instanceof String)) {
					
					myDice = ((Long) queryParameters.get(COMMAND_DICE).iterator().next()).intValue();
					mySides = ((Long) queryParameters.get(COMMAND_SIDES).iterator().next()).intValue();
					myCount = ((Long) queryParameters.get(COMMAND_COUNT).iterator().next()).intValue();
					myPred = Predicate.valueOf((String) queryParameters.get(COMMAND_PREDICATE).iterator().next());					
					
					try {
						myResult = myPred.get(myDice, mySides, myCount);
						
						returnVal = true;
						hasComputed = false;
					} catch (Exception e) {
						
					}
				}			
			}
		}
		
		return returnVal;
	}

	public Map<String, List<Object>> next() {
		if (!hasComputed) {
			hasComputed = true;
			HashMap<String, List<Object>> returnVal = new HashMap<String, List<Object>>();
			List<Object> newList = new LinkedList<Object>();
			
			newList.add(myResult);
			returnVal.put("probability", newList);
			
			return returnVal;
		}
		
		return null;
	}

}
