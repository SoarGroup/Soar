package org.msoar.sps.control;

import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;

import lcm.lcm.LCM;
import lcmtypes.differential_drive_command_t;

import orc.util.GamePad;

import org.apache.log4j.Logger;
import org.msoar.sps.Names;
import org.msoar.sps.config.Config;
import org.msoar.sps.config.ConfigFile;


public class Controller extends TimerTask {
	private static Logger logger = Logger.getLogger(Controller.class);
	private static int DEFAULT_RANGES_COUNT = 5;
	
	private Config config;
	private GamePad gp = new GamePad();
	private SoarInterface soar;
	private Timer timer = new Timer();
	private differential_drive_command_t dc = new differential_drive_command_t();
	private LCM lcm;
	private OdometryLogger odom;
	
	private enum Buttons {
		OVERRIDE, SOAR, TANK, SLOW, CAPTURE, TAG;
		private ModeButton b;
		
		void setButton(ModeButton b) {
			this.b = b;
		}
		
		ModeButton getButton() {
			return b;
		}
	}

	Controller(Config config) {
		if (config == null) {
			throw new NullPointerException();
		}
		
		this.config = config;

		String productions = this.config.getString("productions");
		int rangesCount = this.config.getInt("ranges_count", DEFAULT_RANGES_COUNT);
		soar = new SoarInterface(productions, rangesCount);
		lcm = LCM.getSingleton();

		Buttons.OVERRIDE.setButton(new ModeButton("Override", gp, 0));
		Buttons.SOAR.setButton(new ModeButton("Soar control", gp, 1));
		Buttons.TANK.setButton(new ModeButton("Tank mode", gp, 2));
		Buttons.SLOW.setButton(new ModeButton("Slow mode", gp, 3));
		Buttons.CAPTURE.setButton(new ModeButton("Capture mode", gp, 4));
		Buttons.TAG.setButton(new ModeButton("Tag capture", gp, 5));
		
		Runtime.getRuntime().addShutdownHook(new ShutdownHook());
		
		// if gp == null, start soar
		timer.schedule(this, 0, 1000 / 10); // 10 Hz
	}
	
	public class ShutdownHook extends Thread {
		@Override
		public void run() {
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
			button.getButton().update();
		}
		if (gp != null) {
			if (Buttons.OVERRIDE.getButton().isEnabled()) {
				getDC(dc);
			} else {
				soar.getDC(dc);
			}
			
			if (Buttons.SOAR.getButton().checkAndDisable()) {
				soar.changeRunningState();
			}
			
			if (Buttons.CAPTURE.getButton().isEnabled()) {
				try {
					if (odom == null) {
						odom = new OdometryLogger();
					}
					odom.update(Buttons.TAG.getButton().checkAndDisable());
					
				} catch (IOException e) {
					logger.error("IOException while updating odometry: " + e.getMessage());
					if (odom != null) {
						odom.close();
						odom = null;
					}
				}
			} else {
				if (odom != null) {
					odom.close();
					odom = null;
				}
			}
		} else {
			soar.getDC(dc);
		}	
		
		transmit(dc);
	}
	
	private void getDC(differential_drive_command_t dc) {
		dc.left_enabled = true;
		dc.right_enabled = true;
		
		if (Buttons.TANK.getButton().isEnabled()) {
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
		dc.utime = System.nanoTime() / 1000;
		if (Buttons.SLOW.getButton().isEnabled()) {
			logger.debug("slow mode halving throttle");
			dc.left /= 2;
			dc.right /= 2;
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
