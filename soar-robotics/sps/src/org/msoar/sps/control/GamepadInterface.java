package org.msoar.sps.control;

import lcmtypes.differential_drive_command_t;
import orc.util.GamePad;

public class GamepadInterface {
	private GamePad gp = new GamePad();
	private ModeButton override;
	private ModeButton soarControl;
	private ModeButton tankMode;
	private ModeButton slowMode;
	private ModeButton captureMode;
	private ModeButton tagCapture;
	
	GamepadInterface() {
		override = new ModeButton("Override", gp, 0);
		soarControl = new ModeButton("Soar control", gp, 1);
		tankMode = new ModeButton("Tank mode", gp, 2);
		slowMode = new ModeButton("Slow mode", gp, 3);
		captureMode = new ModeButton("Capture mode", gp, 4);
		tagCapture = new ModeButton("Tag capture", gp, 5);
	}
	
	public void update() {
		override.update();
		if (override.isEnabled()) {
			tankMode.update();
		}
		
		soarControl.update();
		slowMode.update();

		captureMode.update();
		if (captureMode.isEnabled()) {
			tagCapture.update();
		}
	}
	
	public boolean getOverrideMode() {
		return override.isEnabled();
	}

	public boolean getSoarControlButton() {
		return soarControl.checkAndDisable();
	}
	
	public boolean getSlowMode() {
		return slowMode.isEnabled();
	}

	public boolean getCaptureMode() {
		return captureMode.isEnabled();
	}

	public boolean getTagCapture() {
		return tagCapture.checkAndDisable();
	}

	public void getDC(differential_drive_command_t dc) {
		dc.left_enabled = true;
		dc.right_enabled = true;
		
		if (tankMode.isEnabled()) {
			dc.left = gp.getAxis(1) * -1;
			dc.right = gp.getAxis(3) * -1;
		} else {
			// this should not be linear, it is difficult to precicely control
			double fwd = -1 * gp.getAxis(3); // +1 = forward, -1 = back
			double lr = -1 * gp.getAxis(2); // +1 = left, -1 = right

			dc.left = fwd - lr;
			dc.right = fwd + lr;

			double max = Math.max(Math.abs(dc.left), Math.abs(dc.right));
			if (max > 1) {
				dc.left /= max;
				dc.right /= max;
			}
		}
	}
}
