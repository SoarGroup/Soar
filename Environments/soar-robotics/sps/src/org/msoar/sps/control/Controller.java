package org.msoar.sps.control;

import java.io.IOException;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import jmat.LinAlg;

import org.apache.log4j.Logger;
import org.msoar.sps.HzChecker;
import org.msoar.sps.config.Config;
import org.msoar.sps.config.ConfigFile;

final class Controller extends TimerTask {
	private static final Logger logger = Logger.getLogger(Controller.class);
	private static final double LIN_MAX = 0.5; //0.602;				// experimentally derived
	private static final double ANG_MAX = Math.toRadians(189);	// experimentally derived
	private static final double ZERO_THRESHOLD = 0.4; 			// for GAS_AND_WHEEL, what is angvel = 0
	private enum GamepadInputScheme {
		JOY_MOTOR,		// left: off, right x: turn component, right y: forward component
		TANK,			// left y: left motor, right y: right motor
		JOY_VELOCITIES,	// left: off, right x: linvel, right y: angvel
		GAS_AND_WHEEL,	// left y: linvel, right: heading, right center: angvel -> 0
	}
	
	private final Config config;
	private final Gamepad gp;
	private final SoarInterface soar;
	private final Timer timer = new Timer();
	private final HttpController httpController = HttpController.newInstance();
	private final SplinterModel splinter = SplinterModel.newInstance();
	private DifferentialDriveCommand ddc = DifferentialDriveCommand.newEStopCommand();
	private boolean override = false;
	private GamepadInputScheme gpInputScheme = GamepadInputScheme.JOY_MOTOR;
	private final HzChecker hzChecker = new HzChecker(logger);

	private Controller(Config config) {
		if (config == null) {
			throw new NullPointerException();
		}
		this.config = config;
		
		httpController.setSplinter(splinter);
		
		Gamepad gamepad = null;
		try {
			gamepad = new Gamepad();
			Buttons.setGamepad(gamepad);
		} catch (IllegalStateException e) {
			logger.warn("Disabling gamepad: " + e.getMessage());
		}
		gp = gamepad;

		soar = SoarInterface.newInstance(this.config, splinter);

		Runtime.getRuntime().addShutdownHook(new ShutdownHook());

		// TODO: make configurable
		timer.schedule(this, 0, 1000 / 30); // 30 Hz	
		
		if (logger.isDebugEnabled()) {
			timer.schedule(hzChecker, 0, 5000); 
		}
	}
	
	private class ShutdownHook extends Thread {
		@Override
		public void run() {
			soar.shutdown();

			System.out.flush();
			System.err.println("Terminated");
			System.err.flush();
		}
	}

	@Override
	public void run() {
		try {
			if (logger.isDebugEnabled()) {
				hzChecker.tick();
			}
			
			for (Buttons button : Buttons.values()) {
				button.update();
			}
	
			List<String> messageTokens = httpController.getMessageTokens();
			if (messageTokens != null) {
				soar.setStringInput(messageTokens);
			}
	
			if (Buttons.SOAR.checkAndDisable()) {
				soar.changeRunningState();
				ddc = DifferentialDriveCommand.newMotorCommand(0, 0);
			}
			
			if (Buttons.OVERRIDE.checkAndDisable()) {
				override = !override;
				logger.info("Override " + (override ? "enabled" : "disabled"));
				ddc = DifferentialDriveCommand.newMotorCommand(0, 0);
			}
			
			if (Buttons.GPMODE.checkAndDisable()) {
				// new scheme = (current scheme + 1) mod (num schemes)
				gpInputScheme = GamepadInputScheme.values()[(gpInputScheme.ordinal() + 1) % GamepadInputScheme.values().length];
				logger.info("GP scheme " + gpInputScheme);
			}
	
			if (override) {
				ddc = getGPDDCommand();
				logger.trace("gmpd: " + ddc);
			} else {
				if (httpController.hasDDCommand()) {
					ddc = httpController.getDDCommand();
					logger.trace("http: " + ddc);
				} else {
					if (soar.hasDDCommand()) {
						ddc = soar.getDDCommand();
						logger.trace("soar: " + ddc);
					} else {
						logger.trace("cont: " + ddc);
					}
				}
			}
	
			splinter.update(ddc);
		} catch (Exception e) {
			e.printStackTrace();
			logger.error("Uncaught exception: " + e);
			ddc = DifferentialDriveCommand.newEStopCommand();
		}
	}
	
	private DifferentialDriveCommand getGPDDCommand() {
		//double left_x;
		double right_x = gp.getAxis(2);
		double left_y = gp.getAxis(1) * -1;
		double right_y = gp.getAxis(3) * -1;
		
		double left = 0;
		double right = 0;
		
		DifferentialDriveCommand ddc = DifferentialDriveCommand.newEStopCommand();
		
		switch (gpInputScheme) {
		case JOY_MOTOR:
			// this should not be linear, it is difficult to precisely control
			double fwd = right_y; // +1 = forward, -1 = back
			double lr = -1 * right_x; // +1 = left, -1 = right

			left = fwd - lr;
			right = fwd + lr;

			double max = Math.max(Math.abs(left), Math.abs(right));
			if (max > 1) {
				left /= max;
				right /= max;
			}

			if (Buttons.SLOW.isEnabled()) {
				left *= 0.5;
				right *= 0.5;
			}
			ddc = DifferentialDriveCommand.newMotorCommand(left, right);
			break;
			
		case TANK:
			left = left_y;
			right = right_y;

			if (Buttons.SLOW.isEnabled()) {
				left *= 0.5;
				right *= 0.5;
			}
			ddc = DifferentialDriveCommand.newMotorCommand(left, right);
			break;
			
		case JOY_VELOCITIES:
			double angvel = ANG_MAX * right_x * -1;
			double linvel = LIN_MAX * right_y;
			ddc = DifferentialDriveCommand.newVelocityCommand(angvel, linvel);
			break;
			
		case GAS_AND_WHEEL:
			linvel = LIN_MAX * left_y;
			
			double magnitude = LinAlg.magnitude(new double[] { right_x, right_y } );
			if (magnitude < ZERO_THRESHOLD) {
				// angvel to 0
				ddc = DifferentialDriveCommand.newVelocityCommand(0, linvel);
			} else {
				double heading = Math.atan2(right_y, right_x);
				ddc = DifferentialDriveCommand.newHeadingLinearVelocityCommand(heading, linvel);
			}
			break;
		}
		
		return ddc;
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
