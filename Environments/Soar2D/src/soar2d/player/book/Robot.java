package soar2d.player.book;

import soar2d.*;
import soar2d.map.BookMap;
import soar2d.map.CellObject;
import soar2d.player.MoveInfo;
import soar2d.player.Player;
import soar2d.player.PlayerConfig;
import soar2d.world.World;

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

	int inId = -1;
	String inType = null;
	
	public void update(java.awt.Point location) {
		World world = Soar2D.simulation.world;
		BookMap map = (BookMap)world.getMap();
		CellObject obj = map.getInObject(location);
		
		inId = obj.getIntProperty(Names.kPropertyNumber);
		inType = new String(obj.getProperty(Names.kPropertyID));
		
		//System.out.println("Type: " + inType + ", ID: " + inId);
	}
}
