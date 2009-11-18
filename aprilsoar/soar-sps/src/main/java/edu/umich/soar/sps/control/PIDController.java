package edu.umich.soar.sps.control;

import jmat.MathUtil;

class PIDController {
	static class Gains {
		Gains() {
			this.p = 0;
			this.i = 0;
			this.d = 0;
		}
		
		Gains(double p, double i, double d) {
			this.p = p;
			this.i = i;
			this.d = d;
		}

		final double p;
		final double i;
		final double d;
	}
	
	private Gains gains = new Gains();
	private double previousError;
	private double integral;
	private double previousTarget;
	
	PIDController() {
	}
	
	void setGains(Gains gains) {
		this.gains = gains;
	}
	
	Gains getGains() {
		return gains;
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

		double output = gains.p * error;
		output += gains.i * integral;
		output += gains.d * derivative;

		previousError = error;
		previousTarget = target;
		
		return output;
	}
}
