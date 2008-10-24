package splintersoar;

import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;

import lcm.lcm.LCM;

import erp.config.Config;
import erp.config.ConfigFile;
import erp.lcmtypes.differential_drive_command_t;

import orc.util.GamePad;

import splintersoar.laserloc.LaserLoc;
import splintersoar.orc.OrcInterface;
import splintersoar.ranger.RangerManager;
import splintersoar.soar.SoarInterface;

/**
 * @author voigtjr
 * Top-level management of the Soar/uOrc interface. Also responsible for injecting override splinter input using the game pad.
 */
public class SplinterSoar {
	SoarInterface soar;
	OrcInterface orc;
	RangerManager ranger;
	LaserLoc laserloc;
	LCM lcm;
	GamePad gamePad;

	boolean running = true;

	private Logger logger;

	public SplinterSoar(String args[]) {
		Config config = null;

		if (args.length == 1) {
			try {
				config = (new ConfigFile(args[0])).getConfig();
			} catch (IOException ex) {
				logger.severe("Couldn't open config file: " + args[0]);
				return;
			}
		}

		logger = LogFactory.createSimpleLogger("SplinterSoar", Level.INFO);

		lcm = LCM.getSingleton();

		logger.info("Starting laserloc");
		laserloc = new LaserLoc(config);
		laserloc.setDaemon(true);
		laserloc.start();

		logger.info("Starting orc interface");
		orc = new OrcInterface(config);

		logger.info("Starting ranger manager");
		ranger = new RangerManager();

		//logger.info("Starting Soar interface");
		//soar = new SoarInterface(ranger, config);

		logger.info("Creating game pad for override");
		gamePad = new GamePad();

		Runtime.getRuntime().addShutdownHook(new ShutdownHook());

		logger.info("Ready");
		while (running) {
			try {
				updateOverride();
				updateStartStop();
				Thread.sleep(50);
			} catch (InterruptedException ignored) {
			}
		}
	}

	boolean overrideEnabled = false;
	boolean overrideButton = false;
	differential_drive_command_t overrideCommand = new differential_drive_command_t();
	boolean tankMode = false;

	private void updateOverride() {
		boolean currentOverrideButton = gamePad.getButton(0);
		// change on trailing edge
		if (overrideButton && !currentOverrideButton) {
			overrideEnabled = !overrideEnabled;
			soar.setOverride(overrideEnabled);

			if (overrideEnabled) {
				overrideCommand.left_enabled = true;
				overrideCommand.right_enabled = true;
				overrideCommand.left = 0;
				overrideCommand.right = 0;
				overrideCommand.utime = System.nanoTime() / 1000;
				lcm.publish(LCMInfo.DRIVE_COMMANDS_CHANNEL, overrideCommand);
			}

			logger.info("Override " + (overrideEnabled ? "enabled" : "disabled"));
		}
		overrideButton = currentOverrideButton;

		if (overrideEnabled) {
			differential_drive_command_t newCommand = new differential_drive_command_t();
			newCommand.left_enabled = true;
			newCommand.right_enabled = true;

			if (tankMode) {
				newCommand.left = gamePad.getAxis(1) * -1;
				newCommand.right = gamePad.getAxis(3) * -1;
			} else {
				double fwd = -1 * gamePad.getAxis(3); // +1 = forward, -1 = back
				double lr = -1 * gamePad.getAxis(2); // +1 = left, -1 = right

				newCommand.left = fwd - lr;
				newCommand.right = fwd + lr;

				double max = Math.max(Math.abs(newCommand.left), Math.abs(newCommand.right));
				if (max > 1) {
					newCommand.left /= max;
					newCommand.right /= max;
				}
			}
			if ((newCommand.left != overrideCommand.left) || (newCommand.right != overrideCommand.right)) {
				overrideCommand = newCommand;
				overrideCommand.utime = System.nanoTime() / 1000;
				lcm.publish(LCMInfo.DRIVE_COMMANDS_CHANNEL, overrideCommand);
			}
		}
	}

	boolean startStopButton = false;
	boolean startStop = false;

	private void updateStartStop() {
		boolean currentStartStopButton = gamePad.getButton(1);
		// change on trailing edge
		if (startStopButton && !currentStartStopButton) {
			startStop = !startStop;

			if (startStop) {
				logger.info("Starting Soar");
				soar.start();
			} else {
				logger.info("Stop Soar requested");
				soar.stop();
			}
		}
		startStopButton = currentStartStopButton;
	}

	public class ShutdownHook extends Thread {
		@Override
		public void run() {
			running = false;

			soar.shutdown();
			orc.shutdown();

			System.out.flush();
			System.err.println("Terminated");
			System.err.flush();
		}
	}

	public static void main(String args[]) {
		new SplinterSoar(args);
	}
}
