package edu.umich.soar.gridmap2d.config;

import edu.umich.soar.gridmap2d.Game;

public class SoarConfig {
	public int max_memory_usage = -1;
	public int port = 12121;
	public String remote = null;
	public boolean spawn_debuggers = true;
	public String metadata = null;
	public boolean soar_print = false;
	
	public boolean runTilOutput(Game game) {
		if (game.equals(Game.TANKSOAR)) {
			return true;
		}
		return false;
	}
}
