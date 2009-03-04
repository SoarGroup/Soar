package org.msoar.sps.control.html;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.log4j.Logger;

import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class HttpController {
	private static Logger logger = Logger.getLogger(HttpController.class);
	private static final int HTTP_PORT = 8000;
	
	private final String ACTION = "action";
	private enum Actions {
		postmessage;
	}
	
	private enum Keys {
		message;
	}
	
	private IndexHandler indexHandler = new IndexHandler();
	
	public HttpController() {
		try {
		    HttpServer server = HttpServer.create(new InetSocketAddress(HTTP_PORT), 0);
		    server.createContext("/", new IndexHandler());
		    server.createContext("/debug", new DebugHandler());
		    server.start();
		} catch (IOException e) {
			logger.fatal("Error starting http server: " + e.getMessage());
			e.printStackTrace();
			System.exit(1);
		}
		
		logger.info("http server running on port " + HTTP_PORT);
	}
	
	private class IndexHandler implements HttpHandler {
		private List<String> tokens;
		
		public void handle(HttpExchange xchg) throws IOException {
			if (xchg.getRequestMethod().equals("GET")) {
				sendFile(xchg, "/org/msoar/sps/control/html/index.html");
			} else {
				handlePost(xchg);
			}
		}
		
		private void sendFile(HttpExchange xchg, String path) throws IOException {
			StringBuffer response = new StringBuffer();

			// I'm sure there is a better way to pipe this stuff!
			BufferedReader br = 
				new BufferedReader(
					new InputStreamReader(
							HttpController.class.getResourceAsStream(path)));

			String line;
			while ((line = br.readLine()) != null) {
				response.append(line);
				response.append("\n");
			}

			sendResponse(xchg, response.toString());
		}

		private void handlePost(HttpExchange xchg) throws IOException {
			BufferedReader br = new BufferedReader(new InputStreamReader(xchg.getRequestBody()));
			char[] buf = new char[256];
			int len;
			StringBuffer body = new StringBuffer();
			while ((len = br.read(buf)) != -1) {
				body.append(buf, 0, len);
			}

			Map<String, String> properties = new HashMap<String, String>();
			String[] all = body.toString().split("&");
			for (String each : all) {
				String[] pair = each.split("=");
				if (pair.length > 2) {
					logger.error("Too many tokens: " + each);
					sendFile(xchg, "/org/msoar/sps/control/html/error.html");
					return;
				} else if (pair.length == 2) {
					properties.put(pair[0], URLDecoder.decode(pair[1], "UTF-8"));
				} else if (pair.length == 1) {
					properties.put(pair[0], null);
				} else {
					logger.error("No tokens");
					sendFile(xchg, "/org/msoar/sps/control/html/error.html");
					return;
				}
			}
			
			if (properties.size() == 0 || !properties.containsKey(ACTION) 
					|| properties.get(ACTION) == null) {
				logger.error("No action specified");
				sendFile(xchg, "/org/msoar/sps/control/html/error.html");
				return;
			}
			
			if (properties.get(ACTION).equals(Actions.postmessage.name())) {
				postMessage(xchg, properties);
			} else {
				logger.error("Unknown action: " + properties.get(ACTION));
				sendFile(xchg, "/org/msoar/sps/control/html/error.html");
				return;
			}
		}
		
		private void postMessage(HttpExchange xchg, Map<String, String> properties) throws IOException {
			String message = properties.get(Keys.message.name());
			if (message == null) {
				sendFile(xchg, "/org/msoar/sps/control/html/index.html");
				return;
			}
			List<String> newTokens = Arrays.asList(message.split(" "));
			
			Iterator<String> iter = newTokens.iterator();
			while (iter.hasNext()) {
				String token = iter.next();
				if (token.length() == 0) {
					iter.remove();
				}
			}
			
			this.tokens = newTokens;
			
		    StringBuffer response = new StringBuffer();
		    if (tokens.size() > 0) {
			    response.append("Sent: ");
			    response.append(Arrays.toString(tokens.toArray(new String[tokens.size()])));
			    response.append("\n");
		    } else {
		    	response.append("Cleared all messages.\n");
		    }
		    
		    sendResponse(xchg, response.toString());
		}
	}
	
	private class DebugHandler implements HttpHandler {
		public void handle(HttpExchange xchg) throws IOException {
			Headers headers = xchg.getRequestHeaders();
			Set<Map.Entry<String, List<String>>> entries = headers.entrySet();

			StringBuffer response = new StringBuffer();
			for (Map.Entry<String, List<String>> entry : entries) {
				response.append(entry.toString() + "\n");
			}
			response.append("\n\n");
			
			BufferedReader br = new BufferedReader(new InputStreamReader(xchg.getRequestBody()));
			char[] buf = new char[256];
			int len;
			while ((len = br.read(buf)) != -1) {
				response.append(buf, 0, len);
			}

			sendResponse(xchg, response.toString());
		}
	}
	
	private void sendResponse(HttpExchange xchg, String response) throws IOException {
	    xchg.sendResponseHeaders(200, response.length());
	    OutputStream os = xchg.getResponseBody();
	    os.write(response.toString().getBytes());
	    os.close();
	}
	
	public List<String> getMessageTokens() {
		List<String> temp = indexHandler.tokens;
		indexHandler.tokens = null;
		return temp;
	}
	
}
