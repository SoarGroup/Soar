package edu.umich.soar.sproom.control;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import jmat.LinAlg;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.HzChecker;
import edu.umich.soar.sproom.control.HttpControllerEvent.DDCChanged;
import edu.umich.soar.sproom.control.HttpControllerEvent.MessageChanged;
import edu.umich.soar.sproom.control.HttpControllerEvent.SoarChanged;

import april.config.Config;
import april.config.ConfigUtil;

public class Controller implements Runnable {
	private static final Log logger = LogFactory.getLog(Controller.class);
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
	private final HttpController httpController = HttpController.newInstance();
	private final SplinterModel splinter = SplinterModel.newInstance();
	private DifferentialDriveCommand ddc = DifferentialDriveCommand.newEStopCommand();
	private boolean override = false;
	private GamepadInputScheme gpInputScheme = GamepadInputScheme.JOY_MOTOR;
	private final HzChecker hzChecker = HzChecker.newInstance(Controller.class.toString());
	private final ScheduledExecutorService schexec = Executors.newSingleThreadScheduledExecutor();

	public Controller(Config config) {
		if (config == null) {
			throw new NullPointerException();
		}
		this.config = config;

		httpController.setGains(splinter.getHGains(), splinter.getAGains(), splinter.getLGains());
		httpController.addEventHandler(new HttpControllerEventHandler() {
			@Override
			public void handleEvent(HttpControllerEvent event) {
				if (event instanceof SoarChanged) {
					soarChanged();
				} else if (event instanceof DDCChanged) {
					synchronized(this) {
						DDCChanged ddcevent = (DDCChanged)event;
						logger.trace("http: " + ddcevent.getDdc());
						ddc = ddcevent.getDdc();
					}
				} else if (event instanceof MessageChanged) {
					MessageChanged mc = (MessageChanged)event;
					soar.newMessage("http", mc.getTokens());
				}
			}
		});
		
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
		schexec.scheduleAtFixedRate(this, 0, 33, TimeUnit.MILLISECONDS); // 30 Hz
	}
	
	private void soarChanged() {
		soar.changeRunningState();
		ddc = DifferentialDriveCommand.newMotorCommand(0, 0);
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
	
			if (Buttons.SOAR.checkAndDisable()) {
				soarChanged();
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
				synchronized(this) {
					ddc = getGPDDCommand();
					logger.trace("gmpd: " + ddc);
				}
			} else {
				if (soar.hasDDCommand()) {
					synchronized(this) {
						ddc = soar.getDDCommand();
						logger.trace("soar: " + ddc);
					}
				} else {
					logger.trace("cont: " + ddc);
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
		new Controller(ConfigUtil.getDefaultConfig(args));
	}

}
