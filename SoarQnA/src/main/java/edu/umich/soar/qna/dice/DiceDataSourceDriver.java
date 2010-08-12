package edu.umich.soar.qna.dice;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import edu.umich.soar.qna.DataSourceConnection;
import edu.umich.soar.qna.DataSourceDriver;

public class DiceDataSourceDriver implements DataSourceDriver {

	@Override
	public DataSourceConnection connect(Map<String, String> parameters) {
		return new DiceConnection();
	}

	@Override
	public List<String> getInstanceParameters() {
		return new LinkedList<String>();
	}

}
