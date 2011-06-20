package edu.umich.soar.qna.dice;

import java.util.List;
import java.util.Map;

import edu.umich.soar.qna.DataSourceConnection;
import edu.umich.soar.qna.QueryState;

public class DiceConnection implements DataSourceConnection {

	public void disconnect() {
	}

	public QueryState executeQuery(String querySource,
			Map<Object, List<Object>> queryParameters) {
		QueryState returnVal = null;
		
		if (querySource.equals("compute-probability")) {
			returnVal = new ComputeProbabilityQueryState();
		}
		
		if ((returnVal!=null) && !returnVal.initialize(querySource, queryParameters)) {
			returnVal = null;
		}
		
		return returnVal;
	}

}
