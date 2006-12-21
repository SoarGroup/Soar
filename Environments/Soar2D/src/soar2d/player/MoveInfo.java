package soar2d.player;

import soar2d.*;

/**
 * @author voigtjr
 *
 * output required for interaction with the world
 */
public class MoveInfo {
	/**
	 * true indicates a directional move has been made
	 */
	public boolean move = false;
	/**
	 * must be valid if move is true, indicates direction to move
	 */
	public int moveDirection = -1;
	/**
	 * eaters: indicates if a jump is made, move must be true if this is true,
	 * and by implication, moveDirection must be valid
	 */
	public boolean jump = false;
	/**
	 * eaters: eat the food encountered during this update. does not eat food
	 * on the current cell unless we don't actually move
	 */
	public boolean dontEat = false;
	/**
	 * stop the simulation by command
	 */
	public boolean stopSim = false;
	/**
	 * open the box on the current cell
	 */
	public boolean open = false;
	/**
	 * rotate tank
	 */
	public boolean rotate = false;
	/**
	 * Which way to rotate, must be valid if rotate true
	 */
	public String rotateDirection;
	
	/**
	 * fire the tank cannon
	 */
	public boolean fire = false;
	
	/**
	 * change radar status
	 */
	public boolean radar = false;
	/**
	 * status to change radar to
	 */
	public boolean radarSwitch = false;
	
	/**
	 * change radar power setting
	 */
	public boolean radarPower = false;
	/**
	 * setting to change radar power to
	 */
	public int radarPowerSetting = -1;
	
	/**
	 * change shields status
	 */
	public boolean shields = false;
	/**
	 * setting to change shields to
	 */
	public boolean shieldsSetting = false;
	
	public MoveInfo() {
	}
	
	public String toString() {
		String output = new String();
		if (jump) {
			output += "(" + Names.kJumpID + ": " + Direction.stringOf[moveDirection] + ")";
		} else if (move) {
			output += "(" + Names.kMoveID + ": " + Direction.stringOf[moveDirection] + ")";
		}
		
		if (dontEat) {
			output += "(" + Names.kDontEatID + ")";
		}
		if (open) {
			output += "(" + Names.kOpenID + ")";
		}
		if (stopSim) {
			output += "(" + Names.kStopSimID + ")";
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
		return output;
	}

}
