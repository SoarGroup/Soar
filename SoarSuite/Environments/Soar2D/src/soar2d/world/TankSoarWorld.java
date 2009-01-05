package soar2d.world;

import java.awt.Point;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.logging.Level;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.configuration.Configuration;
import soar2d.map.CellObject;
import soar2d.map.GridMap;
import soar2d.map.TankSoarMap;
import soar2d.player.MoveInfo;
import soar2d.player.Player;

public class TankSoarWorld implements IWorld {

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
		while (tMap.numberMissilePacks() < Soar2D.config.tConfig.getMaxMissilePacks()) {
			if (spawnMissilePack(tMap, true) == false) {
				Soar2D.logger.severe("Missile pack spawn failed.");
				return false;
			}
		}
		return true;
	}
	
	
	private HashMap<Player, HashSet<Player> > killedTanks = new HashMap<Player, HashSet<Player> >(7);
	private int missileID = 0;
	private int missileReset = 0;

	public boolean update(GridMap _map, PlayersManager players) {
		TankSoarMap tMap = (TankSoarMap)_map;
		
		// We'll cache the tank new locations
		HashMap<Player, Point> newLocations = new HashMap<Player, Point>();
		
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
					if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer(player + ": fired with no ammo");
				}
			}
			
			// Check for rotate
			if (playerMove.rotate) {
				int facing = player.getFacingInt();
				if (playerMove.rotateDirection.equals(Names.kRotateLeft)) {
					facing = Direction.leftOf[facing];
				} else if (playerMove.rotateDirection.equals(Names.kRotateRight)) {
					facing = Direction.rightOf[facing];
				} else {
					Soar2D.logger.warning(player + ": unknown rotation command: " + playerMove.rotateDirection);
				}
				player.setFacingInt(facing);
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
			Point oldLocation = players.getLocation(player);
			
			if (!playerMove.move) {
				// No move, cross collision impossible
				newLocations.put(player, oldLocation);
				continue;
			}
			
			// we moved, calcuate new location
			Point newLocation = new Point(oldLocation);
			Direction.translate(newLocation, playerMove.moveDirection);
			
			//Cell dest = map.getCell(newLocation);
			
			// Check for wall collision
			if (!tMap.enterable(newLocation)) {
				// Moving in to wall, there will be no player in that cell

				// Cancel the move
				playerMove.move = false;
				newLocations.put(player, players.getLocation(player));
				
				// take damage
				String name = tMap.getAllWithProperty(newLocation, Names.kPropertyBlock).get(0).getName();
				player.adjustHealth(Soar2D.config.tConfig.getCollisionPenalty(), name);
				
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
			
			if (playerMove.moveDirection != Direction.backwardOf[otherMove.moveDirection]) {
				// we moved but not toward each other, cross collision impossible
				newLocations.put(player, newLocation);
				movedTanks.add(player);
				continue;
			}

			// Cross collision detected
			
			// take damage
			
			player.adjustHealth(Soar2D.config.tConfig.getCollisionPenalty(), "cross collision " + other);
			// Getting rammed on a charger is deadly
			if (tMap.getAllWithProperty(players.getLocation(player), Names.kPropertyCharger).size() > 0) {
				player.adjustHealth(player.getHealth() * -1, "hit on charger");
			}
			
			other.adjustHealth(Soar2D.config.tConfig.getCollisionPenalty(), "cross collision " + player);
			// Getting rammed on a charger is deadly
			if (tMap.getAllWithProperty(players.getLocation(other), Names.kPropertyCharger).size() > 0) {
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
		HashMap<Point, ArrayList<Player> > collisionMap = new HashMap<Point, ArrayList<Player> >();
		
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
					player.adjustEnergy(Soar2D.config.tConfig.getShieldEnergyUsage(), "shields");
				} else {
					if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer(player + ": shields ran out of energy");
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
				damage *= Soar2D.config.tConfig.getCollisionPenalty();
				
				if (Soar2D.logger.isLoggable(Level.FINE)) Soar2D.logger.fine("Collision, " + (damage * -1) + " damage:");
				
				playerIter = collision.iterator();
				while (playerIter.hasNext()) {
					Player player = playerIter.next();
					player.adjustHealth(damage, "collision");
					
					// Getting rammed on a charger is deadly
					if (tMap.getAllWithProperty(players.getLocation(player), Names.kPropertyCharger).size() > 0) {
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
			Point location = newLocations.get(player);
			players.setLocation(player, location);
			tMap.setPlayer(location, player);
			
			// get missile pack
			ArrayList<CellObject> missilePacks = tMap.getAllWithProperty(location, Names.kPropertyMissiles);
			if (missilePacks.size() > 0) {
				assert missilePacks.size() == 1;
				CellObject pack = missilePacks.get(0);
				pack.apply(player);
				tMap.removeAllWithProperty(location, Names.kPropertyMissiles);
			}
			
			
			// is there a missile in the cell?
			ArrayList<CellObject> missiles = tMap.getAllWithProperty(location, Names.kPropertyMissile);
			if (missiles.size() == 0) {
				// No, can't collide
				continue;
			}

			// are any flying toward me?
			Iterator<CellObject> iter = missiles.iterator();
			MoveInfo move = players.getMove(player);
			while (iter.hasNext()) {
				CellObject missile = iter.next();
				if (move.moveDirection == Direction.backwardOf[missile.getIntProperty(Names.kPropertyDirection)]) {
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
			Point missileLoc = new Point(players.getLocation(player));
			
			int direction = player.getFacingInt();
			Direction.translate(missileLoc, direction);
			
			if (!tMap.isInBounds(missileLoc)) {
				continue;
			}
			if (!tMap.enterable(missileLoc)) {
				// explosion
				tMap.setExplosion(missileLoc);
				continue;
			}
			
			CellObject missile = tMap.createRandomObjectWithProperty(Names.kPropertyMissile);
			missile.setName(player + "-" + missileID++);
			missile.addProperty(Names.kPropertyDirection, Integer.toString(direction));
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
		if (tMap.numberMissilePacks() < Soar2D.config.tConfig.getMaxMissilePacks()) {
			spawnMissilePack(tMap, false);
		}
		
		// Respawn killed Tanks in safe squares
		playerIter = killedTanks.keySet().iterator();
		while (playerIter.hasNext()) {
			
			// apply points
			Player player = playerIter.next();
			
			player.adjustPoints(Soar2D.config.tConfig.getKillPenalty(), "fragged");
			assert killedTanks.containsKey(player);
			Iterator<Player> killedPlayerIter = killedTanks.get(player).iterator();
			while (killedPlayerIter.hasNext()) {
				Player assailant = killedPlayerIter.next();
				if (assailant.equals(player)) {
					continue;
				}
				assailant.adjustPoints(Soar2D.config.tConfig.getKillAward(), "fragged " + player);
			}
			
			Soar2D.simulation.world.fragPlayer(player);
		}
		
		// if the missile reset counter is 100 and there were no killed tanks
		// this turn, reset all tanks
		if ((missileReset >= Soar2D.config.tConfig.getMissileResetThreshold()) && (killedTanks.size() == 0)) {
			Soar2D.logger.info("missile reset threshold exceeded, resetting all tanks");
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
		return false;
	}
	
	private void handleRadarEnergy(Player player) {
		int available = player.getEnergy();
		if (available < player.getRadarPower()) {
			if (available > 0) {
				if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer(player.getName() + ": reducing radar power due to energy shortage");
				player.setRadarPower(available);
			} else {
				if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer(player.getName() + ": radar switched off due to energy shortage");
				player.setRadarSwitch(false);
				return;
			}
		}
		player.adjustEnergy(player.getRadarPower() * -1, "radar");
	}
	
	public void fragPlayer(Player player, GridMap map, PlayersManager players, Point location) {
		
	}
	
	public void putInStartingLocation(Player player, GridMap map, PlayersManager players, Point location) {
		
	}
	
	public void reset(GridMap map) {
		missileID = 0;
		missileReset = 0;
	}
	
	private void chargeUp(Player player, TankSoarMap map, Point location) {
		// Charge up
		ArrayList<CellObject> chargers = map.getAllWithProperty(location, Names.kPropertyCharger);
		Iterator<CellObject> iter = chargers.iterator();
		while (iter.hasNext()) {
			CellObject charger = iter.next();
			if (charger.hasProperty(Names.kPropertyHealth)) {
				player.setOnHealthCharger(true);
				if (player.getHealth() < Soar2D.config.tConfig.getDefaultHealth()) {
					player.adjustHealth(charger.getIntProperty(Names.kPropertyHealth), "charger");
				}
			}
			if (charger.hasProperty(Names.kPropertyEnergy)) {
				player.setOnEnergyCharger(true);
				if (player.getEnergy() < Soar2D.config.tConfig.getDefaultEnergy()) {
					player.adjustEnergy(charger.getIntProperty(Names.kPropertyEnergy), "charger");
				}
			}
		}
	}
	

	
	public void updatePlayers(boolean playersChanged, GridMap map, PlayersManager players) {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			
			if (playersChanged) {
				player.playersChanged();
			}
			if (players.numberOfPlayers() < 2) {
				player.setSmell(0, null);
				player.setSound(0);
			} else {
				int distance = 99;
				String color = null;
				
				Iterator<Player> smellIter = players.iterator();
				while (smellIter.hasNext()) {
					
					Player other = smellIter.next();
					if (other.equals(player)) {
						continue;
					}
					
					Point playerLoc = players.getLocation(player);
					Point otherLoc = players.getLocation(other);
					int newDistance = Math.abs(playerLoc.x - otherLoc.x) + Math.abs(playerLoc.y - otherLoc.y);
					
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
				// TODO: can eliminate sound check if smell is greater than max sound distance 
				player.setSound(map.getSoundNear(players.getLocation(player)));
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
	


	private boolean addCharger(boolean health, TankSoarMap newMap) {
		ArrayList<Point> locations = newMap.getAvailableLocations();
		if (locations.size() <= 0) {
			Soar2D.control.severeError("No place to put charger!");
			return false;
		}
		
		Point location = locations.get(Simulation.random.nextInt(locations.size()));
		if (health) {
			Soar2D.logger.info("spawning health charger at (" + location.x + "," + location.y + ")");
			if (!newMap.addRandomObjectWithProperties(location, Names.kPropertyHealth, Names.kPropertyCharger)) {
				Soar2D.control.severeError("Couldn't add charger to map!");
				return false;
			}
		} else {			
			Soar2D.logger.info("spawning energy charger at (" + location.x + "," + location.y + ")");
			if (!newMap.addRandomObjectWithProperties(location, Names.kPropertyEnergy, Names.kPropertyCharger)) {
				Soar2D.control.severeError("Couldn't add charger to map!");
				return false;
			}
		}
		
		return true;
	}
	
	private boolean spawnMissilePack(TankSoarMap theMap, boolean force) {
		if (force || (Simulation.random.nextInt(100) < Soar2D.config.tConfig.getMissilePackRespawnChance())) {
			// Get available spots
			ArrayList<Point> spots = theMap.getAvailableLocations();
			if (spots.size() <= 0) {
				return false;
			}
			
			// Add a missile pack to a spot
			Point spot = spots.get(Simulation.random.nextInt(spots.size()));
			Soar2D.logger.info("spawning missile pack at (" + spot.x + "," + spot.y + ")");
			boolean ret = theMap.addRandomObjectWithProperty(spot, Names.kPropertyMissiles);
			if (ret == false) {
				return false;
			}
		}
		return true;
	}
	
	public void missileHit(Player player, TankSoarMap tMap, Point location, CellObject missile, PlayersManager players) {
		// Yes, I'm hit
		missile.apply(player);
		
		// apply points
		player.adjustPoints(Soar2D.config.tConfig.getMissileHitPenalty(), missile.getName());
		Player other = players.get(missile.getProperty(Names.kPropertyOwner));
		// can be null if the player was deleted after he fired but before the missile hit
		if (other != null) {
			other.adjustPoints(Soar2D.config.tConfig.getMissileHitAward(), missile.getName());
		}
		
		// charger insta-kill
		if (tMap.getAllWithProperty(location, Names.kPropertyCharger).size() > 0) {
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
			HashMap<Player, Point> newLocations, 
			HashMap<Point, ArrayList<Player> > collisionMap, 
			ArrayList<Player> movedTanks) {
		
		// Get destination location
		Point newLocation = newLocations.get(player);
		
		// Wall collisions checked for earlier
		
		// is there a collision in the cell
		ArrayList<Player> collision = collisionMap.get(newLocation);
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
			collisionMap.put(newLocation, collision);
			
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
		collisionMap.put(newLocation, collision);
	}
	
	private void cancelMove(Player player, PlayersManager players, 
			HashMap<Player, Point> newLocations, 
			ArrayList<Player> movedTanks) {
		MoveInfo move = players.getMove(player);
		move.move = false;
		movedTanks.remove(player);
		newLocations.put(player, players.getLocation(player));
	}
	
	public int getMinimumAvailableLocations() {
		return Soar2D.config.tConfig.getMaxMissilePacks() + 1;
	}
	
	public void resetPlayer(GridMap map, Player player, PlayersManager players, boolean resetDuringRun) {
		player.reset();
	}

	public GridMap newMap(Configuration config) {
		return new TankSoarMap(config);
	}
}
