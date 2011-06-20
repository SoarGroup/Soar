package edu.umich.soar.qna.db;

import java.sql.Connection;
import java.sql.DriverManager;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

import edu.umich.soar.qna.DataSourceConnection;
import edu.umich.soar.qna.DataSourceDriver;

public class DatabaseDataSourceDriver implements DataSourceDriver {
	
	private static final List<String> instanceParameters = Arrays.asList( "driver", "url", "username", "password" );

	public DataSourceConnection connect(Map<String, String> parameters) {
		DataSourceConnection returnVal = null;
		
		boolean goodParameters = true;
		if (parameters.size() == DatabaseDataSourceDriver.instanceParameters.size()) {
			for (Object o : DatabaseDataSourceDriver.instanceParameters) {
				if (!parameters.containsKey(o) || !(parameters.get(o) instanceof String)) {
					goodParameters = false;
				}
			}
		}
		
		if (goodParameters) {
			try {
				Class.forName((String) parameters.get("driver")).newInstance();
				Connection conn = DriverManager.getConnection((String) parameters.get("url"), (String) parameters.get("username"), (String) parameters.get("password"));
				returnVal = new DatabaseConnection(conn);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		
		return returnVal;
	}

	public List<String> getInstanceParameters() {
		return DatabaseDataSourceDriver.instanceParameters;
	}

}
