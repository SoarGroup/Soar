package soar2d.player;

import java.util.logging.*;

import soar2d.Soar2D;
import soar2d.World;

public class Eater extends Player {
	Logger logger = Soar2D.logger;
	
	public Eater(String name, PlayerConfig playerConfig) {
		super(name, playerConfig);
	}
	
	public void update(World world, java.awt.Point location) {
		
	}
	
	public MoveInfo getMove() {
		return new MoveInfo();
	}
}
