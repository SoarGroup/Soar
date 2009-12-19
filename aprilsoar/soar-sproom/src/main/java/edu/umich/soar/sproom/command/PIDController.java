package edu.umich.soar.sproom.command;

import java.util.Arrays;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import jmat.MathUtil;

class PIDController {
	private static final Log logger = LogFactory.getLog(PIDController.class);
	
	private final double[] pid = new double[] { 0, 0, 0 };
	private double previousError;
	private double integral;
	private double previousTarget;
	private final String name;
	
	PIDController(String name) {
		this.name = name;
	}
	
	void setGains(double[] pid) {
		System.arraycopy(pid, 0, this.pid, 0, pid.length);
		logger.info("New gains for " + name + ": " + Arrays.toString(this.pid));
	}
	
	double[] getGains() {
		return Arrays.copyOf(pid, pid.length);
	}
	
	void clearIntegral() {
		integral = 0;
	}
	
	double computeMod2Pi(double dt, double target, double actual) {
		return computeInternal(dt, target, actual, true);
	}
	
	double compute(double dt, double target, double actual) {
		return computeInternal(dt, target, actual, false);
	}
	
	private double computeInternal(double dt, double target, double actual, boolean mod2pi) {
		if (Double.compare(dt, 0) == 0) {
			return 0;
		}
		
		if (Math.signum(target) != Math.signum(previousTarget)) {
			integral = 0;
		}
		
		double error = target - actual;
		if (mod2pi) {
			error = MathUtil.mod2pi(error);
		}
		integral = integral + error * dt;
		double derivative = (error - previousError) / dt;

		double output = pid[0] * error;
		output += pid[1] * integral;
		output += pid[2] * derivative;

		previousError = error;
		previousTarget = target;
		
		return output;
	}
}
