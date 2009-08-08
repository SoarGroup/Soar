package edu.umich.soar.gridmap2d;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.Map.Entry;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.config.PlayerConfig;
import edu.umich.soar.gridmap2d.config.SimConfig;
import edu.umich.soar.gridmap2d.config.TaxiConfig;
import edu.umich.soar.gridmap2d.map.Cells;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.soar.Soar;
import edu.umich.soar.gridmap2d.world.EatersWorld;
import edu.umich.soar.gridmap2d.world.RoomWorld;
import edu.umich.soar.gridmap2d.world.TankSoarWorld;
import edu.umich.soar.gridmap2d.world.TaxiWorld;
import edu.umich.soar.gridmap2d.world.World;


/**
 * @author voigtjr
 *
 * Keeps track of the meta simulation state. The world keeps track of more state and
 * is the major member of this class. Creates the soar kernel and registers events.
 */
public class Simulation {
	private static Logger logger = Logger.getLogger(Simulation.class);

	public static Random random = new Random();
	public final String kColors[] = { "red", "blue", "yellow", "purple", "orange", "green", "black",  };
	
	private World world;
	private List<String> unusedColors = new ArrayList<String>(kColors.length);
	private Game game;
	private CognitiveArchitecture cogArch;
	private int worldCount;

	World initialize(SimConfig config) {
		this.game = config.game();
		
		// keep track of colors
		for (String color : kColors) {
			unusedColors.add(color);
		}
		
		// Initialize Soar
		cogArch = new Soar(config.soarConfig(), config.clientConfigs(), game, getBasePath());
		Gridmap2D.control.setCogArch(cogArch);
		if (Gridmap2D.wm.using()) {
			Gridmap2D.wm.setCogArch(cogArch);
		}

		// Make all runs non-random if asked
		// For debugging, set this to make all random calls follow the same sequence
		if (config.hasSeed()) {
			// seed the generators
			cogArch.seed(config.generalConfig().seed);
			logger.debug(Names.Debug.seed + config.generalConfig().seed);
			random.setSeed(config.generalConfig().seed);
		} else {
			logger.debug(Names.Debug.noSeed);
		}
		
		// Load the world
		Cells.setUseSynchronized(!config.generalConfig().headless);
		
		// This probably has performance implications, not sure what yet.
		Cells.setUseHashCells(true);
		
		logger.trace(Names.Trace.loadingWorld);
		switch (game) {
		case TANKSOAR:
			world = new TankSoarWorld(Gridmap2D.config.tanksoarConfig().max_missile_packs, cogArch);
			break;
		case EATERS:
			world = new EatersWorld(cogArch);
			break;
		case TAXI:
			TaxiConfig tc = Gridmap2D.config.taxiConfig();
			world = new TaxiWorld(cogArch, tc.fuel_starting_minimum, tc.fuel_starting_maximum, tc.fuel_maximum, tc.disable_fuel);
			break;
		case ROOM:
			world = new RoomWorld(cogArch);
			break;
		}
		world.setForceHumanInput(config.generalConfig().force_human);
		Gridmap2D.control.setRunsTerminal(config.generalConfig().runs);
		Gridmap2D.control.resetTime();
		
		changeMap(config.generalConfig().map);

		cogArch.doBeforeClients();
		cogArch.doAfterClients();
		
		
		// add initial players
		logger.trace(Names.Trace.initialPlayers);
		for ( Entry<String, PlayerConfig> entry : config.playerConfigs().entrySet()) {
			createPlayer(entry.getKey(), entry.getValue(), cogArch.debug());
		}
		
		return world;
	}
	
	public List<String> getUnusedColors() {
		return unusedColors;
	}
	
	public void changeMap(String mapPath) {
		logger.debug(Names.Debug.changingMap + mapPath);
		world.setAndResetMap(mapPath);
		worldCount = 0;
		Gridmap2D.wm.reset();
	}

	/**
	 * @param color the color to use, or null if any will work
	 * @return null if the color is not available for whatever reason
	 * 
	 * removes a color from the unused colors list (by random if necessary)
	 * a return of null indicates failure, the color is taken or no more
	 * are available
	 */
	public String useAColor(String color) {
		if (unusedColors.size() < 1) {
			return null;
		}
		if (color == null) {
			int pick = random.nextInt(unusedColors.size());
			color = unusedColors.get(pick);
			unusedColors.remove(pick);
			return color;
		}
		Iterator<String> iter = unusedColors.iterator();
		while (iter.hasNext()) {
			if (color.equalsIgnoreCase(iter.next())) {
				iter.remove();
				return color;
			}
		}
		return null;
	}
	
	/**
	 * @param color the color to free up, must be not null
	 * @return false if the color wasn't freed up
	 * 
	 * The opposite of useAColor
	 * a color wouldn't be freed up if it wasn't being used in the first place
	 * or if it wasn't legal
	 */
	public boolean freeAColor(String color) {
		assert color != null;
		boolean legal = false;
		for (String knownColor : kColors) {
			if (color.equals(knownColor)) {
				legal = true;
			}
		}
		if (!legal) {
			return false;
		}
		if (unusedColors.contains(color)) {
			return false;
		}
		unusedColors.add(color);
		return true;
	}

	/**
	 * @param playerConfig configuration data for the future player
	 * @throws IllegalStateException If there are no colors available. This indicates that there are too many players already on the map.
	 * 
	 * create a player and add it to the simulation and world
	 */
	public void createPlayer(String playerId, PlayerConfig playerConfig, boolean debug) {
		if (unusedColors.size() < 1) {
			throw new IllegalStateException("no colors available, too many players already on map or colors not released" +
					"");
		}

		playerConfig.color = useAColor(playerConfig.color);
		assert playerConfig.color != null;

		// if we don't have a name, use our color
		if (playerConfig.name == null) {
			playerConfig.name = playerConfig.color;
		}

		// verify name is available
		if (world.hasPlayer(playerConfig.name)) {
			freeAColor(playerConfig.color);
			return;
		}
			
			
		if (!world.addPlayer(playerId, playerConfig, debug)) {
			freeAColor(playerConfig.color);
			return;
		}
		
		// the agent list has changed, notify things that care
		Gridmap2D.control.playerEvent();
	}
	
	/**
	 * @param player the player to remove
	 * 
	 * removes the player from the world and blows away any associated data, 
	 * frees up its color, etc.
	 */
	public void destroyPlayer(Player player) {
		world.removePlayer(player.getName());
		
		// free its color
		freeAColor(player.getColor());
		
		cogArch.destroyPlayer(player.getName());
		
		// the player list has changed, notify those who care
		Gridmap2D.control.playerEvent();
	}
	
	/**
	 * @param player the player to reload
	 * 
	 * reload the player. only currently makes sense to reload a soar agent.
	 * this re-loads the productions
	 */
	public void reloadPlayer(Player player) {
		cogArch.reload(player.getName());
	}
	
	/**
	 * update the sim, or, in this case, the world
	 */
	public void update() {
		worldCount += 1;
		world.update(worldCount);
	}
	
	public int getWorldCount() {
		return worldCount;
	}

	public void reset() {
		logger.info(Names.Info.reset);
		world.reset();
		worldCount = 0;
		Gridmap2D.control.resetTime();
	}

	public void shutdown() {
		for(Player player : world.getPlayers()) {
			destroyPlayer(player);
		}

		cogArch.shutdown();
	}
	
	public boolean isDone() {
		return world.isTerminal();
	}

	public String getBasePath() {
		return System.getProperty("user.dir") + System.getProperty("file.separator");
	}
	
	public String getMapPath() {
		return Gridmap2D.simulation.getBasePath() + "config" + System.getProperty("file.separator") +  "maps" + System.getProperty("file.separator") + game.id();
	}
	
	public void interrupted(String agentName) {
		world.interrupted(agentName);
	}

	public String getCurrentMapName() {
		return world.getMap().getCurrentMapName();
	}
}
