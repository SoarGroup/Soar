package sps.control;

import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;

import orc.util.GamePad;

import org.apache.log4j.Logger;

import sps.config.Config;
import sps.config.ConfigFile;

public class Controller extends TimerTask {
	private static Logger logger = Logger.getLogger(Controller.class);
	
	private Config config;
	private GamepadInterface gp;
	private SoarInterface soar;
	private boolean gamepadOverride = false;
	private Timer timer = new Timer();
	
	Controller(Config config) {
		if (config == null) {
			throw new NullPointerException();
		}
		
		this.config = config;
		gp = new GamepadInterface();
		timer.schedule(this, 0, 1000 / 100); // 10 Hz
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
		System.exit(0);
	}

	@Override
	public void run() {
		//gamepadOverride = gp.getOverride();
		
		//lcmtypes.differential_drive_command_t newdc;
		//if (gamepadOverride) {
		//	newdc = gp.getDC();
		//} else {
		//	newdc = soar.getDC();
		//}
		
		//transmit(newdc);
	}
}
