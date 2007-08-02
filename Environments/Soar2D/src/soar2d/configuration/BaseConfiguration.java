package soar2d.configuration;

import java.io.File;

import soar2d.Soar2D;

public abstract class BaseConfiguration {

	protected String agentPath;
	protected String mapPath;
	protected File map;
	
	BaseConfiguration() {
		this.mapPath = Soar2D.simulation.getBasePath() + "maps" + System.getProperty("file.separator");
		this.agentPath = Soar2D.simulation.getBasePath() + "agents" + System.getProperty("file.separator");
	}
	
	public String getAgentPath() {
		return new String(agentPath);
	}

	public String getMapPath() {
		return new String(mapPath);
	}

	public File getMap() {
		return map;
	}

	public void setMap(File map) {
		this.map = map;
	}
}
