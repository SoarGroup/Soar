package sps.control;

import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;

import lcm.lcm.LCM;
import lcmtypes.differential_drive_command_t;

import org.apache.log4j.Logger;

import sps.Names;
import sps.config.Config;
import sps.config.ConfigFile;

public class Controller extends TimerTask {
	private static Logger logger = Logger.getLogger(Controller.class);
	
	//private Config config;
	private GamepadInterface gp;
	private SoarInterface soar;
	private Timer timer = new Timer();
	private differential_drive_command_t dc = new differential_drive_command_t();
	private LCM lcm;

	Controller(Config config) {
		if (config == null) {
			throw new NullPointerException();
		}
		
		//this.config = config;

		gp = new GamepadInterface();
		soar = new SoarInterface();
		lcm = LCM.getSingleton();
		
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
		if (gp != null) {
			gp.update();
		
			if (gp.getSoarControlButton()) {
				soar.changeRunningState();
			}
			
			if (gp.getShutdownButton()) {
				this.cancel();
				System.exit(0);
				return;
			}
		}	
		
		if (gp != null && gp.getOverrideMode()) {
			gp.getDC(dc);
		} else {
			soar.getDC(dc);
		}
		
		transmit(dc);
	}
	
	private void transmit(differential_drive_command_t dc) {
		dc.utime = System.nanoTime() / 1000;
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
