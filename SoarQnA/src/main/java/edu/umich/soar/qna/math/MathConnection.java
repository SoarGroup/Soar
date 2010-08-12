package edu.umich.soar.qna.math;

import java.util.List;
import java.util.Map;

import edu.umich.soar.qna.DataSourceConnection;
import edu.umich.soar.qna.QueryState;

public class MathConnection implements DataSourceConnection {

	@Override
	public void disconnect() {
	}

	@Override
	public QueryState executeQuery(String querySource,
			Map<Object, List<Object>> queryParameters) {
		QueryState returnVal = null;
		
		if (querySource.equals("+")) {
			returnVal = new AdditionQueryState();
		} else if (querySource.equals("-")) {
			if (queryParameters.size() == 1) {
				returnVal = new NegationQueryState();
			} else {
				returnVal = new SubtractionQueryState();
			}
		} else if (querySource.equals("*")) {
			returnVal = new MultiplicationQueryState();
		} else if (querySource.equals("/")) {
			if (queryParameters.size() == 1) {
				returnVal = new ReciprocationQueryState();
			} else {
				returnVal = new DivisionQueryState();
			}
		} else if (querySource.equals("mod")) {
			returnVal = new ModQueryState();
		} else if (querySource.equals("div")) {
			returnVal = new DivQueryState();
		} else if (querySource.equals("abs")) {
			returnVal = new AbsQueryState();
		} else if (querySource.equals("sqrt")) {
			returnVal = new SqrtQueryState();
		} else if (querySource.equals("int")) {
			returnVal = new IntQueryState();
		} else if (querySource.equals("float")) {
			returnVal = new FloatQueryState();
		}
		
		if ((returnVal!=null) && !returnVal.initialize(querySource, queryParameters)) {
			returnVal = null;
		}
		
		return returnVal;
	}

}
