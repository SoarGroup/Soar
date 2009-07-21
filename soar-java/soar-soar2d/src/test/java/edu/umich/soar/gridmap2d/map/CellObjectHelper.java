package edu.umich.soar.gridmap2d.map;

import edu.umich.soar.gridmap2d.config.Config;
import edu.umich.soar.gridmap2d.config.ConfigFile;

public class CellObjectHelper {
	static Config createNewConfig(String objectName) {
		ConfigFile cf = new ConfigFile();
		Config config = cf.getConfig();
		config.setString("name", objectName);
		return config;
	}
}
