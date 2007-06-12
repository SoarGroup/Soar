package soar2d.player;

import soar2d.*;
import soar2d.world.CellObject;
import soar2d.world.GridMap;

public class Robot extends Player {
	private MoveInfo move;

	public Robot(PlayerConfig playerConfig) {
		super(playerConfig);
	}
	
	public MoveInfo getMove() {
		return move;
	}

	public boolean getHumanMove() {
		if (Soar2D.config.getNoGUI()) {
			move = new MoveInfo();
			return true;
		}
		
		move = Soar2D.wm.getHumanMove(this);

		if (move == null) {
			return false;
		}
		
		return true;
	}

	public int getLocationId() {
		assert false;
		return -1;
	}
	
	int inId = -1;
	String inType = null;
	
	public void update(World world, java.awt.Point location) {
		GridMap map = world.getMap();
		CellObject obj = map.getInObject(location);
		
		inId = obj.getIntProperty(Names.kPropertyNumber);
		inType = new String(obj.getProperty(Names.kPropertyID));
		
		//System.out.println("Type: " + inType + ", ID: " + inId);
	}
}
