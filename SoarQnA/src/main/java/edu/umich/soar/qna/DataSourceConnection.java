package edu.umich.soar.qna;

import java.util.List;
import java.util.Map;

public interface DataSourceConnection {
	QueryState executeQuery(String querySource, Map<Object, List<Object>> queryParameters);
	void disconnect();
}
