package edu.umich.soar.sproom.command;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

enum Buttons {
	OVERRIDE, SOAR, GPMODE, SLOW, TAG;
	
	private static final Log logger = LogFactory.getLog(Buttons.class);
	private static Gamepad gp;
	
	static void setGamepad(Gamepad gp) {
		Buttons.gp = gp;
	}
	static boolean haveGamepad() {
		return Buttons.gp != null;
	}
	
	private boolean buttonState = false;
	private boolean modeEnabled = false;
	
	boolean isEnabled() {
		return modeEnabled;
	}
	
	boolean checkAndDisable() {
		if (modeEnabled == false) {
			return false;
		}
		boolean temp = modeEnabled;
		modeEnabled = false;
		logger.debug(name() + " changed to " + (modeEnabled ? "enabled" : "disabled"));
		return temp;
	}
	
	void update() {
		if (gp == null) {
			return;
		}
		boolean button = gp.getButton(this.ordinal());		
		// change on leading edge
		if (!buttonState && button) {
			modeEnabled = !modeEnabled;
			logger.debug(name() + " changed to " + (modeEnabled ? "enabled" : "disabled"));
		}
		buttonState = button;
	}
}