package edu.umich.soar.sproom.command;

import java.util.concurrent.atomic.AtomicBoolean;

import jmat.LinAlg;
import jmat.MathUtil;

import lcmtypes.pose_t;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

class Drive2 {
	private static final Log logger = LogFactory.getLog(Drive2.class);
	
	private final AtomicBoolean pidEnabled = new AtomicBoolean(false);
	private double av;
	private double lv;
	private final PIDController aController = new PIDController("angular velocity");
	private final PIDController lController = new PIDController("linear velocity");
	private final Drive1 drive1;

	Drive2(Drive1 drive1) {
		this.drive1 = drive1;
		
		setMotors(0, 0);

		// TODO: make configurable
		setAGains(new double[] { 0.0238, 0, 0.0025 });
		setLGains(new double[] { 0.12,  0, 0.025 });
	}
	
	void setAGains(double[] g) {
		aController.setGains(g);
	}

	void setLGains(double[] g) {
		lController.setGains(g);
	}
	
	double[] getAGains() {
		return aController.getGains();
	}

	double[] getLGains() {
		return lController.getGains();
	}

	void estop() {
		drive1.estop();
	}
	
	void setMotors(double left, double right) {
		pidEnabled.set(false);
		drive1.setMotors(left, right);
	}
	
	void setAngularVelocity(double av) {
		if (pidEnabled.getAndSet(true) == false) {
			resetPidState();
		}
		this.av = av;
	}
	
	void setLinearVelocity(double lv) {
		if (pidEnabled.getAndSet(true) == false) {
			resetPidState();
		}
		this.lv = lv;
	}
	
	private void resetPidState() {
		av = 0;
		lv = 0;
		aController.clearIntegral();
		lController.clearIntegral();
	}
	
	void update(pose_t pose, long utimeElapsed) {
		try {
			if (pose != null) {
				
				// compute
				if (pidEnabled.get()) {
					double dt = utimeElapsed / 1000000.0;
					
					if (Double.compare(pose.vel[2], 0) != 0) {
						logger.warn("Z velocity is not zero!");
					}
		
					if (pose.orientation == null || pose.orientation.length < 3) {
						logger.error("pose has invalid orientation!");
						return;
					}
		
					double aout = aController.compute(dt, av, pose.rotation_rate[2]);
					double left = drive1.getLeft();
					double right = drive1.getRight();
					left -= aout;
					right += aout;
		
					// convert vector to local frame and get forward (x) component
					double theta = MathUtil.mod2pi(LinAlg.quatToRollPitchYaw(pose.orientation)[2]);
					double xvel = LinAlg.rotate2(pose.vel, -theta)[0];
					double lout = lController.compute(dt, lv, xvel);
					left += lout;
					right += lout;

					if (logger.isTraceEnabled()) {
						logger.trace(String.format("a%f l%f", aout, lout));
					}
					drive1.setMotors(left, right);
				}
			}
			
			drive1.update();
			
		} catch (Exception e) {
			e.printStackTrace();
			logger.error("Uncaught exception: " + e);
			estop();
		}
	}
}
