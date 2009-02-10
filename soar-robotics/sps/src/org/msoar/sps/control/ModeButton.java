package org.msoar.sps.control;

import orc.util.GamePad;

class ModeButton {
	private boolean buttonState = false;
	private boolean modeEnabled = false;
	private GamePad gp;
	int buttonNumber;
	
	ModeButton(GamePad gp, int buttonNumber) {
		this.gp = gp;
		this.buttonNumber = buttonNumber;
	}
	
	void update() {
		boolean button = gp.getButton(buttonNumber);		
		// change on trailing edge
		if (buttonState && !button) {
			modeEnabled = !modeEnabled;
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
