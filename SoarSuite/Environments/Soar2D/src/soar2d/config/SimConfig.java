package soar2d.config;

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
	
	private SimConfig(Game game) {
		this.game = game;
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
}
