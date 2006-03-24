package simulation;

import org.eclipse.swt.graphics.Point;

public interface SimulationManager {
	public WorldManager getWorldManager();
	public World getWorld();
	public String getAgentPath();
    public void createEntity(String name, String productions, String color, Point location, String facing);
}
