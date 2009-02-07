package sps.control;

import java.util.Arrays;

import jmat.MathUtil;

import lcmtypes.differential_drive_command_t;

class SplinterInput {
	private double[] throttle = { 0, 0 };
	private double targetYaw = 0;
	private double targetYawTolerance = 0;
	private boolean targetYawEnabled = false;

	enum Direction {
		left, right
	};

	private SplinterInput() {
	}

	public SplinterInput(double throttle) {
		Arrays.fill(this.throttle, throttle);
	}

	public SplinterInput(Direction dir, double throttle) {
		this.throttle[0] = throttle * (dir == Direction.left ? -1 : 1);
		this.throttle[1] = throttle * (dir == Direction.right ? -1 : 1);
	}

	public SplinterInput(double[] throttle) {
		System.arraycopy(throttle, 0, this.throttle, 0, throttle.length);
	}

	public SplinterInput(double yaw, double tolerance, double throttle) {
		this.targetYaw = yaw;
		this.targetYawTolerance = tolerance;
		this.targetYawEnabled = true;
		Arrays.fill(this.throttle, throttle);
	}

	public SplinterInput copy() {
		SplinterInput other = new SplinterInput();

		System.arraycopy(throttle, 0, other.throttle, 0, throttle.length);
		other.targetYaw = targetYaw;
		other.targetYawTolerance = targetYaw;
		other.targetYawEnabled = targetYawEnabled;

		return other;
	}

	public void getDC(differential_drive_command_t dc, double currentYaw) {
		dc.left_enabled = true;
		dc.right_enabled = true;

		if (targetYawEnabled) {
			double relativeBearingValue = targetYaw - currentYaw;
			relativeBearingValue = MathUtil.mod2pi(relativeBearingValue);

			if (relativeBearingValue < (0 - targetYawTolerance)) {
				dc.left = throttle[0];
				dc.right = throttle[1] * -1;
			} else if (relativeBearingValue > targetYawTolerance) {
				dc.left = throttle[0] * -1;
				dc.right = throttle[1];
			} else {
				dc.left = 0;
				dc.right = 0;
			}
		} else {
			dc.left = throttle[0];
			dc.right = throttle[1];
		}
	}
}
