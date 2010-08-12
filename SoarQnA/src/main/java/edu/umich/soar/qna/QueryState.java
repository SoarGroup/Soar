package edu.umich.soar.qna;

import java.util.List;
import java.util.Map;

public interface QueryState {
	boolean initialize(String querySource, Map<Object, List<Object>> queryParameters);
	boolean hasNext();
	Map<String, List<Object>> next();
	void dispose();
}
