package broken.soar2d.world;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Set;

import org.apache.log4j.Logger;

import soar2d.Names;
import soar2d.Soar2D;
import soar2d.Direction;
import soar2d.map.BookMap;
import soar2d.map.CellObject;
import soar2d.map.GridMap;
import soar2d.players.CommandInfo;
import soar2d.players.Player;
import soar2d.players.Player;

public class BookWorld implements World {
	private static Logger logger = Logger.getLogger(BookWorld.class);

	public BookWorld(String map) throws Exception {
		
	}
	
	public boolean postLoad(GridMap _map) {
		BookMap map = (BookMap)_map;
		if (map.generateRoomStructure() == false) {
			return false;
		}
		return true;
	}
	
	private String updateDiscrete(BookMap map, PlayersManager players, double time) {
		
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			
			assert player.getRotationSpeed() == 0;
			assert player.getSpeed() == 0;
			assert player.getVelocity()[0] == 0.0;
			
			CommandInfo move = players.getCommand(player);
			
			logger.debug("Processing move: " + player.getName());
			
			// rotate
			if (move.rotate) {
				Direction facing = player.getFacing();
				if (move.rotateDirection.equals(Names.kRotateLeft)) {
					logger.debug("Rotate: left");
					player.setFacing(facing.left());
				} 
				else if (move.rotateDirection.equals(Names.kRotateRight)) {
					logger.debug("Rotate: right");
					player.setFacing(facing.right());
				} 
				else {
					logger.warn("Rotate: invalid direction");
					move.rotate = false;
				}
				
				// update heading if we rotate
			} 

			// translate
			if (move.forward || move.backward) {
				int [] oldLocation = players.getLocation(player);
				int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);

				if (move.forward && move.backward) {
					logger.warn("Move: both forward and backward indicated, ignoring");
				} 
				else if (move.forward) {
					logger.debug("Move: forward");
					Direction.translate(newLocation, player.getFacing());
				}
				else if (move.backward) {
					logger.debug("Move: backward");
					Direction.translate(newLocation, player.getFacing().backward());
				}
				
				if (checkBlocked(newLocation, map)) {
					logger.info("Move: collision (blocked)");
				} else {
					map.setPlayer(oldLocation, null);
					players.setLocation(player, newLocation);
					player.setLocationId(map.getLocationId(newLocation));
					map.setPlayer(newLocation, player);
					
					players.setFloatLocation(player, defaultFloatLocation(newLocation));
				}
			}

			if (move.get) {
				get(map, move, player);
			}
			
			if (move.drop) {
				assert Soar2D.config.roomConfig().blocks_block == false;
				
				logger.debug("Move: drop");
				
				// FIXME: store drop info for processing later
				map.addObjectToCell(players.getLocation(player), player.drop());
			}
			
			handleCommunication(move, player, players);
		}
		
		handleBookCollisions(players, map, findCollisions(players));
		
		updatePlayers(false, map, players);
		
		// do not reset after update
		return null;
	}

	private void handleCommunication(CommandInfo move, Player player, PlayersManager players) {
		// handle communication
		Iterator<CommandInfo.Communication> commIter = move.messages.iterator();
		while (commIter.hasNext()) {
			CommandInfo.Communication comm = commIter.next();
			Player toPlayer = players.get(comm.to);
			if (toPlayer == null) {
				logger.warn("Move: communicate: unknown player: " + comm.to);
				continue;
			}
			
			toPlayer.receiveMessage(player, comm.message);
		}
	}

	private void get(BookMap map, CommandInfo move, Player player) {
		logger.debug("Move: get, location " + move.getLocation[0] + "," + move.getLocation[1]);
		CellObject block = map.getObject(move.getLocation, Names.kBookObjectName);
		if (block == null || player.isCarrying()) {
			if (block == null) {
				logger.warn("get command failed, no object");
			} else {
				logger.warn("get command failed, full");
			}
			move.get = false;
			player.updateGetStatus(false);
		} else {
			// FIXME: store get info for processing later
			player.carry(map.getAllWithProperty(move.getLocation, Names.kBookObjectName).get(0));
			map.removeObject(move.getLocation, Names.kBookObjectName);
			player.updateGetStatus(true);
		}
	}

	private String updateContinuous(BookMap map, PlayersManager players, double time) {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			CommandInfo move = players.getCommand(player);
			
			logger.debug("Processing move: " + player.getName());
			
			// update rotation speed
			if (move.rotate) {
				if (move.rotateDirection.equals(Names.kRotateLeft)) {
					logger.debug("Rotate: left");
					player.setRotationSpeed(Soar2D.config.roomConfig().rotate_speed * -1 * time);
				} 
				else if (move.rotateDirection.equals(Names.kRotateRight)) {
					logger.debug("Rotate: right");
					player.setRotationSpeed(Soar2D.config.roomConfig().rotate_speed * time);
				} 
				else if (move.rotateDirection.equals(Names.kRotateStop)) {
					logger.debug("Rotate: stop");
					player.setRotationSpeed(0);
				}
				player.resetDestinationHeading();
			} 
			else if (move.rotateAbsolute) {
				while (move.rotateAbsoluteHeading < 0) {
					logger.trace("Correcting command negative heading");
					move.rotateAbsoluteHeading += 2 * Math.PI;
				}
				move.rotateAbsoluteHeading = Direction.fmod(move.rotateAbsoluteHeading, 2 * Math.PI);

				logger.debug("Rotate absolute: " + move.rotateAbsoluteHeading);
				
				setRotationAndAbsoluteHeading(player, move.rotateAbsoluteHeading, time);
			}
			else if (move.rotateRelative) {
				double absoluteHeading = player.getHeadingRadians() + move.rotateRelativeYaw;
				
				while (absoluteHeading < 0) {
					logger.trace("Correcting command negative heading");
					absoluteHeading += 2 * Math.PI;
				}
				absoluteHeading = Direction.fmod(absoluteHeading, 2 * Math.PI);
				logger.debug("Rotate relative: " + move.rotateRelativeYaw + ", absolute: " + absoluteHeading);
				setRotationAndAbsoluteHeading(player, absoluteHeading, time);
			}

			// update heading if we rotate
			if (player.getRotationSpeed() != 0) {
				
				double heading = player.getHeadingRadians() + player.getRotationSpeed();
				
				if (heading < 0) {
					logger.trace("Correcting computed negative heading");
					heading += 2 * Math.PI;
				} 
				heading = Direction.fmod(heading, 2 * Math.PI);

				logger.debug("Rotating, computed heading: " + heading);
				
				if (player.hasDestinationHeading()) {
					double relativeHeading = player.getDestinationHeading() - heading;
					if (relativeHeading == 0) {
						logger.debug("Destination heading reached: " + heading);
						player.rotateComplete();
						player.resetDestinationHeading();
					} else {

						if (relativeHeading < 0) {
							relativeHeading += 2* Math.PI;
						}

						if (player.getRotationSpeed() < 0) {
							if (relativeHeading < Math.PI) {
								heading = player.getDestinationHeading();
								logger.debug("Destination heading reached: " + heading);
								player.rotateComplete();
								player.resetDestinationHeading();
								player.setRotationSpeed(0);
							} else {
								logger.debug("Destination heading pending");
							}
						}
						else if (player.getRotationSpeed() > 0) {
							if (relativeHeading > Math.PI) {
								logger.debug("Destination heading reached: " + heading);
								heading = player.getDestinationHeading();
								player.rotateComplete();
								player.resetDestinationHeading();
								player.setRotationSpeed(0);
							} else {
								logger.debug("Destination heading pending");
							}
						}
					}
				}
				player.setHeadingRadians(heading);
			}
			
			// update speed
			if (move.forward && move.backward) {
				logger.debug("Move: stop");
				player.setSpeed(0);
			} 
			else if (move.forward) {
				logger.debug("Move: forward");
				player.setSpeed(Soar2D.config.roomConfig().speed);
			}
			else if (move.backward) {
				logger.debug("Move: backward");
				player.setSpeed(Soar2D.config.roomConfig().speed * -1);
			}
			
			// reset collision sensor
			player.setCollisionX(false);
			player.setCollisionY(false);

			// if we have velocity, process move
			if (player.getSpeed() != 0) {
				bookMovePlayerContinuous(player, map, players, time);
			} else {
				player.setVelocity(new double [] { 0, 0 });
			}
			
			if (move.get) {
				get(map, move, player);
			}
			
			if (move.drop) {
				double [] dropFloatLocation = Arrays.copyOf(players.getFloatLocation(player), players.getFloatLocation(player).length);
				dropFloatLocation[0] += Soar2D.config.roomConfig().cell_size * Math.cos(player.getHeadingRadians());
				dropFloatLocation[1] += Soar2D.config.roomConfig().cell_size * Math.sin(player.getHeadingRadians());
				int [] dropLocation = new int [] { (int)dropFloatLocation[0] / Soar2D.config.roomConfig().cell_size, (int)dropFloatLocation[1] / Soar2D.config.roomConfig().cell_size };
				
				if (dropLocation.equals(players.getLocation(player))) {
					dropFloatLocation[0] += (Soar2D.config.roomConfig().cell_size * 0.42) * Math.cos(player.getHeadingRadians());
					dropFloatLocation[1] += (Soar2D.config.roomConfig().cell_size * 0.42) * Math.sin(player.getHeadingRadians());
					dropLocation = new int [] { (int)dropFloatLocation[0] / Soar2D.config.roomConfig().cell_size, (int)dropFloatLocation[1] / Soar2D.config.roomConfig().cell_size };
					assert !dropLocation.equals(players.getLocation(player));
				}

				logger.debug("Move: drop " + dropLocation[0] + "," + dropLocation[1]);
				
				if (checkBlocked(dropLocation, map)) {
					logger.warn("drop command failed, blocked");
					move.drop = false;
					player.updateDropStatus(false);
				} else {
					// FIXME: store drop info for processing later
					map.addObjectToCell(dropLocation, player.drop());
					player.updateDropStatus(true);
				}
			}
			
			handleCommunication(move, player, players);
		}
		
		handleBookCollisions(players, map, findCollisions(players));
		
		updatePlayers(false, map, players);
		
		// do not reset after update
		return null;
	}

	public String update(GridMap _map, PlayersManager players) {
		double time = Soar2D.control.getTimeSlice();
		if (Soar2D.config.roomConfig().continuous)
			return updateContinuous((BookMap)_map, players, time);
		return updateDiscrete((BookMap)_map, players, time);
	}
	
	private List<List<Player>> findCollisions(PlayersManager players) {
		List<List<Player>> collisions = new ArrayList<ArrayList<Player>>(players.numberOfPlayers() / 2);

		// Make sure collisions are possible
		if (players.numberOfPlayers() < 2) {
			return collisions;
		}
		
		// Optimization to not check the same name twice
		Set<Player> colliding = new HashSet<Player>(players.numberOfPlayers());
		List<Player> collision = new ArrayList<Player>(players.numberOfPlayers());

		ListIterator<Player> leftIter = players.listIterator();
		while (leftIter.hasNext()) {
			Player left = leftIter.next();
			
			// Check to see if we're already colliding
			if (colliding.contains(left)) {
				continue;
			}
			
			ListIterator<Player> rightIter = players.listIterator(leftIter.nextIndex());
			// Clear collision list now
			collision.clear();
			while (rightIter.hasNext()) {
				// For each player to my right (I've already checked to my left)
				Player right = rightIter.next();

				// Check to see if we're already colliding
				if (colliding.contains(right)) {
					continue;
				}
				
				// If the locations match, we have a collision
				if (players.getLocation(left).equals(players.getLocation(right))) {
					
					// Add to this set to avoid checking same player again
					colliding.add(left);
					colliding.add(right);
					
					// Add the left the first time a collision is detected
					if (collision.size() == 0) {
						collision.add(left);
						
						logger.debug("collision at " + players.getLocation(left));
					}
					// Add each right as it is detected
					collision.add(right);
				}
			}
			
			// Add the collision to the total collisions if there is one
			if (collision.size() > 0) {
				collisions.add(new ArrayList<Player>(collision));
			}
		}

		return collisions;
	}
		
	private void setRotationAndAbsoluteHeading(Player player, double targetHeading, double time) {
		double relativeHeading = targetHeading - player.getHeadingRadians();
		if (relativeHeading == 0) {
			player.rotateComplete();
			player.setRotationSpeed(0);
			player.resetDestinationHeading();
			return;
		}

		if (relativeHeading < 0) {
			relativeHeading += 2* Math.PI;
		}
		
		if (relativeHeading > Math.PI) {
			player.setRotationSpeed(Soar2D.config.roomConfig().rotate_speed * -1 * time);
			player.setDestinationHeading(targetHeading);
		}
		else {
			player.setRotationSpeed(Soar2D.config.roomConfig().rotate_speed * time);
			player.setDestinationHeading(targetHeading);
		}
		
	}
		
	public void reset(GridMap map) {
	}
	
	public void fragPlayer(Player player, GridMap _map, PlayersManager players, int [] location) {
		BookMap map = (BookMap)_map;
		player.setLocationId(map.getLocationId(location));
		players.setFloatLocation(player, defaultFloatLocation(location));
	}
	
	public void putInStartingLocation(Player player, GridMap _map, PlayersManager players, int [] location) {
		BookMap map = (BookMap)_map;
		player.setLocationId(map.getLocationId(location));
		players.setFloatLocation(player, defaultFloatLocation(location));
	}

	private double [] defaultFloatLocation(int [] location) {
		double [] floatLocation = new double [2];
		final int cellSize = Soar2D.config.roomConfig().cell_size;
		
		// default to center of square
		floatLocation[0] = (location[0] * cellSize) + (cellSize / 2); 
		floatLocation[1] = (location[1] * cellSize) + (cellSize / 2); 

		return floatLocation;
	}
	
	public void updatePlayers(boolean playersChanged, GridMap map, PlayersManager players) {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			player.update(players.getLocation(player));
		}
	}

	private void bookMovePlayerContinuous(Player player, BookMap map, PlayersManager players, double time) {
		final int cellSize = Soar2D.config.roomConfig().cell_size;
		
		int [] oldLocation = players.getLocation(player);
		int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);

		double [] oldFloatLocation = players.getFloatLocation(player);
		double [] newFloatLocation = Arrays.copyOf(oldFloatLocation, oldFloatLocation.length);

		newFloatLocation[0] += player.getSpeed() * Math.cos(player.getHeadingRadians()) * time;
		newFloatLocation[1] += player.getSpeed() * Math.sin(player.getHeadingRadians()) * time;
		
		newLocation[0] = (int)newFloatLocation[0] / cellSize;
		newLocation[1] = (int)newFloatLocation[1] / cellSize;
		
		while (checkBlocked(newLocation, map)) {
			// 1) determine what edge we're intersecting
			if ((newLocation[0] != oldLocation[0]) && (newLocation[1] != oldLocation[1])) {
				// corner case
				int [] oldx = new int [] { oldLocation[0], newLocation[1] };
				
				// if oldx is blocked
				if (checkBlocked(oldx, map)) {
					player.setCollisionY(true);
					// calculate y first
					if (newLocation[1] > oldLocation[1]) {
						// south
						newFloatLocation[1] = oldLocation[1] * cellSize;
						newFloatLocation[1] += cellSize - 0.1;
						newLocation[1] = oldLocation[1];
					} 
					else if (newLocation[1] < oldLocation[1]) {
						// north
						newFloatLocation[1] = oldLocation[1] * cellSize;
						newLocation[1] = oldLocation[1];
					} else {
						assert false;
					}
				} 
				else {
					player.setCollisionX(true);
					// calculate x first
					if (newLocation[0] > oldLocation[0]) {
						// east
						newFloatLocation[0] = oldLocation[0] * cellSize;
						newFloatLocation[0] += cellSize - 0.1;
						newLocation[0] = oldLocation[0];
					} 
					else if (newLocation[0] < oldLocation[0]) {
						// west
						newFloatLocation[0] = oldLocation[0] * cellSize;
						newLocation[0] = oldLocation[0];
					} else {
						assert false;
					} 
				}
				continue;
			}
			
			if (newLocation[0] > oldLocation[0]) {
				player.setCollisionX(true);
				// east
				newFloatLocation[0] = oldLocation[0] * cellSize;
				newFloatLocation[0] += cellSize - 0.1;
				newLocation[0] = oldLocation[0];
			} 
			else if (newLocation[0] < oldLocation[0]) {
				player.setCollisionX(true);
				// west
				newFloatLocation[0] = oldLocation[0] * cellSize;
				newLocation[0] = oldLocation[0];
			} 
			else if (newLocation[1] > oldLocation[1]) {
				player.setCollisionY(true);
				// south
				newFloatLocation[1] = oldLocation[1] * cellSize;
				newFloatLocation[1] += cellSize - 0.1;
				newLocation[1] = oldLocation[1];
			} 
			else if (newLocation[1] < oldLocation[1]) {
				player.setCollisionY(true);
				// north
				newFloatLocation[1] = oldLocation[1] * cellSize;
				newLocation[1] = oldLocation[1];
			}
		}
		
		player.setVelocity(new double [] { (newFloatLocation[0] - oldFloatLocation[0])/time, (newFloatLocation[1] - oldFloatLocation[1])/time });
		map.setPlayer(oldLocation, null);
		players.setLocation(player, newLocation);
		player.setLocationId(map.getLocationId(newLocation));
		players.setFloatLocation(player, newFloatLocation);
		map.setPlayer(newLocation, player);
	}
	
	private void handleBookCollisions(PlayersManager players, GridMap map, List<List<Player>> collisions) {
		
		// TODO: this is very similar to eaters code, consider combining to reduce redundancy
		
		// if there are no total collisions, we're done
		if (collisions.size() < 1) {
			return;
		}

		List<Player> collision = new ArrayList<Player>(players.numberOfPlayers());
		
		Iterator<List<Player>> collisionIter = collisions.iterator();
		while (collisionIter.hasNext()) {
			collision = collisionIter.next();

			assert collision.size() > 0;
			logger.debug("Processing collision group with " + collision.size() + " collidees.");

			// Remove from former location (only one of these for all players)
			map.setPlayer(players.getLocation(collision.get(0)), null);
			
			// Move to new cell
			ListIterator<Player> collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				Player player = collideeIter.next();
				int [] location = Soar2D.simulation.world.putInStartingLocation(player, false);
				assert location != null;
				player.fragged();
			}
		}
	}
	
	private boolean checkBlocked(int [] location, GridMap map) {
		if (map.hasAnyWithProperty(location, Names.kPropertyBlock)) {
			return true;
		}
		if (Soar2D.config.roomConfig().blocks_block && map.hasAnyWithProperty(location, Names.kBookObjectName)) {
			// FIXME: check height
			return true;
		}
		return false;
	}
	
	public int getMinimumAvailableLocations() {
		return 1;
	}

	public void resetPlayer(GridMap map, Player player, PlayersManager players, boolean resetDuringRun) {
		if (resetDuringRun) {
			player.fragged();
		} else {
			// reset (init-soar)
			player.reset();
		}
	}

	public GridMap newMap() {
		return new BookMap();
	}

	double angleOff(Player left, Player right) {
		double [] target = new double [] { floatLocations.get(right)[0], floatLocations.get(right)[1] };
		
		return angleOff(left, target);
	}

	double angleOff(Player left, double [] target) {
		double [] playerVector = new double [] { floatLocations.get(left)[0], floatLocations.get(left)[1] };

		if (Soar2D.config.roomConfig().continuous == false) {
			// translate the player's location back a little bit to increase peripheral vision
			playerVector[0] -= Math.cos(left.getHeadingRadians());
			playerVector[1] -= Math.sin(left.getHeadingRadians());
		}
			
		double [] targetVector = new double [] { target[0], target[1] };
		
		// translate target so i'm the origin
		targetVector[0] -= playerVector[0];
		targetVector[1] -= playerVector[1];
		
		// make target unit vector
		double targetVectorLength = Math.sqrt(Math.pow(targetVector[0], 2) + Math.pow(targetVector[1], 2));
		if (targetVectorLength > 0) {
			targetVector[0] /= targetVectorLength;
			targetVector[1] /= targetVectorLength;
		} else {
			targetVector[0] = 0;
			targetVector[1] = 0;
		}
		
		// make player facing vector
		playerVector[0] = Math.cos(left.getHeadingRadians());
		playerVector[1] = Math.sin(left.getHeadingRadians());
		
		double dotProduct = (targetVector[0] * playerVector[0]) + (targetVector[1] * playerVector[1]);
		double crossProduct = (targetVector[0] * playerVector[1]) - (targetVector[1] * playerVector[0]);
		
		// calculate inverse cosine of that for angle
		if (crossProduct < 0) {
			return Math.acos(dotProduct);
		}
		return Math.acos(dotProduct) * -1;
	}

}
