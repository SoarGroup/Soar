package edu.umich.soar.sproom.splinter;

import java.io.IOException;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import april.config.Config;
import april.config.ConfigFile;

import edu.umich.soar.sproom.HzChecker;
import edu.umich.soar.sproom.Odometry;
import edu.umich.soar.sproom.OdometryPoint;
import edu.umich.soar.sproom.SharedNames;

import lcm.lcm.LCM;
import lcm.lcm.LCMDataInputStream;
import lcm.lcm.LCMSubscriber;
import lcmtypes.differential_drive_command_t;
import lcmtypes.pose_t;

import orc.Motor;
import orc.Orc;
import orc.OrcStatus;

/**
 * Interface to the orc board on a splinter bot.
 *
 * @author voigtjr@gmail.com
 */
public class Splinter {
	private static final Log logger = LogFactory.getLog(Splinter.class);
	private static final int LEFT = 0;
	private static final int RIGHT = 1;
	private static final long DELAY_BEFORE_WARN_NO_FIRST_INPUT_MILLIS = 5000;
	private static final long DELAY_BEFORE_WARN_NO_INPUT_MILLIS = 1000;
	
	//green
	//public static final double DEFAULT_BASELINE = 0.383;
	//public static final double DEFAULT_TICKMETERS = 0.000043225;
	
	// blue
	public static final double DEFAULT_BASELINE = 0.37405;
	// TODO: should use two tickmeters, left/right
	public static final double DEFAULT_TICKMETERS = 0.0000428528;
	
	private final HzChecker hzChecker = HzChecker.newInstance(Splinter.class.toString());
	private final Orc orc;
	private final Motor[] motor = new Motor[2];
	private final int[] ports;
	private final boolean[] invert;
	private final LCM lcm;
	private final double tickMeters;
	private final double baselineMeters;
	private final double[] command = { 0, 0 };
	private final double[] minimumMotion = { 0.1299, 0.1481 };
	private final double maxThrottleChangePerUpdate;

	private OdometryLogger capture;
	private differential_drive_command_t dc;
	private long lastSeenDCTime = 0;
	private long lastUtime = 0;
	private boolean failsafeSpew = false;
	
	private enum CalibrateState { NO, STOP1, RIGHT, STOP2, LEFT, YES }
	private CalibrateState calibrated = CalibrateState.YES;

	// for odometry update
	private final Odometry odometry;
	private OdometryPoint oldOdom;
	private final pose_t pose = new pose_t();
	private final ScheduledExecutorService schexec = Executors.newSingleThreadScheduledExecutor();

	public Splinter(Config config) {
		config = config.getChild("splinter");
		
		tickMeters = config.getDouble("tickMeters", DEFAULT_TICKMETERS);
		baselineMeters = config.getDouble("baselineMeters", DEFAULT_BASELINE);
		odometry = new Odometry(tickMeters, baselineMeters);
		
		double maxThrottleAccelleration = config.getDouble("maxThrottleAccelleration", 2.0);
		double updateHz = config.getDouble("updateHz", 30.0);
		maxThrottleChangePerUpdate = maxThrottleAccelleration / updateHz;
		
		orc = Orc.makeOrc(config.getString("orcHostname", "192.168.237.7"));

		// ports: what port the motors are hooked up to, invert: polarity
		ports = config.getInts("ports", new int[] {0,1});
		invert = config.getBooleans("invert", new boolean[] {true, true});
		
		motor[LEFT] = new Motor(orc, ports[LEFT], invert[LEFT]);
		motor[RIGHT] = new Motor(orc, ports[RIGHT], invert[RIGHT]);
		
		OrcStatus currentStatus = orc.getStatus();
		oldOdom = getOdometry(currentStatus);
		
		if (config.getBoolean("captureOdometry", false)) {
			try {
				capture = new OdometryLogger();
				capture.record(oldOdom);
			} catch (IOException e) {
				logger.error("Error opening odometry logger: " + e.getMessage());
			}
		}
		
		// drive commands
		lcm = LCM.getSingleton();
		lcm.subscribe(SharedNames.DRIVE_CHANNEL, new LCMSubscriber() {
			@Override
			public void messageReceived(LCM lcm, String channel, LCMDataInputStream ins) {
				if (channel.equals(SharedNames.DRIVE_CHANNEL)) {
					try {
						dc = new differential_drive_command_t(ins);
					} catch (IOException e) {
						logger.error("Error decoding differential_drive_command_t message: " + e.getMessage());
					}
				}
			}
		});
	
		double updatePeriodMS = 1000 / updateHz;
		logger.debug("Splinter thread running, period set to " + updatePeriodMS);
		schexec.scheduleAtFixedRate(update, 0, (long)updatePeriodMS, TimeUnit.MILLISECONDS); 
	}
	
	private OdometryPoint getOdometry(OrcStatus currentStatus) {
		return OdometryPoint.newInstance(currentStatus.qeiPosition[ports[LEFT]] * -1, currentStatus.qeiPosition[ports[RIGHT]]);
	}
	
	private void calibrate(OrcStatus currentStatus) {
		double dt = (currentStatus.utime - lastUtime) / 1000000.0;
		boolean moving = (currentStatus.qeiVelocity[0] != 0) || (currentStatus.qeiVelocity[1] != 0);
		switch (calibrated) {
		case NO:
			logger.info("Calibrating, please wait...");
			commandFailSafe();
			calibrated = CalibrateState.STOP1;
			break;
			
		case STOP1:
			if (moving) {
				break;
			}
			calibrated = CalibrateState.RIGHT;
			// falls through

		case RIGHT:
			if (!moving) {
				command[RIGHT] += 0.10 * dt;
				motor[RIGHT].setPWM(command[RIGHT]);
				break;
			}
			minimumMotion[RIGHT] = command[RIGHT];
			commandFailSafe();
			calibrated = CalibrateState.STOP2;
			break;
			
		case STOP2:
			if (moving) {
				break;
			}
			calibrated = CalibrateState.LEFT;
			// falls through

		case LEFT:
			if (!moving) {
				command[LEFT] += 0.10 * dt;
				motor[LEFT].setPWM(command[LEFT]);
				break;
			}
			lastUtime = 0;
			minimumMotion[LEFT] = command[LEFT];
			commandFailSafe();
			calibrated = CalibrateState.YES;
			// values are always going to be high due to system delay, back of 20%
			minimumMotion[LEFT] *= .8;
			minimumMotion[RIGHT] *= .8;
			logger.info(String.format("Minimum motion throttle l%1.4f r%1.4f", minimumMotion[LEFT], minimumMotion[RIGHT]));
			break;
		}
		if (calibrated == CalibrateState.YES) {
			lastUtime = 0;
		} else {
			lastUtime = currentStatus.utime;
		}
	}
	
	private Runnable update = new Runnable() {
		
		@Override
		public void run() {
			try {
				if (logger.isDebugEnabled()) {
					hzChecker.tick();
				}
				
				// Get OrcStatus
				OrcStatus currentStatus = orc.getStatus();
		
				if (calibrated != CalibrateState.YES) {
					calibrate(currentStatus);
					return;
				}
				
				boolean moving = (currentStatus.qeiVelocity[0] != 0) || (currentStatus.qeiVelocity[1] != 0);
				
				OdometryPoint newOdom = getOdometry(currentStatus);
				
				// don't update odom unless moving
				if (moving) {
					odometry.propagate(newOdom, oldOdom, pose);
		
					if (capture != null) {
						try {
							capture.record(newOdom);
						} catch (IOException e) {
							logger.error("IOException while writing odometry: " + e.getMessage());
						}
					}
				}
		
				// save old state
				oldOdom = newOdom;
				
				pose.utime = currentStatus.utime;
				lcm.publish(SharedNames.POSE_CHANNEL, pose);
				
				commandMotors();
			} catch (Exception e) {
				e.printStackTrace();
				logger.error("Uncaught exception: " + e);
				commandFailSafe();
			}
		}
	};
	
	private double mapThrottle(double input, int motor) {
		double output = ((1 - minimumMotion[motor]) * Math.abs(input)) + minimumMotion[motor];
		return Math.signum(input) * output;
	}
	
	private void commandMotors() {
		if (dc == null) {
			if (lastSeenDCTime == 0) {
				lastSeenDCTime = System.currentTimeMillis();
			}
			long millisSinceLastCommand = System.currentTimeMillis() - lastSeenDCTime;
			if (millisSinceLastCommand > DELAY_BEFORE_WARN_NO_FIRST_INPUT_MILLIS) {
				logger.warn("No drive command yet"); 
				lastSeenDCTime = System.currentTimeMillis();
			}
			// we haven't seen a drive command yet
			commandFailSafe();
			return;
		}

		differential_drive_command_t dcNew = dc.copy();

		// is it a new command? 
		if (lastUtime != dcNew.utime) {
			// it is, save state
			lastSeenDCTime = System.currentTimeMillis();
			lastUtime = dcNew.utime;
		} else {
			// have we not seen a command in the last second?
			long millisSinceLastCommand = System.currentTimeMillis() - lastSeenDCTime;
			if (millisSinceLastCommand > DELAY_BEFORE_WARN_NO_INPUT_MILLIS) {
				if (failsafeSpew == false) {
					logger.error("No recent drive command " 
							+ millisSinceLastCommand / 1000.0 
							+ " seconds");
					failsafeSpew = true;
				}
				commandFailSafe();
				return;
			}
		}
		
		failsafeSpew = false;		
		if (logger.isTraceEnabled()) {
			logger.trace(String.format("Got input %1.2f %1.2f", dcNew.left, dcNew.right));
		}
		
		if (dcNew.left_enabled) {
			double delta = dcNew.left - command[LEFT];
			
			if (delta > 0) {
				delta = Math.min(delta, maxThrottleChangePerUpdate);
			} else if (delta < 0) {
				delta = Math.max(delta, -1 * maxThrottleChangePerUpdate);
			}

			command[LEFT] += delta;
			motor[LEFT].setPWM(mapThrottle(command[LEFT], LEFT));
		} else {
			motor[LEFT].idle();
		}

		if (dcNew.right_enabled) {
			double delta = dcNew.right - command[RIGHT];
			
			if (delta > 0) {
				delta = Math.min(delta, maxThrottleChangePerUpdate);
			} else if (delta < 0) {
				delta = Math.max(delta, -1 * maxThrottleChangePerUpdate);
			}

			command[RIGHT] += delta;
			motor[RIGHT].setPWM(mapThrottle(command[RIGHT], RIGHT));
		} else {
			motor[RIGHT].idle();
		}

		if (logger.isTraceEnabled()) {
			logger.trace("Sending input " + (dcNew.left_enabled ? command[LEFT] : "(idle)") 
					+ " "
					+ (dcNew.right_enabled ? command[RIGHT] : "(idle)"));
		}
	}
	
	private void commandFailSafe() {
		command[RIGHT] = 0;
		command[LEFT] = 0;
		motor[LEFT].setPWM(0);
		motor[RIGHT].setPWM(0);
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
		new Splinter(config);
	}

}
