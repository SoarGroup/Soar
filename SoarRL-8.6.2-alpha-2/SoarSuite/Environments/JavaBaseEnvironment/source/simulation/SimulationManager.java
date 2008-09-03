package simulation;

import utilities.*;

public interface SimulationManager {
	public WorldManager getWorldManager();
	public World getWorld();
	public String getAgentPath();
	// TODO: energy, health, and missiles don't belong in an interface used by eaters. Bad.
    public void createEntity(String name, String productions, String color, MapPoint location, String facing, int energy, int health, int missiles);
    public void setSpawnDebuggers(boolean setting);
    public boolean getSpawnDebuggers();
    public boolean isDebuggerConnected();
}
