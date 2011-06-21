package edu.umich.soar.qna.db;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import edu.umich.soar.qna.QueryState;

public class DatabaseQueryState implements QueryState {
	
	private enum QueryType {
		qQuery,
		qUpdate
	}
	
	private static final String UPDATES_NAME = "updates";

	Connection conn;
	ResultSet rs;
	PreparedStatement statement;
	boolean dataAvailable;
	QueryType t;
	int numUpdates;
	
	DatabaseQueryState(Connection conn) {
		this.conn = conn;
		this.statement = null;
		this.rs = null;
		this.t = null;
		this.numUpdates = -1;
		
		dataAvailable = false;
	}
	
	public void dispose() {
		dataAvailable = false;
		
		if (rs != null) {
			try {
				rs.close();
				statement.close();
			} catch (SQLException e) {
				e.printStackTrace();
			}
		}
	}

	public boolean hasNext() {
		return dataAvailable;
	}
	
	public boolean initialize(String querySource, Map<Object, List<Object>> queryParameters) {
		
		try {
			statement = conn.prepareStatement(querySource);
		} catch (SQLException e) {
			statement = null;
			e.printStackTrace();
		}
		
		if (statement != null) {
			for ( Entry<Object, List<Object>> p : queryParameters.entrySet() ) {
				if (!(p.getKey() instanceof Long) || (p.getValue().size()!=1)) {
					statement = null;
				}
			}
		}
		
		if (statement != null) {
			for ( Entry<Object, List<Object>> p : queryParameters.entrySet() ) {
				try {
					statement.setObject(((Long) p.getKey()).intValue(), p.getValue().iterator().next());
				} catch (SQLException e) {
					statement = null;
					e.printStackTrace();
				}
			}
		}
		
		if (statement != null) {
			try {
				boolean res = statement.execute();
				
				if (res) {
					rs = statement.getResultSet();
					t = QueryType.qQuery;
					dataAvailable = rs.next();
				} else {
					if (statement.getUpdateCount()!=-1) {
						rs = null;
						numUpdates = statement.getUpdateCount();
						t = QueryType.qUpdate;
						dataAvailable = true;
					} else {
						rs = null;
						t = QueryType.qQuery;
						dataAvailable = false;
					}
				}
			} catch (SQLException e) {
				rs = null;
				statement = null;
				e.printStackTrace();
			}
		}
		
		return dataAvailable;
	}

	public Map<String, List<Object>> next() {
		Map<String, List<Object>> returnVal = null;
		
		if (dataAvailable) {
			returnVal = new HashMap<String, List<Object>>();
			
			if (t == QueryType.qQuery) {
				try {
					ResultSetMetaData md = rs.getMetaData();
					int numColumns = md.getColumnCount();
					
					for (int i=1; i<=numColumns; i++) {
						String columnName = md.getColumnLabel(i);
						
						if (!returnVal.containsKey(columnName)) {
							returnVal.put(columnName, new LinkedList<Object>());
						}
						returnVal.get(columnName).add(rs.getObject(i));
					}
					
					dataAvailable = rs.next();
				} catch (SQLException e) {
					e.printStackTrace();
				}
			} else {
				returnVal.put(UPDATES_NAME, new LinkedList<Object>());
				returnVal.get(UPDATES_NAME).add(new Long(numUpdates));
				dataAvailable = false;
			}
		}
		
		return returnVal;
	}

}
