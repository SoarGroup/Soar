package soar2d.player;

import java.util.logging.*;

import soar2d.Soar2D;
import soar2d.World;

public class Eater extends Player {
	Logger logger = Soar2D.logger;
	
	public Eater( PlayerConfig playerConfig) {
		super(playerConfig);
	}
	
	public void update(World world, java.awt.Point location) {
		
	}
	
	public MoveInfo getMove() {
		if (Soar2D.config.graphical == false) {
			return new MoveInfo();
		}
		MoveInfo move = Soar2D.wm.getHumanEaterMove(this.getColor());
		this.setFacingInt(move.moveDirection);
		return move;
	}
}
