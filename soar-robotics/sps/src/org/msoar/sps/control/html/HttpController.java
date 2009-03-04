package org.msoar.sps.control.html;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.util.Arrays;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import org.apache.log4j.Logger;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class HttpController {
	private static Logger logger = Logger.getLogger(HttpController.class);
	private static final int HTTP_PORT = 8000;
	private static final String MESSAGE = "postmessage";
	
	private PostMessageHandler postMessageHandler = new PostMessageHandler();
	
	public HttpController() {
		try {
		    HttpServer server = HttpServer.create(new InetSocketAddress(HTTP_PORT), 0);
		    server.createContext("/", new IndexHandler());
		    server.createContext("/" + MESSAGE, postMessageHandler);
		    server.start();
		} catch (IOException e) {
			logger.fatal("Error starting http server: " + e.getMessage());
			e.printStackTrace();
			System.exit(1);
		}
		
		logger.info("http server running on port " + HTTP_PORT);

	}
	
	private class IndexHandler implements HttpHandler {
		public void handle(HttpExchange xchg) throws IOException {
			StringBuffer response = new StringBuffer();

			// I'm sure there is a better way to pipe this stuff!
			BufferedReader br = 
				new BufferedReader(
					new InputStreamReader(
							HttpController.class.getResourceAsStream(
									"/org/msoar/sps/control/html/index.html")));

			String line;
			while ((line = br.readLine()) != null) {
				response.append(line);
				response.append("\n");
			}

			sendResponse(xchg, response.toString());
		}
	}

	private class PostMessageHandler implements HttpHandler {
		private List<String> tokens;
		
		public void handle(HttpExchange xchg) throws IOException {
			URI uri = xchg.getRequestURI();
			logger.info("http request: " + uri.getPath());
			
			List<String> tokens = new LinkedList<String>(Arrays.asList(uri.getPath().split("/")));
			Iterator<String> iter = tokens.iterator();
			while (iter.hasNext()) {
				String token = iter.next();
				if (token.length() == 0 || token.equals(MESSAGE)) {
					iter.remove();
				}
			}
			// TODO sidestepping synchronization
			this.tokens = tokens;
			
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
	
	private void sendResponse(HttpExchange xchg, String response) throws IOException {
	    xchg.sendResponseHeaders(200, response.length());
	    OutputStream os = xchg.getResponseBody();
	    os.write(response.toString().getBytes());
	    os.close();
	}
	
	public List<String> getMessageTokens() {
		List<String> temp = postMessageHandler.tokens;
		postMessageHandler.tokens = null;
		return temp;
	}
	
}
