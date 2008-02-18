package soar2d.world;

import java.awt.Point;
import java.awt.geom.Point2D;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.ListIterator;
import java.util.logging.Level;

import soar2d.Names;
import soar2d.Soar2D;
import soar2d.Direction;
import soar2d.configuration.Configuration;
import soar2d.map.BookMap;
import soar2d.map.CellObject;
import soar2d.map.GridMap;
import soar2d.player.MoveInfo;
import soar2d.player.Player;

public class BookWorld implements IWorld {

	public boolean postLoad(GridMap _map) {
		BookMap map = (BookMap)_map;
		if (map.generateRoomStructure() == false) {
			return false;
		}
		return true;
	}
	
	private boolean updateDiscrete(BookMap map, PlayersManager players, double time) {
		
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			
			assert player.getRotationSpeed() == 0;
			assert player.getSpeed() == 0;
			assert player.getVelocity().x == 0.0;
			
			MoveInfo move = players.getMove(player);
			
			Soar2D.logger.finer("Processing move: " + player.getName());
			
			// rotate
			if (move.rotate) {
				int facing = player.getFacingInt();
				double heading = player.getHeadingRadians();
				if (move.rotateDirection.equals(Names.kRotateLeft)) {
					Soar2D.logger.finer("Rotate: left");
					player.setFacingInt(Direction.leftOf[facing]);
					heading -= Math.PI / 2;
				} 
				else if (move.rotateDirection.equals(Names.kRotateRight)) {
					Soar2D.logger.finer("Rotate: right");
					player.setFacingInt(Direction.rightOf[facing]);
					heading += Math.PI / 2;
				} 
				else {
					Soar2D.logger.warning("Rotate: invalid direction");
					move.rotate = false;
				}
				
				// update heading if we rotate
				if (move.rotate) {
					if (heading < 0) {
						Soar2D.logger.finest("Correcting computed negative heading");
						heading += 2 * Math.PI;
					} 

					heading = fmod(heading, 2 * Math.PI);
					Soar2D.logger.finer("Rotating, computed heading: " + heading);
					player.setHeadingRadians(heading);
				}
			} 

			// translate
			if (move.forward || move.backward) {
				Point oldLocation = players.getLocation(player);
				Point newLocation = new Point(oldLocation);

				if (move.forward && move.backward) {
					Soar2D.logger.warning("Move: both forward and backward indicated, ignoring");
				} 
				else if (move.forward) {
					Soar2D.logger.finer("Move: forward");
					Direction.translate(newLocation, player.getFacingInt());
				}
				else if (move.backward) {
					Soar2D.logger.finer("Move: backward");
					Direction.translate(newLocation, Direction.backwardOf[player.getFacingInt()]);
				}
				
				if (checkBlocked(newLocation, map)) {
					Soar2D.logger.info("Move: collision (blocked)");
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
				assert Soar2D.bConfig.getBlocksBlock() == false;
				
				Soar2D.logger.finer("Move: drop");
				
				// FIXME: store drop info for processing later
				map.addObjectToCell(players.getLocation(player), player.drop());
			}
			
			handleCommunication(move, player, players);
		}
		
		handleBookCollisions(players, map, findCollisions(players));
		
		updatePlayers(false, map, players);
		
		// do not reset after update
		return false;
	}

	private void handleCommunication(MoveInfo move, Player player, PlayersManager players) {
		// handle communication
		Iterator<MoveInfo.Communication> commIter = move.messages.iterator();
		while (commIter.hasNext()) {
			MoveInfo.Communication comm = commIter.next();
			Player toPlayer = players.get(comm.to);
			if (toPlayer == null) {
				Soar2D.logger.warning("Move: communicate: unknown player: " + comm.to);
				continue;
			}
			
			toPlayer.receiveMessage(player, comm.message);
		}
	}

	private void get(BookMap map, MoveInfo move, Player player) {
		Soar2D.logger.finer("Move: get, location " + move.getLocation.x + "," + move.getLocation.y);
		CellObject block = map.getObject(move.getLocation, "mblock");
		if (block == null || player.isCarrying()) {
			if (block == null) {
				Soar2D.logger.warning("get command failed, no object");
			} else {
				Soar2D.logger.warning("get command failed, full");
			}
			move.get = false;
			player.updateGetStatus(false);
		} else {
			// FIXME: store get info for processing later
			player.carry(map.getAllWithProperty(move.getLocation, "mblock").get(0));
			map.removeObject(move.getLocation, "mblock");
			player.updateGetStatus(true);
		}
	}

	private boolean updateContinuous(BookMap map, PlayersManager players, double time) {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			MoveInfo move = players.getMove(player);
			
			Soar2D.logger.finer("Processing move: " + player.getName());
			
			// update rotation speed
			if (move.rotate) {
				if (move.rotateDirection.equals(Names.kRotateLeft)) {
					Soar2D.logger.finer("Rotate: left");
					player.setRotationSpeed(Soar2D.bConfig.getRotateSpeed() * -1 * time);
				} 
				else if (move.rotateDirection.equals(Names.kRotateRight)) {
					Soar2D.logger.finer("Rotate: right");
					player.setRotationSpeed(Soar2D.bConfig.getRotateSpeed() * time);
				} 
				else if (move.rotateDirection.equals(Names.kRotateStop)) {
					Soar2D.logger.finer("Rotate: stop");
					player.setRotationSpeed(0);
				}
				player.resetDestinationHeading();
			} 
			else if (move.rotateAbsolute) {
				while (move.rotateAbsoluteHeading < 0) {
					Soar2D.logger.finest("Correcting command negative heading");
					move.rotateAbsoluteHeading += 2 * Math.PI;
				}
				move.rotateAbsoluteHeading = fmod(move.rotateAbsoluteHeading, 2 * Math.PI);

				Soar2D.logger.finer("Rotate absolute: " + move.rotateAbsoluteHeading);
				
				setRotationAndAbsoluteHeading(player, move.rotateAbsoluteHeading, time);
			}
			else if (move.rotateRelative) {
				double absoluteHeading = player.getHeadingRadians() + move.rotateRelativeYaw;
				
				while (absoluteHeading < 0) {
					Soar2D.logger.finest("Correcting command negative heading");
					absoluteHeading += 2 * Math.PI;
				}
				absoluteHeading = fmod(absoluteHeading, 2 * Math.PI);
				Soar2D.logger.finer("Rotate relative: " + move.rotateRelativeYaw + ", absolute: " + absoluteHeading);
				setRotationAndAbsoluteHeading(player, absoluteHeading, time);
			}

			// update heading if we rotate
			if (player.getRotationSpeed() != 0) {
				
				double heading = player.getHeadingRadians() + player.getRotationSpeed();
				
				if (heading < 0) {
					Soar2D.logger.finest("Correcting computed negative heading");
					heading += 2 * Math.PI;
				} 
				heading = fmod(heading, 2 * Math.PI);

				Soar2D.logger.finer("Rotating, computed heading: " + heading);
				
				if (player.hasDestinationHeading()) {
					double relativeHeading = player.getDestinationHeading() - heading;
					if (relativeHeading == 0) {
						Soar2D.logger.fine("Destination heading reached: " + heading);
						player.rotateComplete();
						player.resetDestinationHeading();
					} else {

						if (relativeHeading < 0) {
							relativeHeading += 2* Math.PI;
						}

						if (player.getRotationSpeed() < 0) {
							if (relativeHeading < Math.PI) {
								heading = player.getDestinationHeading();
								Soar2D.logger.fine("Destination heading reached: " + heading);
								player.rotateComplete();
								player.resetDestinationHeading();
								player.setRotationSpeed(0);
							} else {
								Soar2D.logger.finer("Destination heading pending");
							}
						}
						else if (player.getRotationSpeed() > 0) {
							if (relativeHeading > Math.PI) {
								Soar2D.logger.fine("Destination heading reached: " + heading);
								heading = player.getDestinationHeading();
								player.rotateComplete();
								player.resetDestinationHeading();
								player.setRotationSpeed(0);
							} else {
								Soar2D.logger.finer("Destination heading pending");
							}
						}
					}
				}
				player.setHeadingRadians(heading);
			}
			
			// update speed
			if (move.forward && move.backward) {
				Soar2D.logger.finer("Move: stop");
				player.setSpeed(0);
			} 
			else if (move.forward) {
				Soar2D.logger.finer("Move: forward");
				player.setSpeed(Soar2D.bConfig.getSpeed());
			}
			else if (move.backward) {
				Soar2D.logger.finer("Move: backward");
				player.setSpeed(Soar2D.bConfig.getSpeed() * -1);
			}
			
			// reset collision sensor
			player.setCollisionX(false);
			player.setCollisionY(false);

			// if we have velocity, process move
			if (player.getSpeed() != 0) {
				bookMovePlayerContinuous(player, map, players, time);
			} else {
				player.setVelocity(new Point2D.Double(0,0));
			}
			
			if (move.get) {
				get(map, move, player);
			}
			
			if (move.drop) {
				Point2D.Double dropFloatLocation = new Point2D.Double(players.getFloatLocation(player).x, players.getFloatLocation(player).y);
				dropFloatLocation.x += Soar2D.bConfig.getBookCellSize() * Math.cos(player.getHeadingRadians());
				dropFloatLocation.y += Soar2D.bConfig.getBookCellSize() * Math.sin(player.getHeadingRadians());
				java.awt.Point dropLocation = new java.awt.Point((int)dropFloatLocation.x / Soar2D.bConfig.getBookCellSize(), (int)dropFloatLocation.y / Soar2D.bConfig.getBookCellSize());
				
				if (dropLocation.equals(players.getLocation(player))) {
					dropFloatLocation.x += (Soar2D.bConfig.getBookCellSize() * 0.42) * Math.cos(player.getHeadingRadians());
					dropFloatLocation.y += (Soar2D.bConfig.getBookCellSize() * 0.42) * Math.sin(player.getHeadingRadians());
					dropLocation = new java.awt.Point((int)dropFloatLocation.x / Soar2D.bConfig.getBookCellSize(), (int)dropFloatLocation.y / Soar2D.bConfig.getBookCellSize());
					assert !dropLocation.equals(players.getLocation(player));
				}

				Soar2D.logger.finer("Move: drop " + dropLocation.x + "," + dropLocation.y);
				
				if (checkBlocked(dropLocation, map)) {
					Soar2D.logger.warning("drop command failed, blocked");
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
		return false;
	}

	public boolean update(GridMap _map, PlayersManager players) {
		double time = Soar2D.control.getTimeSlice();
		if (Soar2D.bConfig.getContinuous())
			return updateContinuous((BookMap)_map, players, time);
		return updateDiscrete((BookMap)_map, players, time);
	}
	
	private ArrayList<ArrayList<Player>> findCollisions(PlayersManager players) {
		ArrayList<ArrayList<Player>> collisions = new ArrayList<ArrayList<Player>>(players.numberOfPlayers() / 2);

		// Make sure collisions are possible
		if (players.numberOfPlayers() < 2) {
			return collisions;
		}
		
		// Optimization to not check the same name twice
		HashSet<Player> colliding = new HashSet<Player>(players.numberOfPlayers());
		ArrayList<Player> collision = new ArrayList<Player>(players.numberOfPlayers());

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
						
						if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer("collision at " + players.getLocation(left));
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
			player.setRotationSpeed(Soar2D.bConfig.getRotateSpeed() * -1 * time);
			player.setDestinationHeading(targetHeading);
		}
		else {
			player.setRotationSpeed(Soar2D.bConfig.getRotateSpeed() * time);
			player.setDestinationHeading(targetHeading);
		}
		
	}
		
	public void reset(GridMap map) {
	}
	
	public void fragPlayer(Player player, GridMap _map, PlayersManager players, Point location) {
		BookMap map = (BookMap)_map;
		player.setLocationId(map.getLocationId(location));
		players.setFloatLocation(player, defaultFloatLocation(location));
	}
	
	public void putInStartingLocation(Player player, GridMap _map, PlayersManager players, Point location) {
		BookMap map = (BookMap)_map;
		player.setLocationId(map.getLocationId(location));
		players.setFloatLocation(player, defaultFloatLocation(location));
	}

	private Point2D.Double defaultFloatLocation(Point location) {
		Point2D.Double floatLocation = new Point2D.Double();
		final int cellSize = Soar2D.bConfig.getBookCellSize();
		
		// default to center of square
		floatLocation.x = (location.x * cellSize) + (cellSize / 2); 
		floatLocation.y = (location.y * cellSize) + (cellSize / 2); 

		return floatLocation;
	}
	
	public void updatePlayers(boolean playersChanged, GridMap map, PlayersManager players) {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			player.update(players.getLocation(player));
		}
	}

	public double fmod(double a, double mod) {
		double result = a;
		assert mod > 0;
		while (Math.abs(result) >= mod) {
			if (result > 0) {
				result -= mod;
			} else {
				result += mod;
			}
		}
		return result;
	}
	
	private void bookMovePlayerContinuous(Player player, BookMap map, PlayersManager players, double time) {
		final int cellSize = Soar2D.bConfig.getBookCellSize();
		
		Point oldLocation = players.getLocation(player);
		Point newLocation = new Point(oldLocation);

		Point2D.Double oldFloatLocation = players.getFloatLocation(player);
		Point2D.Double newFloatLocation = new Point2D.Double(oldFloatLocation.x, oldFloatLocation.y);

		newFloatLocation.x += player.getSpeed() * Math.cos(player.getHeadingRadians()) * time;
		newFloatLocation.y += player.getSpeed() * Math.sin(player.getHeadingRadians()) * time;
		
		newLocation.x = (int)newFloatLocation.x / cellSize;
		newLocation.y = (int)newFloatLocation.y / cellSize;
		
		while (checkBlocked(newLocation, map)) {
			// 1) determine what edge we're intersecting
			if ((newLocation.x != oldLocation.x) && (newLocation.y != oldLocation.y)) {
				// corner case
				java.awt.Point oldx = new java.awt.Point(oldLocation.x, newLocation.y);
				
				// if oldx is blocked
				if (checkBlocked(oldx, map)) {
					player.setCollisionY(true);
					// calculate y first
					if (newLocation.y > oldLocation.y) {
						// south
						newFloatLocation.y = oldLocation.y * cellSize;
						newFloatLocation.y += cellSize - 0.1;
						newLocation.y = oldLocation.y;
					} 
					else if (newLocation.y < oldLocation.y) {
						// north
						newFloatLocation.y = oldLocation.y * cellSize;
						newLocation.y = oldLocation.y;
					} else {
						assert false;
					}
				} 
				else {
					player.setCollisionX(true);
					// calculate x first
					if (newLocation.x > oldLocation.x) {
						// east
						newFloatLocation.x = oldLocation.x * cellSize;
						newFloatLocation.x += cellSize - 0.1;
						newLocation.x = oldLocation.x;
					} 
					else if (newLocation.x < oldLocation.x) {
						// west
						newFloatLocation.x = oldLocation.x * cellSize;
						newLocation.x = oldLocation.x;
					} else {
						assert false;
					} 
				}
				continue;
			}
			
			if (newLocation.x > oldLocation.x) {
				player.setCollisionX(true);
				// east
				newFloatLocation.x = oldLocation.x * cellSize;
				newFloatLocation.x += cellSize - 0.1;
				newLocation.x = oldLocation.x;
			} 
			else if (newLocation.x < oldLocation.x) {
				player.setCollisionX(true);
				// west
				newFloatLocation.x = oldLocation.x * cellSize;
				newLocation.x = oldLocation.x;
			} 
			else if (newLocation.y > oldLocation.y) {
				player.setCollisionY(true);
				// south
				newFloatLocation.y = oldLocation.y * cellSize;
				newFloatLocation.y += cellSize - 0.1;
				newLocation.y = oldLocation.y;
			} 
			else if (newLocation.y < oldLocation.y) {
				player.setCollisionY(true);
				// north
				newFloatLocation.y = oldLocation.y * cellSize;
				newLocation.y = oldLocation.y;
			}
		}
		
		player.setVelocity(new Point2D.Double((newFloatLocation.x - oldFloatLocation.x)/time, (newFloatLocation.y - oldFloatLocation.y)/time));
		map.setPlayer(oldLocation, null);
		players.setLocation(player, newLocation);
		player.setLocationId(map.getLocationId(newLocation));
		players.setFloatLocation(player, newFloatLocation);
		map.setPlayer(newLocation, player);
	}
	
	private void handleBookCollisions(PlayersManager players, GridMap map, ArrayList<ArrayList<Player>> collisions) {
		
		// TODO: this is very similar to eaters code, consider combining to reduce redundancy
		
		// if there are no total collisions, we're done
		if (collisions.size() < 1) {
			return;
		}

		ArrayList<Player> collision = new ArrayList<Player>(players.numberOfPlayers());
		
		Iterator<ArrayList<Player>> collisionIter = collisions.iterator();
		while (collisionIter.hasNext()) {
			collision = collisionIter.next();

			assert collision.size() > 0;
			if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer("Processing collision group with " + collision.size() + " collidees.");

			// Remove from former location (only one of these for all players)
			map.setPlayer(players.getLocation(collision.get(0)), null);
			
			// Move to new cell
			ListIterator<Player> collideeIter = collision.listIterator();
			while (collideeIter.hasNext()) {
				Player player = collideeIter.next();
				Point location = Soar2D.simulation.world.putInStartingLocation(player, false);
				assert location != null;
				player.fragged();
			}
		}
	}
	
	private boolean checkBlocked(Point location, GridMap map) {
		if (map.getAllWithProperty(location, Names.kPropertyBlock).size() > 0) {
			return true;
		}
		if (Soar2D.bConfig.getBlocksBlock() && map.getAllWithProperty(location, "mblock").size() > 0) {
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

	public GridMap newMap(Configuration config) {
		return new BookMap(config);
	}

}
