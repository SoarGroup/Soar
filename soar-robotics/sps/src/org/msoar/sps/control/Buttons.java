package org.msoar.sps.control;

import org.apache.log4j.Logger;

enum Buttons {
	OVERRIDE, SOAR, TANK, SLOW, TAG;
	
	private static final Logger logger = Logger.getLogger(Buttons.class);
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
		boolean temp = modeEnabled;
		modeEnabled = false;
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
			logger.info(name() + " changed to " + (modeEnabled ? "enabled" : "disabled"));
		}
		buttonState = button;
	}
}