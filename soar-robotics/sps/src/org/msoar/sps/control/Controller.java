package org.msoar.sps.control;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.util.Arrays;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.differential_drive_command_t;
import lcmtypes.pose_t;

import org.apache.log4j.Logger;
import org.msoar.sps.Names;
import org.msoar.sps.config.Config;
import org.msoar.sps.config.ConfigFile;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;


public class Controller extends TimerTask implements LCMSubscriber, HttpHandler {
	private static Logger logger = Logger.getLogger(Controller.class);
	private static int DEFAULT_RANGES_COUNT = 5;
	private static int HTTP_PORT = 8000;
	
	private Config config;
	private Gamepad gp;
	private SoarInterface soar;
	private Timer timer = new Timer();
	private differential_drive_command_t dc = new differential_drive_command_t();
	private LCM lcm;
	private FileWriter tagWriter;
	private long poseUtime;
	
	private enum Buttons {
		OVERRIDE, SOAR, TANK, SLOW, TAG;
		private ModeButton b;
		
		void setButton(ModeButton b) {
			this.b = b;
		}
		
		boolean isEnabled() {
			if (b == null) {
				return false;
			}
			return b.isEnabled();
		}
		
		boolean checkAndDisable() {
			if (b == null) {
				return false;
			}
			return b.checkAndDisable();
		}
		
		void update() {
			if (b != null) {
				b.update();
			}
		}
	}

	Controller(Config config) {
		if (config == null) {
			throw new NullPointerException();
		}
		this.config = config;
		
		try {
			gp = new Gamepad();
			
			Buttons.OVERRIDE.setButton(new ModeButton("Override", gp, 0));
			Buttons.SOAR.setButton(new ModeButton("Soar control", gp, 1));
			Buttons.TANK.setButton(new ModeButton("Tank mode", gp, 2));
			Buttons.SLOW.setButton(new ModeButton("Slow mode", gp, 3));
			Buttons.TAG.setButton(new ModeButton("Tag", gp, 4));
		} catch (IllegalStateException e) {
			logger.warn("Disabling gamepad: " + e.getMessage());
		}

		String productions = this.config.getString("productions");
		int rangesCount = this.config.getInt("ranges_count", DEFAULT_RANGES_COUNT);
		soar = new SoarInterface(productions, rangesCount);
		lcm = LCM.getSingleton();
		lcm.subscribe(Names.POSE_CHANNEL, this);

		Runtime.getRuntime().addShutdownHook(new ShutdownHook());
		
		try {
		    HttpServer server = HttpServer.create(new InetSocketAddress(HTTP_PORT), 0);
		    server.createContext("/", this);
		    server.start();
		} catch (IOException e) {
			logger.fatal("Error starting http server: " + e.getMessage());
			e.printStackTrace();
			System.exit(1);
		}
		
		logger.info("http server running on port " + HTTP_PORT);

	    timer.schedule(this, 0, 1000 / 10); // 10 Hz
		
	}
	
	public class ShutdownHook extends Thread {
		@Override
		public void run() {
			if (tagWriter != null) {
				try {
					tagWriter.close();
				} catch (IOException ignored) {
				}
			}
			
			if (soar != null)
				soar.shutdown();

			System.out.flush();
			System.err.println("Terminated");
			System.err.flush();
		}
	}

	@Override
	public void run() {
		logger.trace("Controller update");
		for (Buttons button : Buttons.values()) {
			button.update();
		}
		if (gp != null) {
			if (Buttons.OVERRIDE.isEnabled()) {
				getDC(dc);
			} else {
				soar.getDC(dc);
			}
			
			if (Buttons.SOAR.checkAndDisable()) {
				soar.changeRunningState();
			}
			
			if (Buttons.TAG.checkAndDisable()) {
				try {
					if (tagWriter == null) {
						// TODO: use date/time
						File datafile = File.createTempFile("tags-", ".txt", new File(System.getProperty("user.dir")));
						tagWriter = new FileWriter(datafile);
						logger.info("Opened " + datafile.getAbsolutePath());
					}
					logger.info("mark " + poseUtime);
					tagWriter.append(poseUtime + "\n");
					tagWriter.flush();
					
				} catch (IOException e) {
					logger.error("IOException while recording mark: " + e.getMessage());
				}
			}
			
		} else {
			soar.getDC(dc);
		}	
		
		transmit(dc);
	}
	
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(Names.POSE_CHANNEL)) {
			try {
				pose_t pose = new pose_t(ins);
				poseUtime = pose.utime;
			} catch (IOException e) {
				logger.error("Error decoding pose_t message: " + e.getMessage());
			}
		}
	}
	
	private void getDC(differential_drive_command_t dc) {
		dc.utime = soar.getCurrentUtime();
		dc.left_enabled = true;
		dc.right_enabled = true;
		
		if (Buttons.TANK.isEnabled()) {
			dc.left = gp.getAxis(1) * -1;
			dc.right = gp.getAxis(3) * -1;
		} else {
			// this should not be linear, it is difficult to precicely control
			double fwd = -1 * gp.getAxis(3); // +1 = forward, -1 = back
			double lr = -1 * gp.getAxis(2); // +1 = left, -1 = right

			dc.left = fwd - lr;
			dc.right = fwd + lr;

			double max = Math.max(Math.abs(dc.left), Math.abs(dc.right));
			if (max > 1) {
				dc.left /= max;
				dc.right /= max;
			}
		}
	}

	private void transmit(differential_drive_command_t dc) {
		if (Buttons.SLOW.isEnabled()) {
			logger.debug("slow mode halving throttle");
			dc.left /= 2;
			dc.right /= 2;
		}
		if (logger.isTraceEnabled()) {
			logger.trace("transmit: " + dc.left + "," + dc.right);
		}
		lcm.publish(Names.DRIVE_CHANNEL, dc);
	}
	
	public static void main(String[] args) {
		Config config = null;
		if (args.length > 0) {
			try {
				config = new Config(new ConfigFile(args[0]));
			} catch (IOException e) {
				logger.error(e.getMessage());
				System.exit(1);
			}
		} else {
			config = new Config(new ConfigFile());
		}
		new Controller(config);
	}

//	private void sendIndex(HttpExchange xchg) throws IOException {
//	    StringBuffer response = new StringBuffer();
//
//	    // I'm sure there is a better way to pipe this stuff!
//	    InputStreamReader ir = new InputStreamReader(Controller.class.getResourceAsStream("/org/msoar/sps/control/html/index.html"));
//	    BufferedReader br = new BufferedReader(ir);
//	    
//	    String line;
//	    while ((line = br.readLine()) != null) {
//	    	response.append(line);
//	    	response.append("\n");
//	    }
//	    
//		sendResponse(xchg, response.toString());
//	}

	public void handle(HttpExchange xchg) throws IOException {
		URI uri = xchg.getRequestURI();
		logger.info("http request: " + uri.getPath());

		//String[] tokens = uri.getPath().split("/");
		List<String> tokens = new LinkedList<String>(Arrays.asList(uri.getPath().split("/")));
		Iterator<String> iter = tokens.iterator();
		while (iter.hasNext()) {
			if (iter.next().length() == 0) {
				iter.remove();
			}
		}
		soar.setStringInput(tokens);
		
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
	
	private void sendResponse(HttpExchange xchg, String response) throws IOException {
	    xchg.sendResponseHeaders(200, response.length());
	    OutputStream os = xchg.getResponseBody();
	    os.write(response.toString().getBytes());
	    os.close();
	}

}
