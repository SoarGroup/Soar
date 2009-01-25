package soar2d.world;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;

import org.apache.log4j.Logger;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.map.GridMap;
import soar2d.map.TankSoarMap;
import soar2d.players.MoveInfo;
import soar2d.players.Player;

public class TankSoarWorld implements IWorld {
	private static Logger logger = Logger.getLogger(TankSoarWorld.class);

	public boolean postLoad(GridMap _map) {
		TankSoarMap tMap = (TankSoarMap)_map;
		if (!tMap.hasEnergyCharger()) {
			if (!addCharger(false, tMap)) {
				return false;
			}
		}
		if (!tMap.hasHealthCharger()) {
			if (!addCharger(true, tMap)) {
				return false;
			}
		}
		// Spawn missile packs
		while (tMap.numberMissilePacks() < Soar2D.config.tanksoarConfig().max_missile_packs) {
			if (spawnMissilePack(tMap, true) == false) {
				logger.error("Missile pack spawn failed.");
				return false;
			}
		}
		return true;
	}
	
	
	private HashMap<Player, HashSet<Player> > killedTanks = new HashMap<Player, HashSet<Player> >(7);
	private int missileID = 0;
	private int missileReset = 0;

	public String update(GridMap _map, PlayersManager players) {
		TankSoarMap tMap = (TankSoarMap)_map;
		
		// We'll cache the tank new locations
		HashMap<Player, int []> newLocations = new HashMap<Player, int []>();
		
		// And we'll cache tanks that moved
		ArrayList<Player> movedTanks = new ArrayList<Player>(players.numberOfPlayers());
		
		// And we'll cache tanks that fired
		ArrayList<Player> firedTanks = new ArrayList<Player>(players.numberOfPlayers());
		
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
		Iterator<Player> playerIter = players.iterator();
		while (playerIter.hasNext()) {
			Player player = playerIter.next();
			
			MoveInfo playerMove = players.getMove(player);
			
			// Check for fire
			if (playerMove.fire) {
				if (player.getMissiles() > 0) {
					player.adjustMissiles(-1, "fire");
					firedTanks.add(player);
				} else {
					logger.debug(player + ": fired with no ammo");
				}
			}
			
			int [] oldLocation = players.getLocation(player);
			
			// Check for rotate
			if (playerMove.rotate) {
				tMap.forceRedraw(oldLocation);
				Direction facing = player.getFacing();
				if (playerMove.rotateDirection.equals(Names.kRotateLeft)) {
					facing = facing.left();
				} else if (playerMove.rotateDirection.equals(Names.kRotateRight)) {
					facing = facing.right();
				} else {
					logger.warn(player + ": unknown rotation command: " + playerMove.rotateDirection);
				}
				player.setFacing(facing);
			}
			
			// Check shields
			if (playerMove.shields) {
				player.setShields(playerMove.shieldsSetting);
			}
			
			// Radar
			if (playerMove.radar) {
				player.setRadarSwitch(playerMove.radarSwitch);
			}
			if (playerMove.radarPower) {
				player.setRadarPower(playerMove.radarPowerSetting);
			}

			// if we exist in the new locations, we can skip ourselves
			if (newLocations.containsKey(player)) {
				continue;
			}
			
			// we haven't been checked yet
			
			// Calculate new location if I moved, or just use the old location
			if (!playerMove.move) {
				// No move, cross collision impossible
				newLocations.put(player, oldLocation);
				continue;
			}
			
			// we moved, calcuate new location
			int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);
			Direction.translate(newLocation, playerMove.moveDirection);
			
			//Cell dest = map.getCell(newLocation);
			
			// Check for wall collision
			if (tMap.hasAnyWithProperty(newLocation, Names.kPropertyBlock)) {
				// Moving in to wall, there will be no player in that cell

				// Cancel the move
				playerMove.move = false;
				newLocations.put(player, players.getLocation(player));
				
				// take damage
				String name = tMap.getAllWithProperty(newLocation, Names.kPropertyBlock).get(0).getName();
				player.adjustHealth(Soar2D.config.tanksoarConfig().collision_penalty, name);
				
				if (player.getHealth() <= 0) {
					HashSet<Player> assailants = killedTanks.get(player);
					if (assailants == null) {
						assailants = new HashSet<Player>();
					}
					assailants.add(player);
					killedTanks.put(player, assailants);
				}
				continue;
			}
			
			// The cell is enterable, check for player
			
			Player other = tMap.getPlayer(newLocation);
			if (other == null) {
				// No tank, cross collision impossible
				newLocations.put(player, newLocation);
				movedTanks.add(player);
				continue;
			}
			
			// There is another player, check its move
			
			MoveInfo otherMove = players.getMove(other);
			if (!otherMove.move) {
				// they didn't move, cross collision impossible
				newLocations.put(player, newLocation);
				movedTanks.add(player);
				continue;
			}
			
			// the other player is moving, check its direction
			
			if (playerMove.moveDirection != otherMove.moveDirection.backward()) {
				// we moved but not toward each other, cross collision impossible
				newLocations.put(player, newLocation);
				movedTanks.add(player);
				continue;
			}

			// Cross collision detected
			
			// take damage
			
			player.adjustHealth(Soar2D.config.tanksoarConfig().collision_penalty, "cross collision " + other);
			// Getting rammed on a charger is deadly
			if (tMap.hasAnyWithProperty(players.getLocation(player), Names.kPropertyCharger)) {
				player.adjustHealth(player.getHealth() * -1, "hit on charger");
			}
			
			other.adjustHealth(Soar2D.config.tanksoarConfig().collision_penalty, "cross collision " + player);
			// Getting rammed on a charger is deadly
			if (tMap.hasAnyWithProperty(players.getLocation(other), Names.kPropertyCharger)) {
				other.adjustHealth(other.getHealth() * -1, "hit on charger");
			}
			
			
			if (player.getHealth() <= 0) {
				HashSet<Player> assailants = killedTanks.get(player);
				if (assailants == null) {
					assailants = new HashSet<Player>();
				}
				assailants.add(other);
				killedTanks.put(player, assailants);
			}
			if (other.getHealth() <= 0) {
				HashSet<Player> assailants = killedTanks.get(other);
				if (assailants == null) {
					assailants = new HashSet<Player>();
				}
				assailants.add(player);
				killedTanks.put(other, assailants);
			}
			
			// cancel moves
			playerMove.move = false;
			otherMove.move = false;
			
			// store new locations
			newLocations.put(player, players.getLocation(player));
			newLocations.put(other, players.getLocation(other));
		}
		
		// We've eliminated all cross collisions and walls
		
		// We'll need to save where people move, indexed by location
		HashMap<Integer, ArrayList<Player> > collisionMap = new HashMap<Integer, ArrayList<Player> >();
		
		// Iterate through players, checking for all other types of collisions
		// Also, moves are committed at this point and they won't respawn on
		// a charger, so do charging here too
		// and shields and radar
		playerIter = players.iterator();
		while (playerIter.hasNext()) {
			Player player = playerIter.next();

			doMoveCollisions(player, players, newLocations, collisionMap, movedTanks);

			// chargers
			chargeUp(player, tMap, newLocations.get(player));

			// Shields
			if (player.shieldsUp()) {
				if (player.getEnergy() > 0) {
					player.adjustEnergy(Soar2D.config.tanksoarConfig().shield_energy_usage, "shields");
				} else {
					logger.debug(player + ": shields ran out of energy");
					player.setShields(false);
				}
			}
			
			// radar
			if (player.getRadarSwitch()) {
				handleRadarEnergy(player);
			}
		}			
		
		// figure out collision damage
		Iterator<ArrayList<Player> > locIter = collisionMap.values().iterator();
		while (locIter.hasNext()) {
			
			ArrayList<Player> collision = locIter.next();
			
			// if there is more than one player, have them all take damage
			if (collision.size() > 1) {
				
				int damage = collision.size() - 1;
				damage *= Soar2D.config.tanksoarConfig().collision_penalty;
				
				logger.debug("Collision, " + (damage * -1) + " damage:");
				
				playerIter = collision.iterator();
				while (playerIter.hasNext()) {
					Player player = playerIter.next();
					player.adjustHealth(damage, "collision");
					
					// Getting rammed on a charger is deadly
					if (tMap.hasAnyWithProperty(players.getLocation(player), Names.kPropertyCharger)) {
						player.adjustHealth(player.getHealth() * -1, "hit on charger");
					}
					
					// check for kill
					if (player.getHealth() <= 0) {
						HashSet<Player> assailants = killedTanks.get(player);
						if (assailants == null) {
							assailants = new HashSet<Player>();
						}
						// give everyone else involved credit for the kill
						Iterator<Player> killIter = collision.iterator();
						while (killIter.hasNext()) {
							Player other = killIter.next();
							if (other.equals(player)) {
								continue;
							}
							assailants.add(other);
						}
						killedTanks.put(player, assailants);
					}
				}
			}
		}
		
		// Commit tank moves in two steps, remove from old, place in new
		playerIter = movedTanks.iterator();
		while (playerIter.hasNext()) {
			Player player = playerIter.next();
			
			// remove from past cell
			tMap.setPlayer(players.getLocation(player), null);
		}
		
		// commit the new move, grabbing the missile pack if applicable
		playerIter = movedTanks.iterator();
		while (playerIter.hasNext()) {
			Player player = playerIter.next();
			// put in new cell
			int [] location = newLocations.get(player);
			players.setLocation(player, location);
			tMap.setPlayer(location, player);
			
			// get missile pack
			ArrayList<CellObject> missilePacks = tMap.getAllWithProperty(location, Names.kPropertyMissiles);
			if (missilePacks != null) {
				assert missilePacks.size() == 1;
				CellObject pack = missilePacks.get(0);
				pack.apply(player);
				tMap.removeAllByProperty(location, Names.kPropertyMissiles);
			}
			
			
			// is there a missile in the cell?
			ArrayList<CellObject> missiles = tMap.getAllWithProperty(location, Names.kPropertyMissile);
			if (missiles == null) {
				// No, can't collide
				continue;
			}

			// are any flying toward me?
			MoveInfo move = players.getMove(player);
			for (CellObject missile : missiles) {
				if (move.moveDirection == Direction.parse(missile.getProperty(Names.kPropertyDirection)).backward()) {
					missileHit(player, tMap, location, missile, players);
					tMap.removeObject(location, missile.getName());

					// explosion
					tMap.setExplosion(location);
				}
			}
		}
		
		// move missiles to new cells, checking for new victims
		tMap.updateObjects(this);
		
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
		playerIter = firedTanks.iterator();
		while (playerIter.hasNext()) {
			Player player = playerIter.next();
			int [] missileLoc = Arrays.copyOf(players.getLocation(player), players.getLocation(player).length);
			
			Direction direction = player.getFacing();
			Direction.translate(missileLoc, direction);
			
			if (!tMap.isInBounds(missileLoc)) {
				continue;
			}
			if (tMap.hasAnyWithProperty(missileLoc, Names.kPropertyBlock)) {
				// explosion
				tMap.setExplosion(missileLoc);
				continue;
			}
			
			CellObject missile = tMap.createRandomObjectWithProperty(Names.kPropertyMissile);
			missile.setName(player + "-" + missileID++);
			missile.addProperty(Names.kPropertyDirection, direction.id());
			missile.addProperty(Names.kPropertyFlyPhase, "0");
			missile.addProperty(Names.kPropertyOwner, player.getName());
			missile.addProperty(Names.kPropertyColor, player.getColor());
			
			// If there is a tank there, it is hit
			Player other = tMap.getPlayer(missileLoc);
			if (other != null) {
				missileHit(other, tMap, missileLoc, missile, players);
				
				// explosion
				tMap.setExplosion(missileLoc);
				
			} else {
				tMap.addObjectToCell(missileLoc, missile);
			}
		}
		
		// Handle incoming sensors now that all missiles are flying
		tMap.handleIncoming();
		
		// Spawn missile packs
		if (tMap.numberMissilePacks() < Soar2D.config.tanksoarConfig().max_missile_packs) {
			spawnMissilePack(tMap, false);
		}
		
		// Respawn killed Tanks in safe squares
		playerIter = killedTanks.keySet().iterator();
		while (playerIter.hasNext()) {
			
			// apply points
			Player player = playerIter.next();
			
			player.adjustPoints(Soar2D.config.tanksoarConfig().kill_penalty, "fragged");
			assert killedTanks.containsKey(player);
			Iterator<Player> killedPlayerIter = killedTanks.get(player).iterator();
			while (killedPlayerIter.hasNext()) {
				Player assailant = killedPlayerIter.next();
				if (assailant.equals(player)) {
					continue;
				}
				assailant.adjustPoints(Soar2D.config.tanksoarConfig().kill_award, "fragged " + player);
			}
			
			Soar2D.simulation.world.fragPlayer(player);
		}
		
		// if the missile reset counter is 100 and there were no killed tanks
		// this turn, reset all tanks
		if ((missileReset >= Soar2D.config.tanksoarConfig().missile_reset_threshold) && (killedTanks.size() == 0)) {
			logger.info("missile reset threshold exceeded, resetting all tanks");
			missileReset = 0;
			playerIter = players.iterator();
			while (playerIter.hasNext()) {
				Player player = playerIter.next();
				
				Soar2D.simulation.world.fragPlayer(player);
			}
		}
		
		// Update tanks
		updatePlayers(false, tMap, players);

		// no reset after update
		return null;
	}
	
	private void handleRadarEnergy(Player player) {
		int available = player.getEnergy();
		if (available < player.getRadarPower()) {
			if (available > 0) {
				logger.debug(player.getName() + ": reducing radar power due to energy shortage");
				player.setRadarPower(available);
			} else {
				logger.debug(player.getName() + ": radar switched off due to energy shortage");
				player.setRadarSwitch(false);
				return;
			}
		}
		player.adjustEnergy(player.getRadarPower() * -1, "radar");
	}
	
	public void fragPlayer(Player player, GridMap map, PlayersManager players, int [] location) {
		
	}
	
	public void putInStartingLocation(Player player, GridMap map, PlayersManager players, int [] location) {
		
	}
	
	public void reset(GridMap map) {
		missileID = 0;
		missileReset = 0;
	}
	
	private void chargeUp(Player player, TankSoarMap map, int [] location) {
		// Charge up
		ArrayList<CellObject> chargers = map.getAllWithProperty(location, Names.kPropertyCharger);
		if (chargers != null) {
			for (CellObject charger : chargers) {
				if (charger.hasProperty(Names.kPropertyHealth)) {
					player.setOnHealthCharger(true);
					if (player.getHealth() < Soar2D.config.tanksoarConfig().default_health) {
						player.adjustHealth(charger.getIntProperty(Names.kPropertyHealth), "charger");
					}
				}
				if (charger.hasProperty(Names.kPropertyEnergy)) {
					player.setOnEnergyCharger(true);
					if (player.getEnergy() < Soar2D.config.tanksoarConfig().default_energy) {
						player.adjustEnergy(charger.getIntProperty(Names.kPropertyEnergy), "charger");
					}
				}
			}
		}
	}
	

	
	public void updatePlayers(boolean playersChanged, GridMap map, PlayersManager players) {
		for (Player player : players.getAll()) {
			
			if (playersChanged) {
				player.playersChanged();
			}
			if (players.numberOfPlayers() < 2) {
				player.setSmell(0, null);
				player.setSound(Direction.NONE);
			} else {
				int distance = 99;
				String color = null;
				
				Iterator<Player> smellIter = players.iterator();
				while (smellIter.hasNext()) {
					
					Player other = smellIter.next();
					if (other.equals(player)) {
						continue;
					}
					
					int [] playerLoc = players.getLocation(player);
					int [] otherLoc = players.getLocation(other);
					int newDistance = Math.abs(playerLoc[0] - otherLoc[0]) + Math.abs(playerLoc[1] - otherLoc[1]);
					
					if (newDistance < distance) {
						distance = newDistance;
						color = other.getColor();
					} else if (newDistance == distance) {
						if (Simulation.random.nextBoolean()) {
							// BUGBUG: note that this is not random, later ones are biased
							// not determined important
							distance = newDistance;
							color = other.getColor();
						}
					}
				}
				player.setSmell(distance, color);
				
				if (distance > Soar2D.config.tanksoarConfig().max_sound_distance) {
					if (logger.isTraceEnabled()) {
						logger.trace("Skipping sound check, smell was " + distance);
					}
					player.setSound(Direction.NONE);
				} else {
					player.setSound(map.getSoundNear(players.getLocation(player)));
				}
			}
				
			player.update(players.getLocation(player));
		}
		
		Iterator<Player> playerIter = players.iterator();
		while (playerIter.hasNext()) {
			Player player = playerIter.next();
			player.commit(players.getLocation(player));
		}
		playerIter = players.iterator();
		while (playerIter.hasNext()) {
			Player player = playerIter.next();
			player.resetPointsChanged();
		}
	}
	
	private void error(String message) {
		logger.error(message);
		Soar2D.control.errorPopUp(message);
	}

	private boolean addCharger(boolean health, TankSoarMap newMap) {
		int [] location = newMap.getAvailableLocationAmortized();
		if (location == null) {
			error("No place to put charger!");
			return false;
		}

		if (health) {
			logger.info("spawning health charger at (" + location[0] + "," + location[1] + ")");
			CellObject charger = newMap.createRandomObjectWithProperties(Names.kPropertyHealth, Names.kPropertyCharger);
			if (charger == null) {
				error("Couldn't add charger to map!");
				return false;
			}
			newMap.addObjectToCell(location, charger);
		} else {			
			logger.info("spawning energy charger at (" + location[0] + "," + location[1] + ")");
			CellObject charger = newMap.createRandomObjectWithProperties(Names.kPropertyEnergy, Names.kPropertyCharger);
			if (charger == null) {
				error("Couldn't add charger to map!");
				return false;
			}
			newMap.addObjectToCell(location, charger);
		}
		
		return true;
	}
	
	private boolean spawnMissilePack(TankSoarMap theMap, boolean force) {
		if (force || (Simulation.random.nextInt(100) < Soar2D.config.tanksoarConfig().missile_pack_respawn_chance)) {
			// I used to call getAvailableLocations but that is slow. Brute force find a spot. Time out in case of crazyness.
			int [] spot = theMap.getAvailableLocationAmortized();
			if (spot == null) {
				logger.error("could not find available location!");
				return false;
			}
			
			// Add a missile pack to a spot
			logger.info("spawning missile pack at (" + spot[0] + "," + spot[1] + ")");
			CellObject missiles = theMap.createRandomObjectWithProperty(Names.kPropertyMissiles);
			if (missiles == null) {
				return false;
			}
			theMap.addObjectToCell(spot, missiles);
		}
		return true;
	}
	
	public void missileHit(Player player, TankSoarMap tMap, int [] location, CellObject missile, PlayersManager players) {
		// Yes, I'm hit
		missile.apply(player);
		
		// apply points
		player.adjustPoints(Soar2D.config.tanksoarConfig().missile_hit_penalty, missile.getName());
		Player other = players.get(missile.getProperty(Names.kPropertyOwner));
		// can be null if the player was deleted after he fired but before the missile hit
		if (other != null) {
			other.adjustPoints(Soar2D.config.tanksoarConfig().missile_hit_award, missile.getName());
		}
		
		// charger insta-kill
		if (tMap.hasAnyWithProperty(location, Names.kPropertyCharger)) {
			player.adjustHealth(player.getHealth() * -1, "hit on charger");
		}
		
		// check for kill
		if (player.getHealth() <= 0) {
			HashSet<Player> assailants = killedTanks.get(player);
			if (assailants == null) {
				assailants = new HashSet<Player>();
			}
			if (other != null) {
				assailants.add(other);
			}
			killedTanks.put(player, assailants);
		}
	}
	
	private void doMoveCollisions(Player player, PlayersManager players, 
			HashMap<Player, int []> newLocations, 
			HashMap<Integer, ArrayList<Player> > collisionMap, 
			ArrayList<Player> movedTanks) {
		
		// Get destination location
		int [] newLocation = newLocations.get(player);
		
		// Wall collisions checked for earlier
		
		// is there a collision in the cell
		ArrayList<Player> collision = collisionMap.get(Arrays.hashCode(newLocation));
		if (collision != null) {
			
			// there is a collision

			// if there is only one player here, cancel its move
			if (collision.size() == 1) {
				Player other = collision.get(0);
				if (players.getMove(other).move) {
					cancelMove(other, players, newLocations, movedTanks);
					doMoveCollisions(other, players, newLocations, collisionMap, movedTanks);
				}
			} 

			// If there is more than one guy here, they've already been cancelled
			
			
			// Add ourselves to this cell's collision list
			collision.add(player);
			collisionMap.put(Arrays.hashCode(newLocation), collision);
			
			// cancel my move
			if (players.getMove(player).move) {
				cancelMove(player, players, newLocations, movedTanks);
				doMoveCollisions(player, players, newLocations, collisionMap, movedTanks);
			}
			return;

		}

		// There is nothing in this cell, create a new list and add ourselves
		collision = new ArrayList<Player>(4);
		collision.add(player);
		collisionMap.put(Arrays.hashCode(newLocation), collision);
	}
	
	private void cancelMove(Player player, PlayersManager players, 
			HashMap<Player, int []> newLocations, 
			ArrayList<Player> movedTanks) {
		MoveInfo move = players.getMove(player);
		move.move = false;
		movedTanks.remove(player);
		newLocations.put(player, players.getLocation(player));
	}
	
	public int getMinimumAvailableLocations() {
		return Soar2D.config.tanksoarConfig().max_missile_packs + 1;
	}
	
	public void resetPlayer(GridMap map, Player player, PlayersManager players, boolean resetDuringRun) {
		player.reset();
	}

	public GridMap newMap() {
		return new TankSoarMap();
	}
}
