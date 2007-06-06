package soar2d.player;

import soar2d.*;

/**
 * @author voigtjr
 *
 * output required for interaction with the world
 */
public class MoveInfo {
	
	// all
	public boolean stopSim = false;	// stop the simulation by command
	
	// eaters + tanksoar
	public boolean move = false;	// move
	public int moveDirection = -1;	// direction to move
	
	// eaters
	public boolean open = false;	// open the box on the current cell
	public int openCode = 0;		// the open code to use (only used with reward system)
	public boolean jump = false;	// jump if we move
	public boolean dontEat = false;	// don't eat food
	
	// tanksoar + book
	public boolean rotate = false;	// rotate
	public String rotateDirection;	// Which way to rotate, must be valid if rotate true
	
	// tanksoar
	public boolean fire = false;	// fire the tank cannon
	public boolean radar = false;	// change radar status
	public boolean radarSwitch = false;	// status to change radar to
	public boolean radarPower = false;	// change radar power setting
	public int radarPowerSetting = -1;	// setting to change radar power to
	public boolean shields = false;	// change shields status
	public boolean shieldsSetting = false;	// setting to change shields to
	
	// book
	public boolean forward = false;	// move forward
	public boolean backward = false;	// move backward
	public boolean rotateAbsolute = false;	// rotate to a heading
	public double rotateAbsoluteHeading;	// what heading to stop at
	public boolean rotateRelative = false;	// rotate tank
	public double rotateRelativeAmount;		// how far to rotate
	public boolean get = false;
	public java.awt.Point getLocation = null;
	public int getId;
	public boolean drop = false;
	public int dropId;
	
	public MoveInfo() {
	}
	
	public String toString() {
		String output = new String();
		
		switch(Soar2D.config.getType()) {
		case kEaters:
			if (jump) {
				output += "(" + Names.kJumpID + ")";
			} 
			if (move) {
				output += "(" + Names.kMoveID + ": " + Direction.stringOf[moveDirection] + ")";
			}
			
			if (dontEat) {
				output += "(" + Names.kDontEatID + ")";
			}
			if (open) {
				output += "(" + Names.kOpenID + ")";
				if (openCode != 0) {
					output += "(" + Names.kOpenCodeID + ": "+ Integer.toString(openCode) + ")";
				}
			}
			break;
			
		case kTankSoar:
			if (move) {
				output += "(" + Names.kMoveID + ": " + Direction.stringOf[moveDirection] + ")";
			}
			if (rotate) {
				output += "(" + Names.kRotateID + ": " + rotateDirection + ")";			
			}
			if (fire) {
				output += "(" + Names.kFireID + ")";
			}
			if (radar) {
				output += "(" + Names.kRadarID + ": " + (radarSwitch ? "on" : "off") + ")";
			}
			if (radarPower) {
				output += "(" + Names.kRadarPowerID + ": " + Integer.toString(radarPowerSetting) + ")";
			}
			if (shields) {
				output += "(" + Names.kShieldsID + ": " + (shieldsSetting ? "on" : "off") + ")";
			}
			break;
			
		case kBook:
			if (forward) {
				output += "(" + Names.kForwardID + ")";
			}
			if (backward) {
				output += "(" + Names.kBackwardID + ")";
			}
			if (rotate) {
				output += "(" + Names.kRotateID + ": " + rotateDirection + ")";			
			}
			if (rotateAbsolute) {
				output += "(" + Names.kRotateAbsoluteID + ": " + rotateAbsoluteHeading + ")";			
			}
			if (rotateRelative) {
				output += "(" + Names.kRotateRelativeID + ": " + rotateRelativeAmount + ")";			
			}
			if (get) {
				output += "(" + Names.kGetID + ": " + getId + ": " + getLocation.x + "," + getLocation.y + ")";
			}
			if (drop) {
				output += "(" + Names.kDropID + ": " + dropId + ")";
			}
			break;
		}
		
		if (stopSim) {
			output += "(" + Names.kStopSimID + ")";
		}
		
		return output;
	}

}
