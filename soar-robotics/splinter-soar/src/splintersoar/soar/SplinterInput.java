package splintersoar.soar;

import java.util.Arrays;

import jmat.LinAlg;

import erp.lcmtypes.differential_drive_command_t;

import splintersoar.lcmtypes.splinterstate_t;

public class SplinterInput {
	public double[] throttle = { 0, 0 };
	public double targetYaw = 0;
	public double targetYawTolerance = 0;
	public boolean targetYawEnabled = false;

	public enum Direction {
		left, right
	};

	public SplinterInput() {
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

	public differential_drive_command_t generateDriveCommand(splinterstate_t splinterState) {
		differential_drive_command_t driveCommand = new differential_drive_command_t();
		driveCommand.left_enabled = true;
		driveCommand.right_enabled = true;

		if (targetYawEnabled) {
			double relativeBearingValue = targetYaw - LinAlg.quatToRollPitchYaw(splinterState.pose.orientation)[2];
			relativeBearingValue = erp.math.MathUtil.mod2pi(relativeBearingValue);

			if (relativeBearingValue < (0 - targetYawTolerance)) {
				driveCommand.left = throttle[0];
				driveCommand.right = throttle[1] * -1;
			} else if (relativeBearingValue > targetYawTolerance) {
				driveCommand.left = throttle[0] * -1;
				driveCommand.right = throttle[1];
			} else {
				driveCommand.left = 0;
				driveCommand.right = 0;
			}
		} else {
			driveCommand.left = throttle[0];
			driveCommand.right = throttle[1];
		}

		driveCommand.utime = System.nanoTime() / 1000;
		return driveCommand;
	}
}
