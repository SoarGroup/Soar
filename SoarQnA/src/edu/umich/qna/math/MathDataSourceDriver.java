package edu.umich.qna.math;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import edu.umich.qna.DataSourceConnection;
import edu.umich.qna.DataSourceDriver;

public class MathDataSourceDriver implements DataSourceDriver {

	@Override
	public List<String> getInstanceParameters() {
		return new LinkedList<String>();
	}	

	@Override
	public DataSourceConnection connect(Map<String, String> parameters) {
		return new MathConnection();
	}

}
