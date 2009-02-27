package org.msoar.sps.control;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
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
	private static Logger logger = Logger.getLogger(Controller.class);
	private static int DEFAULT_RANGES_COUNT = 5;
	
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

}
