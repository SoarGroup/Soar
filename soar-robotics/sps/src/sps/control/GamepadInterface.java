package sps.control;

import org.apache.log4j.Logger;

import lcmtypes.differential_drive_command_t;
import orc.util.GamePad;

public class GamepadInterface {
	private static Logger logger = Logger.getLogger(Controller.class);

	private GamePad gp = new GamePad();
	private boolean overrideButton = false;
	private boolean override = false;

	private boolean soarControlButton = false;
	private boolean soarControl = false;
	
	private boolean tankModeButton = false;
	private boolean tankMode = false;
	
	private boolean shutdownRequested = false;
	
	public void update() {
		boolean button = gp.getButton(0);		
		// change on trailing edge
		if (overrideButton && !button) {
			override = !override;
			logger.info("Override " + (override ? "enabled" : "disabled"));
		}
		overrideButton = button;
		
		button = gp.getButton(1);		
		// change on trailing edge
		if (soarControlButton && !button) {
			soarControl = !soarControl;
		}
		soarControlButton = button;
		
		button = gp.getButton(2);		
		// change on trailing edge
		if (tankModeButton && !button) {
			tankMode = !tankMode;
			logger.info("Tank mode " + (tankMode ? "enabled" : "disabled"));
		}
		tankModeButton = button;
		
		if (gp.getButton(3)) {
			shutdownRequested = true;
		}
	}
	
	public boolean getOverrideMode() {
		return override;
	}

	public boolean getSoarControlButton() {
		if (soarControl) {
			soarControl = false;
			logger.info("Soar control triggered");
			return true;
		}
		return false;
	}
	
	public boolean getShutdownButton() {
		return shutdownRequested;
	}

	public void getDC(differential_drive_command_t dc) {
		dc.left_enabled = true;
		dc.right_enabled = true;
		
		if (tankMode) {
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
