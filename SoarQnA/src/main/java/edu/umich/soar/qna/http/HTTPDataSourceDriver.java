package edu.umich.soar.qna.http;

import java.util.Arrays;
import java.util.List;
import java.util.Map;

import edu.umich.soar.qna.DataSourceConnection;
import edu.umich.soar.qna.DataSourceDriver;

public class HTTPDataSourceDriver implements DataSourceDriver {

	private static final List<String> instanceParameters = Arrays.asList( "method", "scraper" );
	
	public List<String> getInstanceParameters() {
		return HTTPDataSourceDriver.instanceParameters;
	}

	public DataSourceConnection connect(Map<String, String> parameters) {
		DataSourceConnection returnVal = null;
		
		boolean goodParameters = true;
		if (parameters.size() == HTTPDataSourceDriver.instanceParameters.size()) {
			for (Object o : HTTPDataSourceDriver.instanceParameters) {
				if (!parameters.containsKey(o) || !(parameters.get(o) instanceof String)) {
					goodParameters = false;
				}
			}
		}
		
		String method = parameters.get("method");
		if (goodParameters && ((method.compareTo("GET")!=0) && (method.compareTo("POST")!=0))) {
			goodParameters = false;
		}
		
		if (goodParameters) {
			try {
				HTTPScraper scraper = (HTTPScraper) Class.forName((String) parameters.get("scraper")).newInstance();
				returnVal = new HTTPConnection(scraper, method);
				
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		
		return returnVal;
	}

}
