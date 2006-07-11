package tanksoar;

import java.util.logging.*;

import simulation.WorldEntity;

public class MoveInfo {
	private static Logger logger = Logger.getLogger("simulation");
	public boolean move;
	public int moveDirection;
	
	public boolean rotate;
	public String rotateDirection;
	
	public boolean fire;
	
	public boolean radar;
	public boolean radarSwitch;
	
	public boolean radarPower;
	public int radarPowerSetting;
	
	public boolean shields;
	public boolean shieldsSetting;
	
	public MoveInfo() {
		reset();
	}
	
	public void reset() {
		move = rotate = fire = radar = radarPower = shields = false;
	}
	
	public String toString() {
		String output = "(";
		if (move) {
			output += "(move: " + WorldEntity.directionToString(moveDirection) + ")";
		}
		if (rotate) {
			output += "(rotate: " + rotateDirection + ")";			
		}
		if (fire) {
			output += "(fire)";
		}
		if (radar) {
			output += "(radar: " + (radarSwitch ? "on" : "off") + ")";
		}
		if (radarPower) {
			output += "(radarPower: " + Integer.toString(radarPowerSetting) + ")";
		}
		if (shields) {
			output += "(shields: " + (shieldsSetting ? "on" : "off") + ")";
		}
		 
		output += ")";
		return output;
	}
}

