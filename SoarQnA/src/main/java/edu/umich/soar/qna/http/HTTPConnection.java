package edu.umich.soar.qna.http;

import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URI;
import java.net.URL;
import java.util.List;
import java.util.Map;

import edu.umich.soar.qna.DataSourceConnection;
import edu.umich.soar.qna.QueryState;

public class HTTPConnection implements DataSourceConnection {

	private final HTTPScraper scraper;
	private final String method;
	
	public HTTPConnection(HTTPScraper scraper, String method) {
		this.scraper = scraper;
		this.method = method;
	}
	
	public QueryState executeQuery(String querySource, Map<Object, List<Object>> queryParameters) {
		QueryState httpQueryState = null;
		
		try {
			String paramsQuery = "";
			for (Map.Entry<Object, List<Object>> k : queryParameters.entrySet()) {
				for (Object v : k.getValue()) {
					paramsQuery = paramsQuery + ( k.getKey().toString() + "=" + v.toString() + "&" );
				}
			}
			
			URI parsed = new URI(querySource);
			String scheme = parsed.getScheme();
			String host = parsed.getHost();
			String path = ((parsed.getPath()==null) || (parsed.getPath().length()==0))?("/"):(parsed.getPath());
			int port = (parsed.getPort()>0)?(parsed.getPort()):(80);
			String query = parsed.getQuery();
			
			String uriQuery = ((query==null)?(""):(query));
			if (method.compareTo("GET")==0) {
				if (query!=null) {
					uriQuery = uriQuery + "&";
				}
				
				uriQuery = uriQuery + paramsQuery;
			}
			
			URI uri = new URI(scheme, null, host, port, path, uriQuery, null);
			HttpURLConnection conn = (HttpURLConnection) new URL(uri.toString()).openConnection();
			conn.setRequestMethod(method);
			if (method.compareTo("POST")==0) {
				conn.setDoOutput(true);
				OutputStreamWriter out = new OutputStreamWriter(conn.getOutputStream());
				out.write(paramsQuery);
				out.close();
			}
			conn.connect();
			
			httpQueryState = new HTTPQueryState(conn, scraper);
			if (!httpQueryState.initialize(querySource, queryParameters)) {
				httpQueryState = null;
			}
			
		} catch (Exception e) {
			e.printStackTrace();
		}
		
		return httpQueryState;
	}

	public void disconnect() {
	}

}
