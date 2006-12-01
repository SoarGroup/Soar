package soar2d;

import java.util.logging.*;

public class Eater extends Entity {
	Logger logger = Soar2D.logger;
	
	public Eater(String name, int facingInt, int points, String color) {
		super(name, facingInt, points, color);
	}
	
	public void update(World world, java.awt.Point location) {
		
	}
	
	public MoveInfo getMove() {
		return new MoveInfo();
	}
}
