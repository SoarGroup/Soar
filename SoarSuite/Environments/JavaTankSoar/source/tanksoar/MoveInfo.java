package tanksoar;

public class MoveInfo {
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
}

