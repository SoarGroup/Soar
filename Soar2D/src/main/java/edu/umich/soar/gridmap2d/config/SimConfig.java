package edu.umich.soar.gridmap2d.config;

import java.io.IOException;
import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.Map;

import edu.umich.soar.config.Config;
import edu.umich.soar.config.ConfigFile;
import edu.umich.soar.config.ParseError;
import edu.umich.soar.gridmap2d.Game;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;


public class SimConfig implements GameConfig {	
	/**
	 * @param path
	 * @return
	 * 
	 * @throws ParseError If there is an error parsing the config file specified by path
	 * @throws IOException If there is an error finding or reading the config file specified by path
	 * @throws IllegalArgumentException If an unknown game type is passed.
	 */
	public static SimConfig newInstance(String path) throws ParseError, IOException {
		return new SimConfig(new Config(new ConfigFile(path)));
	}
	
	private static class Keys {
		private static final String last_productions = "last_productions";
		private static final String window_position_x = "window_position.x";
		private static final String window_position_y = "window_position.y";
		private static final String general = "general";
		private static final String soar = "soar";
		private static final String terminals = "terminals";
		private static final String players = "players";
		private static final String active_players = players + ".active_players";
		private static final String clients = "clients";
		private static final String active_clients = clients + ".active_clients";
		private static final String points = "points"; // keep in synch with PlayerConfig
	}
	
	private Game game;

	private Config config;
	
	private GeneralConfig generalConfig;
	private SoarConfig soarConfig;
	private TerminalsConfig terminalsConfig;
	
	private GameConfig gameConfig;
	
	private Map<String, PlayerConfig> playerConfigs = new HashMap<String, PlayerConfig>();
	private Map<String, ClientConfig> clientConfigs = new HashMap<String, ClientConfig>();;
	
	/**
	 * @param config
	 * 
	 * @throws IllegalArgumentException If an unknown game type is passed.
	 */
	private SimConfig(Config config) {
		this.config = config;
		
		// verify we have a map
		config.requireString("general.map");
		
		generalConfig = new GeneralConfig();
		loadSubConfig(config.getChild(Keys.general), GeneralConfig.class.getFields(), generalConfig);

		soarConfig = new SoarConfig();
		loadSubConfig(config.getChild(Keys.soar), SoarConfig.class.getFields(), soarConfig);

		terminalsConfig = new TerminalsConfig();
		loadSubConfig(config.getChild(Keys.terminals), TerminalsConfig.class.getFields(), terminalsConfig);

		try {
			game = Game.valueOf(generalConfig.game.toUpperCase());
		} catch (IllegalArgumentException e) {
			e.printStackTrace();
			StringBuilder sb = new StringBuilder();
			sb.append("Unknown game type: ");
			sb.append(generalConfig.game);
			sb.append("\nKnown game types:");
			for (Game gameType : Game.values()) {
				sb.append(" ");
				sb.append(gameType.id());
			}
			Gridmap2D.control.errorPopUp(sb.toString());
			throw new IllegalArgumentException(sb.toString(), e);
		}
		
		Config childConfig = config.getChild(game.id());
		switch (game) {
		case TANKSOAR:
			gameConfig = new TankSoarConfig();
			loadSubConfig(childConfig, TankSoarConfig.class.getFields(), gameConfig);
			break;
		case EATERS:
			gameConfig = new EatersConfig();
			loadSubConfig(childConfig, EatersConfig.class.getFields(), gameConfig);
			break;
		case TAXI:
			gameConfig = new TaxiConfig();
			loadSubConfig(childConfig, TaxiConfig.class.getFields(), gameConfig);
			break;
		}

		if (config.hasKey(Keys.active_players)) {
			for (String playerID : config.getStrings(Keys.active_players)) {
				PlayerConfig playerConfig = new PlayerConfig();
				loadSubConfig(config.getChild(Keys.players + "." + playerID), PlayerConfig.class.getFields(), playerConfig);
				
				// process special has points key
				playerConfig.hasPoints = config.hasKey(Keys.players + "." + playerID + "." + Keys.points);
				
				playerConfigs.put(playerID, playerConfig);
			}
		}

		if (config.hasKey(Keys.active_clients)) {
			for(String clientName : config.getStrings(Keys.active_clients)) {
				ClientConfig clientConfig = new ClientConfig();
				loadSubConfig(config.getChild(Keys.clients + "." + clientName), ClientConfig.class.getFields(), clientConfig);
				clientConfigs.put(clientName, clientConfig);
				
			}
		}
		
		// Add default debugger client to configuration, overwriting any existing java-debugger config:
		ClientConfig clientConfig = new ClientConfig();
		clientConfig.timeout = 15;
		clientConfigs.put(Names.kDebuggerClient, clientConfig);
	}
	
	private void loadSubConfig(Config childConfig, Field [] fields, Object target) {
		// use reflection to load fields
		try {
			for (Field f : fields) {
				if (f.getType().getName() == "boolean") {
					f.set(target, childConfig.getBoolean(f.getName(), f.getBoolean(target)));
					
				} else if (f.getType().getName() == "double") {
					f.set(target, childConfig.getDouble(f.getName(), f.getDouble(target)));
					
				} else if (f.getType().getName() == "int") {
					f.set(target, childConfig.getInt(f.getName(), f.getInt(target)));
					
				} else if (f.getType().getName() == "java.lang.String") {
					f.set(target, childConfig.getString(f.getName(), (String)f.get(target)));
					
				} else 	if (f.getType().getName() == "[Z") {
					f.set(target, childConfig.getBooleans(f.getName(), (boolean [])f.get(target)));
					
				} else if (f.getType().getName() == "[D") {
					f.set(target, childConfig.getDoubles(f.getName(), (double [])f.get(target)));
					
				} else if (f.getType().getName() == "[I") {
					f.set(target, childConfig.getInts(f.getName(), (int [])f.get(target)));
					
				} else if (f.getType().getName() == "[Ljava.lang.String;") {
					f.set(target, childConfig.getStrings(f.getName(), (String [])f.get(target)));
				} else {
					System.out.println("Unsupported type encountered: " + f.getType().getName());
				}
			}
		} catch (IllegalAccessException e) {
			e.printStackTrace();
			// This shouldn't happen as long as all fields are public.
			assert false;
		}
	}
	
	public boolean hasSeed() {
		return config.hasKey("general.seed");
	}
	
	public Game game() {
		return game;
	}
	
	public GeneralConfig generalConfig() {
		return generalConfig;
	}
	
	public SoarConfig soarConfig() {
		return soarConfig;
	}
	
	public TerminalsConfig terminalsConfig() {
		return terminalsConfig;
	}
	
	public EatersConfig eatersConfig() {
		return (EatersConfig)gameConfig;
	}
	
	public TankSoarConfig tanksoarConfig() {
		return (TankSoarConfig)gameConfig;
	}
	
	public TaxiConfig taxiConfig() {
		return (TaxiConfig)gameConfig;
	}
	
	public Map<String, PlayerConfig> playerConfigs() {
		return playerConfigs;
	}
	
	public Map<String, ClientConfig> clientConfigs() {
		return clientConfigs;
	}
	
	public void saveLastProductions(String productionsPath) {
		String game_specific_key = game.id() + "." + Keys.last_productions;
		Gridmap2D.preferences.put(game_specific_key, productionsPath);
	}
	
	public String getLastProductions() {
		String game_specific_key = game.id() + "." + Keys.last_productions;
		return Gridmap2D.preferences.get(game_specific_key, null);
	}
	
	public void saveWindowPosition(int [] xy) {
		Gridmap2D.preferences.putInt(Keys.window_position_x, xy[0]);
		Gridmap2D.preferences.putInt(Keys.window_position_y, xy[1]);
	}
	
	public int [] getWindowPosition() {
		int [] xy = new int[] { 0, 0 };
		xy[0] = Gridmap2D.preferences.getInt(Keys.window_position_x, xy[0]);
		xy[1] = Gridmap2D.preferences.getInt(Keys.window_position_y, xy[1]);
		return xy;
	}
	
	public String title() {
		return gameConfig.title();
	}
}
