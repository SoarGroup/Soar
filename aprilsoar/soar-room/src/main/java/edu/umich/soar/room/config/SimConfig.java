package edu.umich.soar.room.config;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.net.URISyntaxException;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.config.Config;
import edu.umich.soar.config.ConfigFile;
import edu.umich.soar.config.ParseError;
import edu.umich.soar.room.Application;
import edu.umich.soar.room.core.PlayerColor;


public class SimConfig {	
	private static final Log logger = LogFactory.getLog(SimConfig.class);
	
	private static final String ROOM_CNF = "room.cnf";

	private static final File home;
	
	static {
		home = figureOutHome();
		
		try {
			boolean installed = false; 
			installed |= install(ROOM_CNF);
			if (installed) {
				System.err.println("Installed at least one config file." +
						"\nYou may need to refresh the project if you are working inside of Eclipse.");
			}
		} catch (IOException e) {
			e.printStackTrace();
			logger.error("Unable to install config file(s): " + e.getMessage());
		}
	}
	
	private static File figureOutHome() {
		File home = null;
		String homeProperty = System.getProperty("room.home");
		if (homeProperty != null) {
			home = new File(homeProperty);
			return home;
		}

		try {
			home = new File(SimConfig.class.getProtectionDomain().getCodeSource().getLocation().toURI());
			if (!home.isDirectory()) {
				home = home.getParentFile();
			}
		} catch (URISyntaxException e) {
			e.printStackTrace();
			throw new IllegalStateException("Internal error: getCodeSource returned bad URL");
		}

		// if we're running in eclipse, this will be the soar-room/bin folder
		// if we were launched by double-clicking the jar, we'll be in SoarLibrary/lib or soar-room/lib

		// look up one folder
		home = home.getParentFile();
		
		// this should be soar-room
		final String SOARROOM = "soar-room";
		final String SOARLIBRARY = "soar-room";
		final String SOARJAVA = "soar-java";
		if (home.getName().equals(SOARROOM)) {
			return home;
		} else if (home.getName().equals(SOARLIBRARY)) {
			// in SoarLibrary
			// up another and over to soar-java/soar-room
			home = new File(home.getParentFile(), SOARJAVA + File.separator + SOARROOM);
			if (home.exists()) {
				return home;
			}
		}
		
		throw new IllegalStateException("Can't figure out where the " + SOARROOM + " folder is.");
	}
	
	public static File getHome() {
		return home;
	}

	private static boolean install(String file) throws IOException {
		File cnf = new File(file);
		File cnfDest = new File(home + File.separator + "config"
				+ File.separator + file);

		if (cnfDest.exists()) {
			return false;
		}

		// Get the DLL from inside the JAR file
		// It should be placed at the root of the JAR (not in a subfolder)
		String jarpath = "/" + cnf.getPath();
		InputStream is = SimConfig.class.getResourceAsStream(jarpath);

		if (is == null) {
			System.err.println("Failed to find " + jarpath
					+ " in the JAR file");
			return false;
		}

		// Create the new file on disk
		FileOutputStream os = new FileOutputStream(cnfDest);

		// Copy the file onto disk
		byte bytes[] = new byte[2048];
		int read;
		while (true) {
			read = is.read(bytes);

			// EOF
			if (read == -1)
				break;

			os.write(bytes, 0, read);
		}

		is.close();
		os.close();
		return true;
	}

	public static SimConfig newInstance() {
		return new SimConfig(new Config(new ConfigFile()));
	}
	
	/**
	 * @param path
	 * @return
	 * 
	 * @throws IllegalStateException If there is an error loading the config file
	 * @throws FileNotFoundException If the specified path doesn't point to a valid file
	 */
	public static SimConfig getInstance(String path) throws FileNotFoundException {
		// See if it exists:
		File configFile = new File(path);
		if (!configFile.exists()) {
			configFile = new File(home + File.separator + path);
			if (!configFile.exists()) {
				configFile = new File(home + File.separator + "config"
						+ File.separator + path);
				if (!configFile.exists()) {
					throw new FileNotFoundException(path);
				}
			}
		}

		if (!configFile.isFile()) {
			throw new FileNotFoundException(path);
		}

		// Read config file
		SimConfig config = null;
		try {
			config = new SimConfig(new Config(new ConfigFile(path)));
		} catch (IOException e) {
			throw new IllegalStateException("Error loading " + path, e);
		} catch (ParseError e) {
			throw new IllegalStateException("Error loading " + path, e);
		} catch (IllegalArgumentException e) {
			throw new IllegalStateException("Error loading " + path, e);
		}

		return config;
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
		private static final String images = "images";
		private static final String active_images = images + ".active_images";
		private static final String points = "points"; // keep in synch with PlayerConfig
	}
	
	private Config config;
	
	private GeneralConfig generalConfig;
	private SoarConfig soarConfig;
	private TerminalsConfig terminalsConfig;
	
	private Map<String, PlayerConfig> playerConfigs = new HashMap<String, PlayerConfig>();
	private Map<String, File> images = new HashMap<String, File>();
	
	/**
	 * @param config
	 * 
	 * @throws IllegalArgumentException If an unknown game type is passed.
	 */
	private SimConfig(Config config) {
		this.config = config;
		
		generalConfig = new GeneralConfig();
		loadSubConfig(config.getChild(Keys.general), GeneralConfig.class.getFields(), generalConfig);

		soarConfig = new SoarConfig();
		loadSubConfig(config.getChild(Keys.soar), SoarConfig.class.getFields(), soarConfig);

		terminalsConfig = new TerminalsConfig();
		loadSubConfig(config.getChild(Keys.terminals), TerminalsConfig.class.getFields(), terminalsConfig);

		if (config.hasKey(Keys.active_players)) {
			for (String playerID : config.getStrings(Keys.active_players)) {
				PlayerConfig playerConfig = new PlayerConfig();
				loadSubConfig(config.getChild(Keys.players + "." + playerID), PlayerConfig.class.getFields(), playerConfig);
				
				// process special has points key
				playerConfig.hasPoints = config.hasKey(Keys.players + "." + playerID + "." + Keys.points);
				
				playerConfigs.put(playerID, playerConfig);
			}
		}

		if (config.hasKey(Keys.active_images)) {
			for (String imageId : config.getStrings(Keys.active_images)) {
				String fileString = config.getString(Keys.images + "." + imageId);
				if (fileString == null) {
					logger.warn("Unable to load active image key '" + imageId + "', key is not defined in config file.");
					continue;
				}
				
				mapImage(imageId, fileString);
			}
		}
	}
	
	public void mapImage(String imageId, String fileString) {
		File file = new File(fileString);
		if (!file.exists()) {
			file = new File(getHome() + File.separator + fileString);
			if (!file.exists()) {
				logger.warn("Unable to load active image key '" + imageId + "', file does not exist.");
				return;
			}
		}
		
		logger.info(imageId + " -> " + file.getAbsolutePath());
		images.put(imageId, file);
	}
	
	public File getImageFile(String key) {
		return images.get(key);
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
					
				} else if (f.getType().getName() == "edu.umich.soar.room.core.PlayerColor") {
					String colorString = childConfig.getString(f.getName(), null);
					if (colorString != null) {
						f.set(target, PlayerColor.valueOf(colorString.toUpperCase()));
					}
					
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
	
	public GeneralConfig generalConfig() {
		return generalConfig;
	}
	
	public SoarConfig soarConfig() {
		return soarConfig;
	}
	
	public TerminalsConfig terminalsConfig() {
		return terminalsConfig;
	}
	
	public Map<String, PlayerConfig> playerConfigs() {
		return playerConfigs;
	}
	
	public void saveLastProductions(String productionsPath) {
		String game_specific_key = "room" + "." + Keys.last_productions;
		Application.PREFERENCES.put(game_specific_key, productionsPath);
	}
	
	public String getLastProductions() {
		String game_specific_key = "room" + "." + Keys.last_productions;
		return Application.PREFERENCES.get(game_specific_key, null);
	}
	
	public void saveWindowPosition(int [] xy) {
		Application.PREFERENCES.putInt(Keys.window_position_x, xy[0]);
		Application.PREFERENCES.putInt(Keys.window_position_y, xy[1]);
	}
	
	public int [] getWindowPosition() {
		int [] xy = new int[] { 0, 0 };
		xy[0] = Application.PREFERENCES.getInt(Keys.window_position_x, xy[0]);
		xy[1] = Application.PREFERENCES.getInt(Keys.window_position_y, xy[1]);
		return xy;
	}
}
