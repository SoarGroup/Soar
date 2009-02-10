package org.msoar.sps.splinter;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;

import jmat.LinAlg;
import jmat.MathUtil;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.differential_drive_command_t;
import lcmtypes.pose_t;

import orc.Motor;
import orc.Orc;
import orc.OrcStatus;

import org.apache.log4j.Logger;
import org.msoar.sps.Names;
import org.msoar.sps.config.Config;
import org.msoar.sps.config.ConfigFile;
import org.msoar.sps.lcmtypes.odom_t;

public class Splinter extends TimerTask implements LCMSubscriber {
	private static Logger logger = Logger.getLogger(Splinter.class);
	private static int LEFT = 0;
	private static int RIGHT = 1;
	private static long DELAY_BEFORE_WARN_NO_FIRST_INPUT_MILLIS = 5000;
	private static long DELAY_BEFORE_WARN_NO_INPUT_MILLIS = 1000;
	
	private Timer timer = new Timer();
	private Orc orc;
	private Motor[] motor = new Motor[2];
	private int[] ports;
	private boolean[] invert;
	private differential_drive_command_t dc;
	private LCM lcm;
	private double tickMeters;
	private double baselineMeters;
	private double[] command = { 0, 0 };
	private double maxThrottleChangePerUpdate;
	private long lastSeenDCTime = 0;
	private long lastUtime = 0;
	private boolean failsafeSpew = false;
	
	// for odometry update
	private pose_t pose = new pose_t();
	private odom_t odom = new odom_t();
	private odom_t newOdom = new odom_t();
	private double[] deltaxyt = new double[3];
	
	Splinter(Config config) {
		tickMeters = config.getDouble("tickMeters", 0.000043225);
		baselineMeters = config.getDouble("baselineMeters", 0.383);
		
		double maxThrottleAccelleration = config.getDouble("maxThrottleAccelleration", 2.0);
		double updateHz = config.getDouble("updateHz", 30.0);
		maxThrottleChangePerUpdate = maxThrottleAccelleration / updateHz;
		
		orc = Orc.makeOrc(config.getString("orcHostname", "192.168.237.7"));

		// ports: what port the motors are hooked up to, invert: polarity
		ports = config.getInts("ports", new int[] {0,1});
		invert = config.getBooleans("invert", new boolean[] {true, true});
		
		motor[LEFT] = new Motor(orc, ports[LEFT], invert[LEFT]);
		motor[RIGHT] = new Motor(orc, ports[RIGHT], invert[RIGHT]);
		
		getOdometry(odom, orc.getStatus());
		
		// drive commands
		lcm = LCM.getSingleton();
		lcm.subscribe(Names.DRIVE_CHANNEL, this);
	
		double updatePeriodMS = 1000 / updateHz;
		logger.debug("Splinter thread running, period " + updatePeriodMS);
		timer.schedule(this, 0, (long)updatePeriodMS); 
	}
	
	private void getOdometry(odom_t dest, OrcStatus currentStatus) {
		dest.left = currentStatus.qeiPosition[ports[LEFT]] * (invert[LEFT] ? -1 : 1);
		dest.right = currentStatus.qeiPosition[ports[RIGHT]] * (invert[RIGHT] ? -1 : 1);
	}
	
	@Override
	public void run() {
		// Get OrcStatus
		OrcStatus currentStatus = orc.getStatus();
		
		boolean moving = (currentStatus.qeiVelocity[0] != 0) || (currentStatus.qeiVelocity[1] != 0);
		
		getOdometry(newOdom, currentStatus);
		
		// don't update odom unless moving
		if (moving) {
			double dleft = (newOdom.left - odom.left) * tickMeters;
			double dright = (newOdom.right - odom.right) * tickMeters;
			
			// phi
			deltaxyt[2] = MathUtil.mod2pi((dright - dleft) / baselineMeters);
			
			double dCenter = (dleft + dright) / 2;
			
			// delta x, delta y
			deltaxyt[0] += dCenter * Math.cos(deltaxyt[2]);
			deltaxyt[1] += dCenter * Math.sin(deltaxyt[2]);
			
			// our current theta
			double theta = LinAlg.quatToRollPitchYaw(pose.orientation)[2];
			
			// calculate and store new xyt
			pose.pos[0] += (deltaxyt[0] * Math.cos(theta)) - (deltaxyt[1] * Math.sin(theta));
			pose.pos[1] += (deltaxyt[0] * Math.sin(theta)) + (deltaxyt[1] * Math.cos(theta));
			theta += deltaxyt[2];
			
			// convert theta to quat and store
			pose.orientation = LinAlg.rollPitchYawToQuat(new double[] {0, 0, theta});
			
			if (logger.isTraceEnabled()) {
				theta = MathUtil.mod2pi(theta);
				theta = Math.toDegrees(theta);
				logger.trace(String.format(" odom: %5.2f %5.2f %5.1f", pose.pos[0], pose.pos[1], theta));
			}
		}

		// save old state
		odom.left = newOdom.left;
		odom.right = newOdom.right;
		
		pose.utime = currentStatus.utime;
		lcm.publish(Names.POSE_CHANNEL, pose);
		
		odom.utime = currentStatus.utime;
		lcm.publish(Names.ODOM_CHANNEL, odom);
		
		commandMotors();
	}

	private void commandMotors() {
		if (dc == null) {
			if (lastSeenDCTime == 0) {
				lastSeenDCTime = System.currentTimeMillis();
			}
			long millisSinceLastCommand = System.currentTimeMillis() - lastSeenDCTime;
			if (millisSinceLastCommand > DELAY_BEFORE_WARN_NO_FIRST_INPUT_MILLIS) {
				logger.warn("Haven't seen a drive command yet!"); 
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
					logger.error("Haven't seen new drive command in " 
							+ millisSinceLastCommand / 1000.0 
							+ " seconds, commanding fail safe");
					failsafeSpew = true;
				}
				commandFailSafe();
				return;
			}
		}
		
		failsafeSpew = false;		
		if (logger.isTraceEnabled()) {
			logger.trace(String.format("Got input %f %f", dcNew.left, dcNew.right));
		}
		
		if (dcNew.left_enabled) {
			double delta = dcNew.left - command[LEFT];
			
			if (delta > 0) {
				delta = Math.min(delta, maxThrottleChangePerUpdate);
			} else if (delta < 0) {
				delta = Math.max(delta, -1 * maxThrottleChangePerUpdate);
			}

			command[LEFT] += delta;
			motor[LEFT].setPWM(command[LEFT]);
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
			motor[RIGHT].setPWM(command[RIGHT]);
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

	@Override
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(Names.DRIVE_CHANNEL)) {
			try {
				dc = new differential_drive_command_t(ins);
			} catch (IOException e) {
				logger.error("Error decoding differential_drive_command_t message: " + e.getMessage());
			}
		}
	}
}
