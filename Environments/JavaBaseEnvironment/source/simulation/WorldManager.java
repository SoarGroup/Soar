package simulation;

public interface WorldManager {
	public boolean load(String mapFile);
	public void update();
	public boolean noAgents();
	public void shutdown();
	public WorldEntity[] getEntities();
	public void destroyEntity(WorldEntity entity);
	public void setStopping(boolean status);
}
