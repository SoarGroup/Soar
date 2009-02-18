package org.msoar.sps.control.io;

import java.util.Arrays;

import org.apache.log4j.Logger;

import jmat.MathUtil;

import lcmtypes.differential_drive_command_t;

class SplinterInput {
	private static Logger logger = Logger.getLogger(SplinterInput.class);

	private double[] throttle = { 0, 0 };
	private double targetYaw = 0;
	private double targetYawTolerance = 0;
	private boolean targetYawEnabled = false;
	private long utime = 0;

	public String toString() {
		StringBuilder sb = new StringBuilder();
		if (targetYawEnabled) {
			sb.append("--> ");
			sb.append(targetYaw);
			sb.append(" (");
			sb.append(targetYawTolerance);
			sb.append(") ");
		}
		sb.append(Arrays.toString(throttle));
		sb.append(" ");
		sb.append(utime);
		return sb.toString();
	}
	enum Direction {
		left, right
	};

	public SplinterInput(double throttle) {
		Arrays.fill(this.throttle, throttle);
		if (logger.isDebugEnabled()) {
			logger.debug("SplinterInput: " + this);
		}
	}

	public SplinterInput(Direction dir, double throttle) {
		this.throttle[0] = throttle * (dir == Direction.left ? -1 : 1);
		this.throttle[1] = throttle * (dir == Direction.right ? -1 : 1);
		if (logger.isDebugEnabled()) {
			logger.debug("SplinterInput: " + this);
		}
	}

	public SplinterInput(double[] throttle) {
		System.arraycopy(throttle, 0, this.throttle, 0, throttle.length);
		if (logger.isDebugEnabled()) {
			logger.debug("SplinterInput: " + this);
		}
	}

	public SplinterInput(double yaw, double tolerance, double throttle) {
		this.targetYaw = yaw;
		this.targetYawTolerance = tolerance;
		this.targetYawEnabled = true;
		Arrays.fill(this.throttle, throttle);
		if (logger.isDebugEnabled()) {
			logger.debug("SplinterInput: " + this);
		}
	}

	public void getDC(differential_drive_command_t dc, double currentYawRadians) {
		dc.utime = this.utime;
		dc.left_enabled = true;
		dc.right_enabled = true;

		if (targetYawEnabled) {
			double relativeBearingValue = targetYaw - currentYawRadians;
			relativeBearingValue = MathUtil.mod2pi(relativeBearingValue);
			
			if (Math.abs(relativeBearingValue) < targetYawTolerance) {
				logger.trace("within tolerance: " + relativeBearingValue);
				dc.left = 0;
				dc.right = 0;
			} else if (relativeBearingValue < 0) {
				if (logger.isTraceEnabled()) {
					logger.trace("right turn: " + relativeBearingValue);
				}
				dc.left = throttle[0];
				dc.right = throttle[1] * -1;
			} else if (relativeBearingValue > 0) {
				if (logger.isTraceEnabled()) {
					logger.trace("left turn: " + relativeBearingValue);
				}
				dc.left = throttle[0] * -1;
				dc.right = throttle[1];
			} else {
				throw new IllegalStateException();
			}
		} else {
			dc.left = throttle[0];
			dc.right = throttle[1];
		}
		
		if (logger.isDebugEnabled()) {
			logger.debug(String.format("[%b:%f,%b:%f]",dc.left_enabled,dc.left,dc.right_enabled,dc.right));
		}
	}

	public void setUtime(long utime) {
		this.utime = utime;		
	}
}
