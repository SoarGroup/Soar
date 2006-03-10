package simulation;

public interface SimulationManager {
	public WorldManager getWorldManager();
	public World getWorld();
	public String getAgentPath();
    public void createEntity(String name, String productions, String color);
}
