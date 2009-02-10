package org.msoar.sps.control;

import org.apache.log4j.Logger;

import lcmtypes.differential_drive_command_t;
import orc.util.GamePad;

public class GamepadInterface {
	private static Logger logger = Logger.getLogger(Controller.class);

	private GamePad gp = new GamePad();
	private ModeButton override;
	private ModeButton soarControl;
	private ModeButton tankMode;
	private ModeButton slowMode;
	
	GamepadInterface() {
		override = new ModeButton(gp, 0);
		soarControl = new ModeButton(gp, 1);
		tankMode = new ModeButton(gp, 2);
		slowMode = new ModeButton(gp, 3);
	}
	
	public void update() {
		override.update();
		logger.info("Override " + (override.isEnabled() ? "enabled" : "disabled"));
		
		soarControl.update();
		
		tankMode.update();
		logger.info("Tank mode " + (tankMode.isEnabled() ? "enabled" : "disabled"));
		
		slowMode.update();
		logger.info("Slow mode " + (slowMode.isEnabled() ? "enabled" : "disabled"));
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
