package edu.umich.qna.db;

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

import edu.umich.qna.QueryState;

public class DatabaseQueryState implements QueryState {

	Connection conn;
	ResultSet rs;
	PreparedStatement statement;
	boolean dataAvailable;
	
	DatabaseQueryState(Connection conn) {
		this.conn = conn;
		this.statement = null;
		this.rs = null;
		
		dataAvailable = false;
	}
	
	@Override
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

	@Override
	public boolean hasNext() {
		return dataAvailable;
	}

	@Override
	public boolean initialize(String querySource, Map<Object, List<Object>> queryParameters) {
		
		try {
			statement = conn.prepareStatement(querySource);
		} catch (SQLException e) {
			statement = null;
			e.printStackTrace();
		}
		
		if (statement != null) {
			for ( Entry<Object, List<Object>> p : queryParameters.entrySet() ) {
				if (!(p.getKey() instanceof Integer) || (p.getValue().size()!=1)) {
					statement = null;
				}
			}
		}
		
		if (statement != null) {
			for ( Entry<Object, List<Object>> p : queryParameters.entrySet() ) {
				try {
					statement.setObject(((Integer) p.getKey()).intValue(), p.getValue().iterator().next());
				} catch (SQLException e) {
					statement = null;
					e.printStackTrace();
				}
			}
		}
		
		if (statement != null) {
			try {
				rs = statement.executeQuery();
				dataAvailable = rs.next();
			} catch (SQLException e) {
				rs = null;
				statement = null;
				e.printStackTrace();
			}
		}
		
		return dataAvailable;
	}

	@Override
	public Map<String, List<Object>> next() {
		Map<String, List<Object>> returnVal = null;
		
		if (dataAvailable) {
			returnVal = new HashMap<String, List<Object>>();
			
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
		}
		
		return returnVal;
	}

}
