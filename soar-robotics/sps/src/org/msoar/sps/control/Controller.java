package org.msoar.sps.control;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
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

public class Controller extends TimerTask implements LCMSubscriber {
	private static final Logger logger = Logger.getLogger(Controller.class);
	private static final int DEFAULT_RANGES_COUNT = 5;
	
	private final Config config;
	private final SoarInterface soar;
	private final Timer timer = new Timer();
	private final differential_drive_command_t dc = new differential_drive_command_t();
	private final LCM lcm;
	private final Gamepad gp;
	private FileWriter tagWriter;
	private long poseUtime;
	private final HttpController httpController = new HttpController();
	
	private Controller(Config config) {
		if (config == null) {
			throw new NullPointerException();
		}
		this.config = config;

		Gamepad gamepad = null;
		try {
			gamepad = new Gamepad();
			Buttons.setGamepad(gamepad);
		} catch (IllegalStateException e) {
			logger.warn("Disabling gamepad: " + e.getMessage());
		}
		gp = gamepad;

		String productions = this.config.getString("productions");
		int rangesCount = this.config.getInt("ranges_count", DEFAULT_RANGES_COUNT);
		soar = new SoarInterface(productions, rangesCount);
		lcm = LCM.getSingleton();
		lcm.subscribe(Names.POSE_CHANNEL, this);

		Runtime.getRuntime().addShutdownHook(new ShutdownHook());
		
	    timer.schedule(this, 0, 1000 / 10); // 10 Hz
	}
	
	private class ShutdownHook extends Thread {
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
		
		List<String> messageTokens = httpController.getMessageTokens();
		if (messageTokens != null) {
			soar.setStringInput(messageTokens);
		}
		
		if (Buttons.haveGamepad()) {
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

}
