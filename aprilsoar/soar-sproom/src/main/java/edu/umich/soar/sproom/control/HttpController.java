package edu.umich.soar.sproom.control;

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

import jmat.MathUtil;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import edu.umich.soar.sproom.control.PIDController.Gains;

final class HttpController {
	private static final Log logger = LogFactory.getLog(HttpController.class);
	private static final int HTTP_PORT = 8000;
	private static final String INDEX_HTML = "/edu/umich/soar/sproom/control/index.html";
	private static final String ERROR_HTML = "/edu/umich/soar/sproom/control/error.html";
	
	static HttpController newInstance() {
		return new HttpController();
	}
	
	private final String ACTION = "action";
	private enum Actions {
		postmessage, heading, angvel, linvel, estop, stop, agains, lgains, hgains
	}
	
	private enum Keys {
		message, heading, angvel, linvel, pgain, igain, dgain
	}
	
	private final IndexHandler indexHandler = new IndexHandler();
	private DifferentialDriveCommand ddc;
	private SplinterModel splinter;
	
	private HttpController() {
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
	
	void setSplinter(SplinterModel splinter) {
		this.splinter = splinter;
	}
	
	private class IndexHandler implements HttpHandler {
		private List<String> tokens;
		
		public void handle(HttpExchange xchg) throws IOException {
			if (xchg.getRequestMethod().equals("GET")) {
				sendFile(xchg, INDEX_HTML);
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
				line = performSubstitutions(line);
				response.append(line);
				response.append("\n");
			}

			sendResponse(xchg, response.toString());
		}
		
		private String performSubstitutions(String line) {
			Gains hgains = splinter.getHGains();
			line = line.replaceAll("%hpgain%", Double.toString(hgains.p));
			line = line.replaceAll("%higain%", Double.toString(hgains.i));
			line = line.replaceAll("%hdgain%", Double.toString(hgains.d));

			Gains agains = splinter.getAGains();
			line = line.replaceAll("%apgain%", Double.toString(agains.p));
			line = line.replaceAll("%aigain%", Double.toString(agains.i));
			line = line.replaceAll("%adgain%", Double.toString(agains.d));

			Gains lgains = splinter.getLGains();
			line = line.replaceAll("%lpgain%", Double.toString(lgains.p));
			line = line.replaceAll("%ligain%", Double.toString(lgains.i));
			line = line.replaceAll("%ldgain%", Double.toString(lgains.d));
			return line;
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
					sendFile(xchg, ERROR_HTML);
					return;
				} else if (pair.length == 2) {
					properties.put(pair[0], URLDecoder.decode(pair[1], "UTF-8"));
				} else if (pair.length == 1) {
					properties.put(pair[0], null);
				} else {
					logger.error("No tokens");
					sendFile(xchg, ERROR_HTML);
					return;
				}
			}
			
			if (properties.size() == 0 || !properties.containsKey(ACTION) 
					|| properties.get(ACTION) == null) {
				logger.error("No action specified");
				sendFile(xchg, ERROR_HTML);
				return;
			}
			
			if (properties.get(ACTION).equals(Actions.postmessage.name())) {
				postMessage(xchg, properties);
			} else if (properties.get(ACTION).equals(Actions.heading.name())) {
				heading(xchg, properties);
			} else if (properties.get(ACTION).equals(Actions.angvel.name())) {
				angvel(xchg, properties);
			} else if (properties.get(ACTION).equals(Actions.linvel.name())) {
				linvel(xchg, properties);
			} else if (properties.get(ACTION).equals(Actions.estop.name())) {
				estop(xchg);
			} else if (properties.get(ACTION).equals(Actions.stop.name())) {
				stop(xchg);
			} else if (properties.get(ACTION).equals(Actions.agains.name())) {
				agains(xchg, properties);
			} else if (properties.get(ACTION).equals(Actions.lgains.name())) {
				lgains(xchg, properties);
			} else if (properties.get(ACTION).equals(Actions.hgains.name())) {
				hgains(xchg, properties);
			} else {
				logger.error("Unknown action: " + properties.get(ACTION));
				sendFile(xchg, ERROR_HTML);
				return;
			}
		}
		
		private void postMessage(HttpExchange xchg, Map<String, String> properties) throws IOException {
			logger.trace("postMessage");
			String message = properties.get(Keys.message.name());
			if (message == null) {
				sendFile(xchg, ERROR_HTML);
				return;
			}
			
			// test
			//Say.newMessage(message);
			
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
		    
			logger.debug(response);
		    sendResponse(xchg, response.toString());
		}
		
		private void heading(HttpExchange xchg, Map<String, String> properties) throws IOException {
			logger.trace("heading");
			String headingString = properties.get(Keys.heading.name());
			if (headingString == null) {
				sendFile(xchg, INDEX_HTML);
				return;
			}
			
			try {
				double yaw = Math.toRadians(Double.parseDouble(headingString));
				yaw = MathUtil.mod2pi(yaw);
				ddc = DifferentialDriveCommand.newHeadingCommand(yaw);
				logger.debug(ddc);
				sendFile(xchg, INDEX_HTML);
			} catch (NumberFormatException e) {
				sendResponse(xchg, "Invalid number");
				return;
			}
		}
		
		private void angvel(HttpExchange xchg, Map<String, String> properties) throws IOException {
			logger.trace("angvel");
			String angvelString = properties.get(Keys.angvel.name());
			if (angvelString == null) {
				sendFile(xchg, INDEX_HTML);
				return;
			}
			
			try {
				double angvel = Math.toRadians(Double.parseDouble(angvelString));
				ddc = DifferentialDriveCommand.newAngularVelocityCommand(angvel);
				logger.debug(ddc);
				sendFile(xchg, INDEX_HTML);
			} catch (NumberFormatException e) {
				sendResponse(xchg, "Invalid number");
				return;
			}
		}
		
		private void linvel(HttpExchange xchg, Map<String, String> properties) throws IOException {
			logger.trace("linvel");
			String linvelString = properties.get(Keys.linvel.name());
			if (linvelString == null) {
				sendFile(xchg, INDEX_HTML);
				return;
			}
			
			try {
				double linvel = Double.parseDouble(linvelString);
				ddc = DifferentialDriveCommand.newLinearVelocityCommand(linvel);
				logger.debug(ddc);
				sendFile(xchg, INDEX_HTML);
			} catch (NumberFormatException e) {
				sendResponse(xchg, "Invalid number");
				return;
			}
		}
		
		private void estop(HttpExchange xchg) throws IOException {
			logger.trace("estop");
			ddc = DifferentialDriveCommand.newEStopCommand();
			sendFile(xchg, INDEX_HTML);
		}

		private void stop(HttpExchange xchg) throws IOException {
			logger.trace("stop");
			ddc = DifferentialDriveCommand.newVelocityCommand(0, 0);
			sendFile(xchg, INDEX_HTML);
		}

		private double parseDefault(String value) {
			double out = 0;
			try {
				out = Double.parseDouble(value);
			} catch (NullPointerException ignored) {
				// ignored, use 0
			}
			return out;
		}
		
		private void agains(HttpExchange xchg, Map<String, String> properties) throws IOException {
			logger.trace("agains");
			double p = 0;
			double i = 0;
			double d = 0;

			try {
				p = parseDefault(properties.get(Keys.pgain.name()));
				i = parseDefault(properties.get(Keys.igain.name()));
				d = parseDefault(properties.get(Keys.dgain.name()));
			} catch (NumberFormatException e) {
				sendResponse(xchg, "Invalid number");
				return;
			}
			
			splinter.setAGains(new Gains(p, i, d));
			sendFile(xchg, INDEX_HTML);
		}
		
		private void lgains(HttpExchange xchg, Map<String, String> properties) throws IOException {
			logger.trace("lgains");
			double p = 0;
			double i = 0;
			double d = 0;

			try {
				p = parseDefault(properties.get(Keys.pgain.name()));
				i = parseDefault(properties.get(Keys.igain.name()));
				d = parseDefault(properties.get(Keys.dgain.name()));
			} catch (NumberFormatException e) {
				sendResponse(xchg, "Invalid number");
				return;
			}
			
			splinter.setLGains(new Gains(p, i, d));
			sendFile(xchg, INDEX_HTML);
		}
		
		private void hgains(HttpExchange xchg, Map<String, String> properties) throws IOException {
			logger.trace("hgains");
			double p = 0;
			double i = 0;
			double d = 0;

			try {
				p = parseDefault(properties.get(Keys.pgain.name()));
				i = parseDefault(properties.get(Keys.igain.name()));
				d = parseDefault(properties.get(Keys.dgain.name()));
			} catch (NumberFormatException e) {
				sendResponse(xchg, "Invalid number");
				return;
			}
			
			splinter.setHGains(new Gains(p, i, d));
			sendFile(xchg, INDEX_HTML);
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
	
	List<String> getMessageTokens() {
		List<String> temp = indexHandler.tokens;
		indexHandler.tokens = null;
		return temp;
	}

	boolean hasDDCommand() {
		return ddc != null;
	}

	DifferentialDriveCommand getDDCommand() {
		DifferentialDriveCommand temp = ddc;
		ddc = null;
		return temp;
	}
	
}
