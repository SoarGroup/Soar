package edu.umich.soar.qna.http.scraper;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.json.JSONObject;

import edu.umich.soar.qna.http.HTTPScraper;

public class JSONScraper implements HTTPScraper {

	public void initialize(InputStream in) {
		this.in = in;
		this.moreData = true;
	}

	public boolean moreResults() {
		return moreData;
	}

	public Map<String, List<Object>> scrape() {
		Map<String, List<Object>> returnVal = new HashMap<String, List<Object>>();
		
		try {
			BufferedReader br = new BufferedReader(new InputStreamReader(in));
			String line;
			String res = "";
			
			while ((line=br.readLine())!=null) {
				res = res + line;
			}
			
			JSONObject json = new JSONObject(res);
			for (String s : JSONObject.getNames(json)) {
				if (!returnVal.containsKey(s)) {
					returnVal.put(s, new LinkedList<Object>());
				}
				
				returnVal.get(s).add(json.get(s));
			}
			
			moreData = false;
			
		} catch (Exception e) {
			e.printStackTrace();
		}
		
		return returnVal;
	}
	
	
	private InputStream in;
	private boolean moreData;

}
