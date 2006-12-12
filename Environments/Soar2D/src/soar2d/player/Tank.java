package soar2d.player;

import soar2d.Soar2D;
import soar2d.World;

public class Tank extends Player {
	public Tank( PlayerConfig playerConfig) {
		super(playerConfig);
	}
	
	public void update(World world, java.awt.Point location) {
		moved = (location.x != this.previousLocation.x) || (location.y != this.previousLocation.y);
	}
	
	public MoveInfo getMove() {
		if (Soar2D.config.graphical == false) {
			return new MoveInfo();
		}
		MoveInfo move = Soar2D.wm.getHumanMove(this.getColor());
		return move;
	}
	public void shutdown() {
		
	}
}
