package org.msoar.gridmap2d.map;

import org.msoar.gridmap2d.config.Config;
import org.msoar.gridmap2d.config.ConfigFile;

public class CellObjectHelper {
	static Config createNewConfig(String objectName) {
		ConfigFile cf = new ConfigFile();
		Config config = cf.getConfig();
		config.setString("name", objectName);
		return config;
	}
}
