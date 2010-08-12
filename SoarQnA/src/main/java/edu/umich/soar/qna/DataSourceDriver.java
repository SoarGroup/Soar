package edu.umich.soar.qna;

import java.util.List;
import java.util.Map;

public interface DataSourceDriver {
	List<String> getInstanceParameters();
	DataSourceConnection connect(Map<String, String> parameters);
}
