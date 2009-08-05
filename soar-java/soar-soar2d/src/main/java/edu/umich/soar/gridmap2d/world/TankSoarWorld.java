package edu.umich.soar.gridmap2d.world;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.CognitiveArchitecture;
import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.Simulation;
import edu.umich.soar.gridmap2d.config.PlayerConfig;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.GridMap;
import edu.umich.soar.gridmap2d.map.TankSoarMap;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.Tank;
import edu.umich.soar.gridmap2d.players.TankCommander;
import edu.umich.soar.gridmap2d.players.TankState;


public class TankSoarWorld implements World {
	private static Logger logger = Logger.getLogger(TankSoarWorld.class);

	private TankSoarMap map;
	private PlayersManager<Tank> players = new PlayersManager<Tank>();
	private int maxMissilePacks;
	private List<String> stopMessages = new ArrayList<String>();
	private CognitiveArchitecture cogArch;
	private boolean forceHuman = false;

	public TankSoarWorld(int maxMissilePacks, CognitiveArchitecture cogArch) {
		this.maxMissilePacks = maxMissilePacks;
		this.cogArch = cogArch;
	}
	
	@Override
	public void setAndResetMap(String mapPath) {
		TankSoarMap newMap = TankSoarMap.generateInstance(mapPath, Gridmap2D.config.tanksoarConfig().max_sound_distance);
		if (newMap == null) {
			return;
		}
		map = newMap;
		resetState();
	}
	
	private void resetState() {
		if (!map.hasEnergyCharger()) {
			addCharger(false);
		}
		if (!map.hasHealthCharger()) {
			addCharger(true);
		}
		
		missileID = 0;
		missileReset = 0;

		// Spawn missile packs
		while (map.numberMissilePacks() < Gridmap2D.config.tanksoarConfig().max_missile_packs) {
			spawnMissilePack(map, true);
		}
		stopMessages.clear();
		resetPlayers();
	}
	
	public void reset() {
		map.reset();
		resetState();
	}

	/**
	 * @throws IllegalStateException If there are no available locations for players to spawn
	 */
	private void resetPlayers() {
		if (players.numberOfPlayers() == 0) {
			return;
		}
		
		for (Tank tank : players.getAll()) {
			// find a suitable starting location
			int [] location = WorldUtil.getStartingLocation(map, players.getInitialLocation(tank));
			if (location == null) {
				throw new IllegalStateException("no empty locations available for spawn");
			}
			players.setLocation(tank, location);
			
			// put the player in it
			map.getCell(location).setPlayer(tank);

			tank.reset();
		}
		
		updatePlayers(true);
	}
	
	private void updatePlayers(boolean playersChanged) {
		for (Tank tank : players.getAll()) {
			TankState state = tank.getState();
			
			if (playersChanged) {
				tank.playersChanged(players.getAllAsPlayers());
			}
			
			if (players.numberOfPlayers() < 2) {
				state.setSmellDistance(0);
				state.setSmellColor(null);
				state.setSound(Direction.NONE);
			} else {
				int distance = 99;
				String color = null;
				
				for (Tank otherTank : players.getAll()) {
					
					if (otherTank.equals(tank)) {
						continue;
					}
					
					int [] playerLoc = players.getLocation(tank);
					int [] otherLoc = players.getLocation(otherTank);
					int newDistance = Math.abs(playerLoc[0] - otherLoc[0]) + Math.abs(playerLoc[1] - otherLoc[1]);
					
					if (newDistance < distance) {
						distance = newDistance;
						color = otherTank.getColor();
					} else if (newDistance == distance) {
						if (Simulation.random.nextBoolean()) {
							// BUGBUG: note that this is not random, later ones are biased
							// not determined important
							distance = newDistance;
							color = otherTank.getColor();
						}
					}
				}
				state.setSmellDistance(distance);
				state.setSmellColor(color);

				if (distance > Gridmap2D.config.tanksoarConfig().max_sound_distance) {
					if (logger.isTraceEnabled()) {
						logger.trace("Skipping sound check, smell was " + distance);
					}
					state.setSound(Direction.NONE);
				} else {
					state.setSound(map.getSoundNear(tank, players));
				}
			}
				
			tank.update(players.getLocation(tank), map);
		}
		
		for (Tank tank : players.getAll()) {
			tank.commit(players.getLocation(tank));
		}
		for (Tank tank : players.getAll()) {
			tank.resetPointsChanged();
		}
	}

	public void setExplosion(int [] xy) {
		CellObject explosion = map.createObjectByName(Names.kExplosion);
		map.getCell(xy).addObject(explosion);
	}

	private Map<Tank, Set<Tank> > killedTanks = new HashMap<Tank, Set<Tank> >(7);
	private int missileID = 0;
	private int missileReset = 0;

	public void update(int worldCount) {
		WorldUtil.checkNumPlayers(players.numberOfPlayers());

		// Reset sensors, collect input
		for (Tank tank : players.getAll()) {
			tank.getState().resetSensors(); // TODO: can this go somewhere else?
			
			CommandInfo command = forceHuman ? Gridmap2D.control.getHumanCommand(tank) : tank.getCommand();
			if (command == null) {
				Gridmap2D.control.stopSimulation();
				return;
			}
			players.setCommand(tank, command);
			WorldUtil.checkStopSim(stopMessages, command, tank);
		}

		// We'll cache the tank new locations
		Map<Tank, int []> newLocations = new HashMap<Tank, int []>();
		
		// And we'll cache tanks that moved
		List<Tank> movedTanks = new ArrayList<Tank>(players.numberOfPlayers());
		
		// And we'll cache tanks that fired
		List<Tank> firedTanks = new ArrayList<Tank>(players.numberOfPlayers());
		
		// We need to keep track of killed tanks, reset the list
		killedTanks.clear();
		
		// Cache players who fire (never fails)
		// Rotate players (never fails)
		// Update shields & consume shield energy
		// Update radar & consume radar energy
		// Do cross checks (and only cross checks) first
		// Cross-check:
		// If moving in to a cell with a tank, check that tank for 
		// a move in the opposite direction
		for (Tank tank : players.getAll()) {
			TankState state = tank.getState();
			
			CommandInfo playerMove = players.getCommand(tank);
			
			// Check for fire
			if (playerMove.fire) {
				if (state.getMissiles() > 0) {
					state.adjustMissiles(-1, "fire");
					firedTanks.add(tank);
				} else {
					logger.debug(tank + ": fired with no ammo");
				}
			}
			
			int [] oldLocation = players.getLocation(tank);
			
			// Check for rotate
			if (playerMove.rotate) {
				map.getCell(oldLocation).forceRedraw();
				Direction facing = tank.getFacing();
				if (playerMove.rotateDirection.equals(Names.kRotateLeft)) {
					facing = facing.left();
				} else if (playerMove.rotateDirection.equals(Names.kRotateRight)) {
					facing = facing.right();
				} else {
					logger.warn(tank + ": unknown rotation command: " + playerMove.rotateDirection);
				}
				tank.setFacing(facing);
			}
			
			// Check shields
			if (playerMove.shields) {
				state.setShieldsUp(playerMove.shieldsSetting);
			}
			
			// Radar
			if (playerMove.radar) {
				state.setRadarSwitch(playerMove.radarSwitch);
			}
			if (playerMove.radarPower) {
				state.setRadarPower(playerMove.radarPowerSetting);
			}

			// if we exist in the new locations, we can skip ourselves
			if (newLocations.containsKey(tank)) {
				continue;
			}
			
			// we haven't been checked yet
			
			// Calculate new location if I moved, or just use the old location
			if (!playerMove.move) {
				// No move, cross collision impossible
				newLocations.put(tank, oldLocation);
				continue;
			}
			
			// we moved, calcuate new location
			int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);
			Direction.translate(newLocation, playerMove.moveDirection);
			
			//Cell dest = map.getCell(newLocation);
			
			// Check for wall collision
			if (map.getCell(newLocation).hasAnyWithProperty(Names.kPropertyBlock)) {
				// Moving in to wall, there will be no player in that cell

				// Cancel the move
				playerMove.move = false;
				newLocations.put(tank, players.getLocation(tank));
				
				// take damage
				String name = map.getCell(newLocation).getAllWithProperty(Names.kPropertyBlock).get(0).getName();
				state.adjustHealth(Gridmap2D.config.tanksoarConfig().collision_penalty, name);
				
				if (state.getHealth() <= 0) {
					Set<Tank> assailants = killedTanks.get(tank);
					if (assailants == null) {
						assailants = new HashSet<Tank>();
					}
					assailants.add(tank);
					killedTanks.put(tank, assailants);
				}
				continue;
			}
			
			// The cell is enterable, check for player
			
			Tank other = (Tank)map.getCell(newLocation).getPlayer();
			if (other == null) {
				// No tank, cross collision impossible
				newLocations.put(tank, newLocation);
				movedTanks.add(tank);
				continue;
			}
			
			// There is another player, check its move
			
			CommandInfo otherMove = players.getCommand(other);
			if (!otherMove.move) {
				// they didn't move, cross collision impossible
				newLocations.put(tank, newLocation);
				movedTanks.add(tank);
				continue;
			}
			
			// the other player is moving, check its direction
			
			if (playerMove.moveDirection != otherMove.moveDirection.backward()) {
				// we moved but not toward each other, cross collision impossible
				newLocations.put(tank, newLocation);
				movedTanks.add(tank);
				continue;
			}

			// Cross collision detected
			
			// take damage
			
			state.adjustHealth(Gridmap2D.config.tanksoarConfig().collision_penalty, "cross collision " + other);
			// Getting rammed on a charger is deadly
			if (map.getCell(players.getLocation(tank)).hasAnyWithProperty(Names.kPropertyCharger)) {
				state.adjustHealth(state.getHealth() * -1, "hit on charger");
			}
			
			TankState otherState = other.getState();
			otherState.adjustHealth(Gridmap2D.config.tanksoarConfig().collision_penalty, "cross collision " + tank);
			// Getting rammed on a charger is deadly
			if (map.getCell(players.getLocation(other)).hasAnyWithProperty(Names.kPropertyCharger)) {
				otherState.adjustHealth(otherState.getHealth() * -1, "hit on charger");
			}
			
			
			if (state.getHealth() <= 0) {
				Set<Tank> assailants = killedTanks.get(tank);
				if (assailants == null) {
					assailants = new HashSet<Tank>();
				}
				assailants.add(other);
				killedTanks.put(tank, assailants);
			}
			if (otherState.getHealth() <= 0) {
				Set<Tank> assailants = killedTanks.get(other);
				if (assailants == null) {
					assailants = new HashSet<Tank>();
				}
				assailants.add(tank);
				killedTanks.put(other, assailants);
			}
			
			// cancel moves
			playerMove.move = false;
			otherMove.move = false;
			
			// store new locations
			newLocations.put(tank, players.getLocation(tank));
			newLocations.put(other, players.getLocation(other));
		}
		
		// We've eliminated all cross collisions and walls
		
		// We'll need to save where people move, indexed by location
		Map<Integer, List<Tank> > collisionMap = new HashMap<Integer, List<Tank> >();
		
		// Iterate through players, checking for all other types of collisions
		// Also, moves are committed at this point and they won't respawn on
		// a charger, so do charging here too
		// and shields and radar
		for (Tank tank : players.getAll()) {
			TankState state = tank.getState();
			
			doMoveCollisions(tank, newLocations, collisionMap, movedTanks);

			// chargers
			chargeUp(tank, newLocations.get(tank));

			// Shields
			if (state.getShieldsUp()) {
				if (state.getEnergy() > 0) {
					state.adjustEnergy(Gridmap2D.config.tanksoarConfig().shield_energy_usage, "shields");
				} else {
					logger.debug(tank + ": shields ran out of energy");
					state.setShieldsUp(false);
				}
			}
			
			// radar
			if (state.getRadarSwitch()) {
				handleRadarEnergy(tank);
			}
		}			
		
		// figure out collision damage
		for (List<Tank> collision : collisionMap.values()) {
			
			// if there is more than one player, have them all take damage
			if (collision.size() > 1) {
				
				int damage = collision.size() - 1;
				damage *= Gridmap2D.config.tanksoarConfig().collision_penalty;
				
				logger.debug("Collision, " + (damage * -1) + " damage:");
				
				for (Tank tank : collision) {
					TankState state = tank.getState();

					state.adjustHealth(damage, "collision");
					
					// Getting rammed on a charger is deadly
					if (map.getCell(players.getLocation(tank)).hasAnyWithProperty(Names.kPropertyCharger)) {
						state.adjustHealth(state.getHealth() * -1, "hit on charger");
					}
					
					// check for kill
					if (state.getHealth() <= 0) {
						Set<Tank> assailants = killedTanks.get(tank);
						if (assailants == null) {
							assailants = new HashSet<Tank>();
						}
						// give everyone else involved credit for the kill
						for (Tank other : collision) {
							if (other.equals(tank)) {
								continue;
							}
							assailants.add(other);
						}
						killedTanks.put(tank, assailants);
					}
				}
			}
		}
		
		// Commit tank moves in two steps, remove from old, place in new
		for (Tank tank : movedTanks) {
			// remove from past cell
			map.getCell(players.getLocation(tank)).setPlayer(null);
		}
		
		// commit the new move, grabbing the missile pack if applicable
		for (Tank tank : movedTanks) {
			// put in new cell
			int [] location = newLocations.get(tank);
			players.setLocation(tank, location);
			map.getCell(location).setPlayer(tank);
			
			// get missile pack
			CellObject missilePack = map.getCell(location).getObject("missiles");
			if (missilePack != null) {
				apply(missilePack, tank);
				map.getCell(location).removeObject("missiles");
			}
			
			// is there a missile in the cell?
			List<CellObject> missiles = map.getCell(location).getAllWithProperty(Names.kPropertyMissile);
			if (missiles == null) {
				// No, can't collide
				continue;
			}

			// are any flying toward me?
			CommandInfo move = players.getCommand(tank);
			for (CellObject missile : missiles) {
				if (move.moveDirection == Direction.parse(missile.getProperty(Names.kPropertyDirection)).backward()) {
					missileHit(tank, missile);
					map.getCell(location).removeObject(missile.getName());

					// explosion
					setExplosion(location);
				}
			}
		}
		
		// move missiles to new cells, checking for new victims
		map.updateObjects(this);
		
		// If there is more than one player out there, keep track of how
		// many updates go by before resetting everything to prevent oscillations
		if (players.numberOfPlayers() > 1) {
			missileReset += 1;
		}
		
		if (firedTanks.size() > 0) {
			// at least one player has fired a missile, reset the 
			// missle reset counter to zero
			missileReset = 0;
		}
		
		// Spawn new Missiles in front of Tanks
		for (Tank tank : firedTanks) {
			int [] missileLoc = Arrays.copyOf(players.getLocation(tank), players.getLocation(tank).length);
			
			Direction direction = tank.getFacing();
			Direction.translate(missileLoc, direction);
			
			if (!map.isInBounds(missileLoc)) {
				continue;
			}
			if (map.getCell(missileLoc).hasAnyWithProperty(Names.kPropertyBlock)) {
				// explosion
				setExplosion(missileLoc);
				continue;
			}
			
			CellObject missile = map.createObjectByName(Names.kPropertyMissile);
			missile.setProperty(Names.kPropertyDirection, direction.id());
			missile.setIntProperty(Names.kPropertyFlyPhase, 0);
			missile.setProperty(Names.kPropertyOwner, tank.getName());
			missile.setIntProperty("missile-id", missileID++);
			missile.setProperty(Names.kPropertyColor, tank.getColor());
			
			// If there is a tank there, it is hit
			Tank other = (Tank)map.getCell(missileLoc).getPlayer();
			if (other != null) {
				missileHit(other, missile);
				
				// explosion
				setExplosion(missileLoc);
				
			} else {
				map.getCell(missileLoc).addObject(missile);
			}
		}
		
		// Handle incoming sensors now that all missiles are flying
		map.handleIncoming();
		
		// Spawn missile packs
		if (map.usingMissilePacks() && map.numberMissilePacks() < maxMissilePacks) {
			spawnMissilePack(map, false);
		}
		
		// Respawn killed Tanks in safe squares
		for (Tank tank : killedTanks.keySet()) {
			// apply points
			tank.adjustPoints(Gridmap2D.config.tanksoarConfig().kill_penalty, "fragged");
			assert killedTanks.containsKey(tank);
			for (Tank assailant : killedTanks.get(tank)) {
				if (assailant.equals(tank)) {
					continue;
				}
				assailant.adjustPoints(Gridmap2D.config.tanksoarConfig().kill_award, "fragged " + tank);
			}
			
			frag(tank);
		}
		
		// if the missile reset counter is 100 and there were no killed tanks
		// this turn, reset all tanks
		if ((missileReset >= Gridmap2D.config.tanksoarConfig().missile_reset_threshold) && (killedTanks.size() == 0)) {
			logger.info("missile reset threshold exceeded, resetting all tanks");
			missileReset = 0;
			for (Tank tank : players.getAll()) {
				frag(tank);
			}
		}
		
		// Update tanks
		updatePlayers(false);
		
		WorldUtil.checkMaxUpdates(stopMessages, worldCount);
		WorldUtil.checkWinningScore(stopMessages, players.getSortedScores());
		
		if (stopMessages.size() > 0) {
			Gridmap2D.control.stopSimulation();
			boolean stopping = Gridmap2D.control.getRunsTerminal() <= 0;
			WorldUtil.dumpStats(players.getSortedScores(), players.getAllAsPlayers(), stopping, stopMessages);
		}
	}
	
	/**
	 * @param tank
	 * @throws IllegalStateException If there is not an available location for the player to spawn
	 */
	private void frag(Tank tank) {
		// remove from past cell
		int [] oldLocation = players.getLocation(tank);
		setExplosion(oldLocation);
		map.getCell(oldLocation).setPlayer(null);

		// put the player somewhere
		int [] location = WorldUtil.getStartingLocation(map, null);
		if (location == null) {
			throw new IllegalStateException("no empty locations available for spawn");
		}
		players.setLocation(tank, location);
		
		map.getCell(location).setPlayer(tank);
		
		// save the location
		players.setLocation(tank, location);
	
		// reset the player state
		tank.fragged();

		printSpawnMessage(tank, location);
	}
	
	private void printSpawnMessage(Tank tank, int [] location) {
		logger.info(tank.getName() + ": Spawning at (" + 
				location[0] + "," + location[1] + "), facing " + tank.getFacing().id());
	}
	
	private boolean apply(CellObject object, Tank tank) {
		TankState state = tank.getState();

		object.applyProperties();

		if (object.hasProperty("apply.missiles")) {
			int missiles = object.getIntProperty("apply.missiles", 0);
			state.adjustMissiles(missiles, object.getName());
		}
		
		if (object.hasProperty("apply.health")) {
			if (!object.getBooleanProperty("apply.health.shields-down", false) || !state.getShieldsUp()) {
				int health = object.getIntProperty("apply.health", 0);
				state.adjustHealth(health, object.getName());
			}
		}
		
		if (object.hasProperty("apply.energy")) {
			if (!object.getBooleanProperty("apply.energy.shields", false) || state.getShieldsUp()) {
				int energy = object.getIntProperty("apply.energy", 0);
				state.adjustEnergy(energy, object.getName());
			}
		}

		return object.getBooleanProperty("apply.remove", false);
	}
			
	private void handleRadarEnergy(Tank tank) {
		TankState state = tank.getState();
		int available = state.getEnergy();
		if (available < state.getRadarPower()) {
			if (available > 0) {
				logger.debug(tank.getName() + ": reducing radar power due to energy shortage");
				state.setRadarPower(available);
			} else {
				logger.debug(tank.getName() + ": radar switched off due to energy shortage");
				state.setRadarSwitch(false);
				return;
			}
		}
		state.adjustEnergy(state.getRadarPower() * -1, "radar");
	}
	
	private void chargeUp(Tank tank, int [] location) {
		TankState state = tank.getState();
		// Charge up
		List<CellObject> chargers = map.getCell(location).getAllWithProperty(Names.kPropertyCharger);
		
		if (chargers != null) {
			for (CellObject charger : chargers) {
				if (charger.hasProperty(Names.kPropertyHealth)) {
					state.setOnHealthCharger(true);
					if (state.getHealth() < Gridmap2D.config.tanksoarConfig().max_health) {
						state.adjustHealth(charger.getIntProperty(Names.kPropertyHealth, 0), "charger");
					}
				}
				if (charger.hasProperty(Names.kPropertyEnergy)) {
					state.setOnEnergyCharger(true);
					if (state.getEnergy() < Gridmap2D.config.tanksoarConfig().max_energy) {
						state.adjustEnergy(charger.getIntProperty(Names.kPropertyEnergy, 0), "charger");
					}
				}
			}
		}
	}
	
	private void addCharger(boolean health) {
		int [] location = map.getAvailableLocationAmortized();
		if (location == null) {
			Gridmap2D.control.errorPopUp("no available location for charger spawn");
			return;
		}

		if (health) {
			logger.info("spawning health charger at (" + location[0] + "," + location[1] + ")");
			CellObject charger = map.createRandomObjectWithProperties(Names.kPropertyHealth, Names.kPropertyCharger);
			if (charger == null) {
				logger.info("no available health chargers");
				return;
			}
			map.getCell(location).addObject(charger);
		} else {			
			logger.info("spawning energy charger at (" + location[0] + "," + location[1] + ")");
			CellObject charger = map.createRandomObjectWithProperties(Names.kPropertyEnergy, Names.kPropertyCharger);
			if (charger == null) {
				logger.info("no available energy chargers");
				return;
			}
			map.getCell(location).addObject(charger);
		}
	}
	
	private void spawnMissilePack(TankSoarMap theMap, boolean force) {
		if (force || (Simulation.random.nextInt(100) < Gridmap2D.config.tanksoarConfig().missile_pack_respawn_chance)) {
			// I used to call getAvailableLocations but that is slow. Brute force find a spot. Time out in case of crazyness.
			int [] spot = theMap.getAvailableLocationAmortized();
			if (spot == null) {
				logger.error("no available location for missile pack spawn");
				return;
			}
			
			// Add a missile pack to a spot
			logger.info("spawning missile pack at (" + spot[0] + "," + spot[1] + ")");
			CellObject missiles = theMap.createObjectByName("missiles");
			assert missiles != null;
			map.getCell(spot).addObject(missiles);
		}
	}
	
	public void missileHit(Tank tank, CellObject missile) {
		TankState state = tank.getState();
		// Yes, I'm hit
		apply(missile, tank);
		
		// apply points
		String owner = missile.getProperty(Names.kPropertyOwner);
		tank.adjustPoints(Gridmap2D.config.tanksoarConfig().missile_hit_penalty, owner + "-" + missile.getProperty("missile-id"));
		Tank other = players.get(owner);
		// can be null if the player was deleted after he fired but before the missile hit
		if (other != null) {
			other.adjustPoints(Gridmap2D.config.tanksoarConfig().missile_hit_award, owner + "-" + missile.getProperty("missile-id"));
		}
		
		// charger insta-kill
		if (map.getCell(players.getLocation(tank)).hasAnyWithProperty(Names.kPropertyCharger)) {
			state.adjustHealth(state.getHealth() * -1, "hit on charger");
		}
		
		// check for kill
		if (state.getHealth() <= 0) {
			Set<Tank> assailants = killedTanks.get(tank);
			if (assailants == null) {
				assailants = new HashSet<Tank>();
			}
			if (other != null) {
				assailants.add(other);
			}
			killedTanks.put(tank, assailants);
		}
	}
	
	private void doMoveCollisions(Tank player, 
			Map<Tank, int []> newLocations, 
			Map<Integer, List<Tank> > collisionMap, 
			List<Tank> movedTanks) {
		
		// Get destination location
		int [] newLocation = newLocations.get(player);
		
		// Wall collisions checked for earlier
		
		// is there a collision in the cell
		List<Tank> collision = collisionMap.get(Arrays.hashCode(newLocation));
		if (collision != null) {
			
			// there is a collision

			// if there is only one player here, cancel its move
			if (collision.size() == 1) {
				Tank other = collision.get(0);
				if (players.getCommand(other).move) {
					cancelMove(other, newLocations, movedTanks);
					doMoveCollisions(other, newLocations, collisionMap, movedTanks);
				}
			} 

			// If there is more than one guy here, they've already been cancelled
			
			
			// Add ourselves to this cell's collision list
			collision.add(player);
			collisionMap.put(Arrays.hashCode(newLocation), collision);
			
			// cancel my move
			if (players.getCommand(player).move) {
				cancelMove(player, newLocations, movedTanks);
				doMoveCollisions(player, newLocations, collisionMap, movedTanks);
			}
			return;

		}

		// There is nothing in this cell, create a new list and add ourselves
		collision = new ArrayList<Tank>(4);
		collision.add(player);
		collisionMap.put(Arrays.hashCode(newLocation), collision);
	}
	
	private void cancelMove(Tank tank, Map<Tank, int []> newLocations, 
			List<Tank> movedTanks) {
		CommandInfo move = players.getCommand(tank);
		move.move = false;
		movedTanks.remove(tank);
		newLocations.put(tank, players.getLocation(tank));
	}
	
	@Override
	public boolean hasPlayer(String name) {
		return players.get(name) != null;
	}
	
	@Override
	public boolean addPlayer(String id, PlayerConfig cfg, boolean debug) {
		int [] location = WorldUtil.getStartingLocation(map, cfg.pos);
		if (location == null) {
			Gridmap2D.control.errorPopUp("There are no suitable starting locations.");
			return false;
		}

		Tank.Builder builder = new Tank.Builder(id);
		Tank player = builder.missiles(cfg.missiles).energy(cfg.energy).health(cfg.health).build();
		players.add(player, cfg.pos);
		
		if (cfg.productions != null) {
			TankCommander cmdr = cogArch.createTankCommander(player, cfg.productions, cfg.shutdown_commands, map.getMetadataFile(), debug);
			if (cmdr == null) {
				players.remove(player);
				return false;
			}
			player.setCommander(cmdr);
		}
	
		players.setLocation(player, location);
	
		// remove food from it
		map.getCell(location).removeAllByProperty(Names.kPropertyEdible);
	
		// put the player in it
		map.getCell(location).setPlayer(player);
		
		logger.info(player.getName() + ": Spawning at (" + location[0] + "," + location[1] + ")");
		
		updatePlayers(true);
		return true;
	}

	public GridMap getMap() {
		return map;
	}

	public Player[] getPlayers() {
		return players.getAllAsPlayers();
	}

	public void interrupted(String agentName) {
		players.interrupted(agentName);
		stopMessages.add("interrupted");
	}

	public boolean isTerminal() {
		return stopMessages.size() > 0;
	}

	public int numberOfPlayers() {
		return players.numberOfPlayers();
	}

	public void removePlayer(String name) {
		Tank tank = players.get(name);
		map.getCell(players.getLocation(tank)).setPlayer(null);
		players.remove(tank);
		tank.shutdownCommander();
		updatePlayers(true);
	}

	public void setForceHumanInput(boolean setting) {
		forceHuman = setting;
	}
}
