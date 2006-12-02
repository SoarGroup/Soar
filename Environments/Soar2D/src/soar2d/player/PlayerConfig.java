package soar2d.player;

import soar2d.*;

public class PlayerConfig {
	public String name;
	public String productions;
	public String color;
	public java.awt.Point initialLocation;
	public int facing = Simulation.random.nextInt(4) + 1;
	// TODO: defaults?
	public int points = 0;
	public int energy = 1000;
	public int health = 1000;
	public int missiles = 14;
}

