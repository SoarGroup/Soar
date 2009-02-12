package org.msoar.sps.control;

import org.apache.log4j.Logger;

import orc.util.GamePad;

class ModeButton {
	private static Logger logger = Logger.getLogger(ModeButton.class);

	private boolean buttonState = false;
	private boolean modeEnabled = false;
	private String name;
	private GamePad gp;
	private int buttonNumber;
	
	ModeButton(String name, GamePad gp, int buttonNumber) {
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
