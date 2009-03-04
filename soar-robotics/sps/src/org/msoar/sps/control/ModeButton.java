package org.msoar.sps.control;

import org.apache.log4j.Logger;

class ModeButton {
	private static final Logger logger = Logger.getLogger(ModeButton.class);

	private boolean buttonState = false;
	private boolean modeEnabled = false;
	private String name;
	private Gamepad gp;
	private int buttonNumber;
	
	ModeButton(String name, Gamepad gp, int buttonNumber) {
		this.name = name;
		this.gp = gp;
		this.buttonNumber = buttonNumber;
	}
	
	void update() {
		boolean button = gp.getButton(buttonNumber);		
		// change on leading edge
		if (!buttonState && button) {
			modeEnabled = !modeEnabled;
			logger.info(name + " changed to " + (modeEnabled ? "enabled" : "disabled"));
		}
		buttonState = button;
	}
	
	boolean isEnabled() {
		return modeEnabled;
	}
	
	boolean checkAndDisable() {
		boolean temp = modeEnabled;
		modeEnabled = false;
		return temp;
	}
}
