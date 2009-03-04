package org.msoar.sps.control.o;

import java.util.Arrays;

import org.apache.log4j.Logger;

import jmat.LinAlg;
import jmat.MathUtil;

import lcmtypes.differential_drive_command_t;
import lcmtypes.pose_t;

class SplinterInput {
	private static final Logger logger = Logger.getLogger(SplinterInput.class);
	private static final double DISABLED = Double.NEGATIVE_INFINITY;
	private static final long NO_INPUT = Long.MIN_VALUE; 
	
	enum Direction {
		left, right
	};
	
	private double[] throttle = new double[2];
	private double targetYaw;
	private double targetYawTolerance;
	private long utime = NO_INPUT;
	
	SplinterInput() {
		clear();
	}
	
	boolean hasInput() {
		return utime != NO_INPUT;
	}
	
	void setUtime(long utime) {
		this.utime = utime;		
	}
	
	private void clear() {
		this.throttle[0] = 0;
		this.throttle[1] = 0;
		this.targetYaw = 0;
		this.targetYawTolerance = DISABLED;
	}
	
	private void debugOut() {
		if (logger.isDebugEnabled()) {
			logger.debug("SplinterInput: " + this);
		}
	}
	
	void stop() {
		clear();
		debugOut();
	}
	
	void move(double throttle) {
		clear();
		Arrays.fill(this.throttle, throttle);
		debugOut();
	}
	
	void rotate(Direction dir, double throttle) {
		clear();
		this.throttle[0] = throttle * (dir == Direction.left ? -1 : 1);
		this.throttle[1] = throttle * (dir == Direction.right ? -1 : 1);
		debugOut();
	}
	
	void motor(double[] throttle) {
		clear();
		System.arraycopy(throttle, 0, this.throttle, 0, throttle.length);
		debugOut();
	}
	
	void rotateTo(double yaw, double tolerance, double throttle) {
		clear();
		this.targetYaw = yaw;
		this.targetYawTolerance = tolerance;
		Arrays.fill(this.throttle, throttle);
		debugOut();
	}

	public String toString() {
		StringBuilder sb = new StringBuilder();
		if (targetYawTolerance != DISABLED) {
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

	CommandStatus getDC(differential_drive_command_t dc, pose_t pose) {
		CommandStatus status = CommandStatus.executing;
		dc.utime = this.utime;
		dc.left_enabled = true;
		dc.right_enabled = true;

		if (targetYawTolerance != DISABLED) {
			if (pose != null) {
				double currentYawRadians = LinAlg.quatToRollPitchYaw(pose.orientation)[2];
				double relativeBearingValue = targetYaw - currentYawRadians;
				relativeBearingValue = MathUtil.mod2pi(relativeBearingValue);
				
				if (Math.abs(relativeBearingValue) < targetYawTolerance) {
					logger.trace("within tolerance: " + relativeBearingValue);
					dc.left = 0;
					dc.right = 0;
					Arrays.fill(this.throttle, 0);
					status = CommandStatus.complete;
					
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
				dc.left = 0;
				dc.right = 0;
			}
			
		} else {
			dc.left = throttle[0];
			dc.right = throttle[1];
		}
		
		if (logger.isDebugEnabled()) {
			logger.debug(String.format("[%b:%f,%b:%f]",dc.left_enabled,dc.left,dc.right_enabled,dc.right));
		}
		
		return status;
	}
}
