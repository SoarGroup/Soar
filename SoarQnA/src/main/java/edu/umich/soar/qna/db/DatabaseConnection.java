package edu.umich.soar.qna.db;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.List;
import java.util.Map;

import edu.umich.soar.qna.DataSourceConnection;
import edu.umich.soar.qna.QueryState;

public class DatabaseConnection implements DataSourceConnection {

	final Connection conn;
	
	DatabaseConnection(Connection conn) {
		this.conn = conn;
	}
	
	public void disconnect() {
		if (conn != null) {
			try {
				conn.close();
			} catch (SQLException e) {
				e.printStackTrace();
			}
		}
	}

	public QueryState executeQuery(String querySource,
			Map<Object, List<Object>> queryParameters) {
		
		QueryState dbQueryState = null;
		
		if (conn != null) {
			dbQueryState = new DatabaseQueryState(conn);
			
			if (!dbQueryState.initialize(querySource, queryParameters)) {
				dbQueryState = null;
			}
		}
		
		return dbQueryState;
	}

}
