package edu.umich.soar.qna.http;

import java.io.IOException;
import java.net.HttpURLConnection;
import java.util.List;
import java.util.Map;

import edu.umich.soar.qna.QueryState;

public class HTTPQueryState implements QueryState {

	public HTTPQueryState(HttpURLConnection conn, HTTPScraper scraper) {
		this.conn = conn;
		this.scraper = scraper;
	}
	
	public boolean initialize(String querySource, Map<Object, List<Object>> queryParameters) {
		boolean returnVal = false;
		
		try {
			scraper.initialize(conn.getInputStream());
			returnVal = scraper.moreResults();
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		return returnVal;
	}

	public boolean hasNext() {
		return scraper.moreResults();
	}

	public Map<String, List<Object>> next() {
		return scraper.scrape();
	}

	public void dispose() {
		if (conn!=null) {
			conn.disconnect();
		}
	}
	
	private final HttpURLConnection conn;
	private final HTTPScraper scraper;

}
