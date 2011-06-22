package edu.umich.soar.qna.http;

import java.io.InputStream;
import java.util.List;
import java.util.Map;

public interface HTTPScraper {
	void initialize(InputStream in);
	boolean moreResults();
	Map<String, List<Object>> scrape();
}
