package soar2d.config;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Arrays;

import soar2d.Soar2D;

public class SimConfig {
	public enum Game { TANKSOAR, EATERS, KITCHEN, TAXI, ROOM }
	public static SimConfig create(String game) {
		if (game.equalsIgnoreCase("tanksoar")) {
			return new SimConfig(Game.TANKSOAR);
		} else if (game.equalsIgnoreCase("eaters")) {
			return new SimConfig(Game.EATERS);
		} else if (game.equalsIgnoreCase("kitchen")) {
			return new SimConfig(Game.KITCHEN);
		} else if (game.equalsIgnoreCase("taxi")) {
			return new SimConfig(Game.TAXI);
		} else if (game.equalsIgnoreCase("room")) {
			return new SimConfig(Game.ROOM);
		}
		return null;
	}
	
	private Game game;
	private Config preferences;
	private String preferencesPath = "preferences";
	
	private SimConfig(Game game) {
		this.game = game;
		
		try {
			File preferencesFile = new File(preferencesPath);
			if (preferencesFile.exists()) {
				preferences = new Config(new ConfigFile(preferencesPath));
			} else {
				preferences = new Config(new ConfigFile());
			}
		} catch (IOException e) {
			System.err.println("Exception loading preferences: " + e.getMessage());
			preferences = new Config(new ConfigFile());
		}
	}
	
	public Game game() {
		return game;
	}
	public boolean is(Game game) {
		return game == this.game;
	}
	
	public String title() {
		switch (game) {
		case TANKSOAR:
			return "Tanksoar";
		case EATERS:
			return "Eaters";
		case KITCHEN:
			return "Kitchen";
		case TAXI:
			return "Taxi";
		case ROOM:
			return "Room";
		}
		assert false;
		return null;
	}
	
	public float timeSlice() {
		if (game == Game.ROOM) {
			return (float)Soar2D.config.getDouble(Soar2DKeys.room.cycle_time_slice, 50.0);
		}
		return 50.0f;
	}
	
	public boolean runTilOutput() {
		if (game == Game.TANKSOAR) {
			return true;
		}
		return false;
	}
	
	private String getGameSpecificPrefix() {
		String game_specific_prefix = null;
		switch (game) {
		case TANKSOAR:
			game_specific_prefix = "tanksoar";
			break;
		case EATERS:
			game_specific_prefix = "eaters";
			break;
		case KITCHEN:
			game_specific_prefix = "kitchen";
			break;
		case TAXI:
			game_specific_prefix = "taxi";
			break;
		case ROOM:
			game_specific_prefix = "room";
			break;
		}
		return game_specific_prefix + ".";
	}

	public void saveLastProductions(String productionsPath) {
		String game_specific_key = getGameSpecificPrefix() + Soar2DKeys.preferences.last_productions;
		preferences.setString(game_specific_key, productionsPath);
	}
	
	public String getLastProductions() {
		String game_specific_key = getGameSpecificPrefix() + Soar2DKeys.preferences.last_productions;
		if (preferences.hasKey(game_specific_key)) {
			return preferences.getString(game_specific_key);
		}
		return null;
	}
	
	public void saveWindowPosition(int [] xy) {
		//System.out.println("Saving window position: " + Arrays.toString(xy));
		preferences.setInts(Soar2DKeys.preferences.window_position, xy);
	}
	
	public int [] getWindowPosition() {
		if (preferences.hasKey(Soar2DKeys.preferences.window_position)) {
			//System.out.println("Loading window position: " + Arrays.toString(preferences.getInts(Soar2DKeys.preferences.window_position)));
			return preferences.getInts(Soar2DKeys.preferences.window_position);
		}
		return null;
	}
	
	public void savePreferences() {
		// meta preferences that need to be preserved across runs
		try {
			preferences.save(preferencesPath);
		} catch (FileNotFoundException e) {
			System.err.println("Couldn't save preferences: " + e.getMessage());
		}
	}
}
