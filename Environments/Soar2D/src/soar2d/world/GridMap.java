package soar2d.world;

import java.awt.Point;
import java.util.*;

import soar2d.*;
import soar2d.player.*;

/**
 * @author voigtjr
 *
 * houses the map and associated meta-data. used for grid worlds.
 */
public class GridMap {
	
	/**
	 * the maps are square, this is the number of row/columns
	 */
	int size = 0;
	/**
	 * the cells
	 */
	Cell[][] mapCells = null;
	/**
	 * the types of objects on this map
	 */
	CellObjectManager cellObjectManager = new CellObjectManager();
	/**
	 * true if there is a health charger
	 */
	private boolean health = false;
	/**
	 * true if there is an energy charger
	 */
	private boolean energy = false;
	/**
	 * returns the number of missile packs on the map
	 */
	private int missilePacks = 0;

	private int scoreCount = 0;
	private int foodCount = 0;
	
	HashSet<CellObject> updatables = new HashSet<CellObject>();
	HashMap<CellObject, java.awt.Point> updatablesLocations = new HashMap<CellObject, java.awt.Point>();
	HashSet<CellObject> unopenedBoxes = new HashSet<CellObject>();
	
	public int getSize() {
		return size;
	}
	
	public int getScoreCount() {
		return scoreCount;
	}
	
	public int getFoodCount() {
		return foodCount;
	}
	
	public int getUnopenedBoxCount() {
		return unopenedBoxes.size();
	}
	
	public int numberMissilePacks() {
		return missilePacks;
	}
	
	public boolean hasHealthCharger() {
		return health;
	}
	
	public boolean hasEnergyCharger() {
		return energy;
	}
	
	public CellObjectManager getObjectManager() {
		return cellObjectManager;
	}
	
	Cell getCell(java.awt.Point location) {
		assert location.x >= 0;
		assert location.y >= 0;
		assert location.x < size;
		assert location.y < size;
		return mapCells[location.y][location.x];
	}
	
	Cell getCell(int x, int y) {
		assert x >= 0;
		assert y >= 0;
		assert x < size;
		assert y < size;
		return mapCells[y][x];
	}
	
	public boolean addRandomObjectWithProperty(java.awt.Point location, String property) {
		CellObject object = cellObjectManager.createRandomObjectWithProperty(property);
		if (object == null) {
			return false;
		}
		addObjectToCell(location, object);
		return true;
	}

	public CellObject createRandomObjectWithProperty(String property) {
		return cellObjectManager.createRandomObjectWithProperty(property);
	}

	public boolean addRandomObjectWithProperties(java.awt.Point location, String property1, String property2) {
		CellObject object = cellObjectManager.createRandomObjectWithProperties(property1, property2);
		if (object == null) {
			return false;
		}
		addObjectToCell(location, object);
		return true;
	}
	
	public void setPlayer(java.awt.Point location, Player player) {
		Cell cell = getCell(location);
		cell.setPlayer(player);
		setRedraw(cell);
	}
	
	public int pointsCount(java.awt.Point location) {
		Cell cell = getCell(location);
		ArrayList list = cell.getAllWithProperty(Names.kPropertyEdible);
		Iterator iter = list.iterator();
		int count = 0;
		while (iter.hasNext()) {
			count += ((CellObject)iter.next()).getIntProperty(Names.kPropertyPoints);
		}
		return count;
	}
	
	public boolean isAvailable(java.awt.Point location) {
		Cell cell = getCell(location);
		boolean enterable = cell.enterable();
		boolean noPlayer = cell.getPlayer() == null;
		boolean noMissilePack = cell.getAllWithProperty(Names.kPropertyMissiles).size() <= 0;
		boolean noCharger = cell.getAllWithProperty(Names.kPropertyCharger).size() <= 0;
		return enterable && noPlayer && noMissilePack && noCharger;
	}
	
	public boolean enterable(java.awt.Point location) {
		Cell cell = getCell(location);
		return cell.enterable();
	}
	
	public CellObject removeObject(java.awt.Point location, String objectName) {
		Cell cell = getCell(location);
		setRedraw(cell);
		CellObject object = cell.removeObject(objectName);
		if (object == null) {
			return null;
		}
		
		if (object.updatable()) {
			updatables.remove(object);
			updatablesLocations.remove(object);
		}
		removalStateUpdate(object);
		
		return object;
	}
	
	public Player getPlayer(java.awt.Point location) {
		Cell cell = getCell(location);
		return cell.getPlayer();
	}
	
	public boolean hasObject(java.awt.Point location, String name) {
		Cell cell = getCell(location);
		return cell.hasObject(name);
	}
	
	public CellObject getObject(java.awt.Point location, String name) {
		Cell cell = getCell(location);
		return cell.getObject(name);
	}
	
	public void updateObjects(World world) {
		if (!updatables.isEmpty()) {
			Iterator<CellObject> iter = updatables.iterator();
			
			ArrayList<java.awt.Point> explosions = new ArrayList<java.awt.Point>();
			while (iter.hasNext()) {
				CellObject cellObject = iter.next();
				java.awt.Point location = updatablesLocations.get(cellObject);
				assert location != null;
				int previousScore = 0;
				if (Soar2D.config.eaters) {
					if (cellObject.hasProperty(Names.kPropertyPoints)) {
						previousScore = cellObject.getIntProperty(Names.kPropertyPoints);
					}
				}
				if (cellObject.update(world, location)) {
					Cell cell = getCell(location);
					
					cellObject = cell.removeObject(cellObject.getName());
					assert cellObject != null;
					
					setRedraw(cell);
					
					// if it is not tanksoar or if the cell is not a missle or if shouldRemoveMissile returns true
					if (!Soar2D.config.tanksoar || !cellObject.hasProperty(Names.kPropertyMissile) 
							|| shouldRemoveMissile(world, location, cell, cellObject)) {
						
						// we need an explosion if it was a tanksoar missile
						if (Soar2D.config.tanksoar && cellObject.hasProperty(Names.kPropertyMissile)) {
							explosions.add(location);
						}
						iter.remove();
						updatablesLocations.remove(cellObject);
						removalStateUpdate(cellObject);
					}
				}
				if (Soar2D.config.eaters) {
					if (cellObject.hasProperty(Names.kPropertyPoints)) {
						scoreCount += cellObject.getIntProperty(Names.kPropertyPoints) - previousScore;
					}
				}
			}
			
			Iterator<java.awt.Point> explosion = explosions.iterator();
			while (explosion.hasNext()) {
				setExplosion(explosion.next());
			}
		}
		
		if (Soar2D.config.terminalUnopenedBoxes) {
			Iterator<CellObject> iter = unopenedBoxes.iterator();
			while (iter.hasNext()) {
				CellObject box = iter.next();
				if (!isUnopenedBox(box)) {
					Soar2D.logger.info("Removed box from unopened list.");
					iter.remove();
				}
			}
		}
	}
	
	private boolean shouldRemoveMissile(World world, java.awt.Point location, Cell cell, CellObject missile) {
		// instead of removing missiles, move them

		// what direction is it going
		int missileDir = missile.getIntProperty(Names.kPropertyDirection);
		
		while (true) {
			// move it
			Direction.translate(location, missileDir);
			
			// check destination
			cell = getCell(location);
			
			if (!cell.enterable()) {
				// missile is destroyed
				world.destroyMissile(missile.getName());
				return true;
			}
			
			Player player = cell.getPlayer();
			
			if (player != null) {
				world.missileHit(player, location, missile);
				// missile is destroyed
				world.destroyMissile(missile.getName());
				return true;
			}
	
			// missile didn't hit anything
			
			// if the missile is not in phase 2, return
			if (missile.getIntProperty(Names.kPropertyFlyPhase) != 2) {
				cell.addCellObject(missile);
				updatablesLocations.put(missile, location);
				return false;
			}
			
			// we are in phase 2, call update again, this will move us out of phase 2 to phase 3
			missile.update(world, location);
		}
	}
		
	private void removalStateUpdate(CellObject object) {
		if (Soar2D.config.tanksoar) {
			if (object.hasProperty(Names.kPropertyCharger)) {
				if (health && object.hasProperty(Names.kPropertyHealth)) {
					health = false;
				}
				if (energy && object.hasProperty(Names.kPropertyEnergy)) {
					energy = false;
				}
			}
			if (object.hasProperty(Names.kPropertyMissiles)) {
				missilePacks -= 1;
			}
		} else if (Soar2D.config.eaters) {
			if (object.hasProperty(Names.kPropertyEdible)) {
				foodCount -= 1;
			}
			if (object.hasProperty(Names.kPropertyPoints)) {
				scoreCount -= object.getIntProperty(Names.kPropertyPoints);
			}
		}
		if (Soar2D.config.terminalUnopenedBoxes) {
			if (isUnopenedBox(object)) {
				Soar2D.logger.info("Removed box from unopened list.");
				unopenedBoxes.remove(object);
			}
		}
	}
	
	public ArrayList<CellObject> getAllWithProperty(java.awt.Point location, String name) {
		Cell cell = getCell(location);
		return cell.getAllWithProperty(name);
	}
	
	public void removeAllWithProperty(java.awt.Point location, String name) {
		Cell cell = getCell(location);
		
		cell.iter = cell.cellObjects.values().iterator();
		CellObject cellObject;
		while (cell.iter.hasNext()) {
			cellObject = cell.iter.next();
			if (cellObject.hasProperty(name)) {
				if (cellObject.updatable()) {
					updatables.remove(cellObject);
					updatablesLocations.remove(cellObject);
				}
				cell.iter.remove();
				removalStateUpdate(cellObject);
			}
		}
	}
	
	private boolean isUnopenedBox(CellObject object) {
		if (object.hasProperty(Names.kPropertyBox)) {
			String status = object.getProperty(Names.kPropertyStatus);
			if (status == null || !status.equals(Names.kOpen)) {
				return true;
			}
		}
		return false;
	}
	
	public void addObjectToCell(java.awt.Point location, CellObject object) {
		Cell cell = getCell(location);
		if (cell.hasObject(object.getName())) {
			CellObject old = cell.removeObject(object.getName());
			assert old != null;
			updatables.remove(old);
			updatablesLocations.remove(old);
			removalStateUpdate(old);
		}
		if (object.updatable()) {
			updatables.add(object);
			updatablesLocations.put(object, location);
		}
		if (Soar2D.config.terminalUnopenedBoxes) {
			if (isUnopenedBox(object)) {
				Soar2D.logger.info("Added box to unopened list.");
				unopenedBoxes.add(object);
			}
		}
		if (Soar2D.config.tanksoar) {
			if (object.hasProperty(Names.kPropertyCharger)) {
				if (!health && object.hasProperty(Names.kPropertyHealth)) {
					health = true;
				}
				if (!energy && object.hasProperty(Names.kPropertyEnergy)) {
					energy = true;
				}
			}
			if (object.hasProperty(Names.kPropertyMissiles)) {
				missilePacks += 1;
			}
		} else if (Soar2D.config.eaters) {
			if (object.hasProperty(Names.kPropertyEdible)) {
				foodCount += 1;
			}
			if (object.hasProperty(Names.kPropertyPoints)) {
				scoreCount += object.getIntProperty(Names.kPropertyPoints);
			}
		}
		cell.addCellObject(object);
		setRedraw(cell);
	}
	
	private void setRedraw(Cell cell) {
		cell.addCellObject(new CellObject(Names.kRedraw, false));
	}
	
	public void setExplosion(java.awt.Point location) {
		CellObject explosion = null;
		if (Soar2D.config.tanksoar) {
			explosion = cellObjectManager.createObject(Names.kExplosion);
		} else {
			explosion = new CellObject(Names.kExplosion, true);
			explosion.addProperty(Names.kPropertyLinger, "2");
			explosion.setLingerUpdate(true);
		}
		addObjectToCell(location, explosion);
	}
	
	public void shutdown() {
		mapCells = null;
		cellObjectManager = null;
		size = 0;
	}

	public int getRadar(RadarCell[][] radar, java.awt.Point location, int facing, int radarPower) {
		assert radar.length == 3;

		int distance = 0;
		
		distance = radarProbe(radar, location, facing, distance, radarPower);
		
		return distance;
	}
	
	private RadarCell getRadarCell(java.awt.Point location) {
		// TODO: cache these each frame!!
		
		Cell cell;
		Iterator<CellObject> iter;
		RadarCell radarCell;

		cell = getCell(location);
		radarCell = new RadarCell();
		radarCell.player = cell.getPlayer();
		if (cell.enterable()) {
			iter = cell.getAllWithProperty(Names.kPropertyMiniImage).iterator();
			while (iter.hasNext()) {
				CellObject object = iter.next();
				if (object.getName().equals(Names.kEnergy)) {
					radarCell.energy = true;
				} else if (object.getName().equals(Names.kHealth)) {
					radarCell.health = true;
				} else if (object.getName().equals(Names.kMissiles)) {
					radarCell.missiles = true;
				} 
			}
		} else {
			radarCell.obstacle = true;
		}
		return radarCell;
	}
	
	private int radarProbe(RadarCell[][] radar, java.awt.Point myLocation, int facing, int distance, int maxDistance) {
		assert maxDistance < radar[1].length;
		assert distance >= 0;
		assert distance + 1 < radar[1].length;
		assert distance < maxDistance;
		assert facing > 0;
		assert facing < 5;
		
		java.awt.Point location;
		
		location = new java.awt.Point(myLocation);
		Direction.translate(location, Direction.leftOf[facing]);
		radar[0][distance] = getRadarCell(location);
		if (radar[0][distance].player != null) {
			radar[0][distance].player.radarTouch(Direction.backwardOf[facing]);
		}
		
		location = new java.awt.Point(myLocation);
		Direction.translate(location, Direction.rightOf[facing]);
		radar[2][distance] = getRadarCell(location);
		if (radar[2][distance].player != null) {
			radar[2][distance].player.radarTouch(Direction.backwardOf[facing]);
		}

		distance += 1;

		location = new java.awt.Point(myLocation);
		Direction.translate(location, facing);
		radar[1][distance] = getRadarCell(location);
		if (radar[1][distance].player != null) {
			radar[1][distance].player.radarTouch(Direction.backwardOf[facing]);
		}

		if (distance == maxDistance) {
			return distance;
		}
		
		boolean enterable = radar[1][distance].obstacle == false;
		boolean noPlayer = radar[1][distance].player == null;
		
		if (enterable && noPlayer) {
			return radarProbe(radar, location, facing, distance, maxDistance);
		}
		return distance;
	}

	public int getBlocked(Point location) {
		Cell cell;
		int blocked = 0;
		
		cell = getCell(location.x+1, location.y);
		if (!cell.enterable() || cell.getPlayer() != null) {
			blocked |= Direction.kEastIndicator;
		}
		cell = getCell(location.x-1, location.y);
		if (!cell.enterable() || cell.getPlayer() != null) {
			blocked |= Direction.kWestIndicator;
		}
		cell = getCell(location.x, location.y+1);
		if (!cell.enterable() || cell.getPlayer() != null) {
			blocked |= Direction.kSouthIndicator;
		}
		cell = getCell(location.x, location.y-1);
		if (!cell.enterable() || cell.getPlayer() != null) {
			blocked |= Direction.kNorthIndicator;
		}
		return blocked;
	}
	
	public int getSoundNear(java.awt.Point location) {
		if (Soar2D.simulation.world.getPlayers().size() < 2) {
			return 0;
		}
		
		// Set all cells unexplored.
		for(int y = 1; y < mapCells.length - 1; ++y) {
			for (int x = 1; x < mapCells.length - 1; ++x) {
				mapCells[y][x].distance = -1;
			}
		}
		
		LinkedList<java.awt.Point> searchList = new LinkedList<java.awt.Point>();
		searchList.addLast(new java.awt.Point(location));
		int distance = 0;
		getCell(location).distance = distance;
		getCell(location).parent = null;

		int relativeDirection = -1;
		int newCellX = 0;
		int newCellY = 0;
		java.awt.Point parentLocation;
		Cell parentCell;
		Cell newCell;

		while (searchList.size() > 0) {
			parentLocation = searchList.getFirst();
			searchList.removeFirst();
			parentCell = getCell(parentLocation);
			distance = parentCell.distance;
			if (distance >= Soar2D.config.kMaxSmellDistance) {
				//System.out.println(parentCell + " too far");
				continue;
			}

			// Explore cell.
			for (int i = 1; i < 5; ++i) {
				newCellX = parentLocation.x;
				newCellY = parentLocation.y;
				newCellX += Direction.xDelta[i];
				newCellY += Direction.yDelta[i];

				if (!isInBounds(newCellX, newCellY)) {
					continue;
				}

				newCell = getCell(newCellX, newCellY);
				if (!newCell.enterable()) {
					//System.out.println(parentCell + " not enterable");
					continue;
				}
							
				if (newCell.distance >= 0) {
					//System.out.println(parentCell + " already explored");
					continue;
				}
				newCell.distance = distance + 1;
				
				Player targetPlayer = newCell.getPlayer();
				if ((targetPlayer != null) && Soar2D.simulation.world.recentlyMovedOrRotated(targetPlayer)) {
					// I'm its parent, so see if I'm the top here
					while(parentCell.parent != null) {
						// the new cell becomes me
						newCellX = parentLocation.x;
						newCellY = parentLocation.y;
						
						// I become my parent
						parentLocation = getCell(parentLocation).parent;
						parentCell = getCell(parentLocation);
					}
					// location is now the top of the list, compare
					// to find the direction to the new cell
					if (newCellX < parentLocation.x) {
						relativeDirection = Direction.kWestIndicator;
					} else if (newCellX > parentLocation.x) {
						relativeDirection = Direction.kEastIndicator;
					} else if (newCellY < parentLocation.y) {
						relativeDirection = Direction.kNorthIndicator;
					} else if (newCellY > parentLocation.y) {
						relativeDirection = Direction.kSouthIndicator;
					} else {
						assert false;
						relativeDirection = 0;
					}
					break;
				}
				
				if (relativeDirection != -1) {
					break;
				}
				
				// add me as the new cell's parent				
				newCell.parent = parentLocation;
				// add the new cell to the search list
				searchList.addLast(new java.awt.Point(newCellX, newCellY));
			}
			
			if (relativeDirection != -1) {
				break;
			}
		}
		
		//if (logger.isLoggable(Level.FINEST)) logger.finest("Finished search.");
		
		if (relativeDirection == -1) {
			relativeDirection = 0;
		}
		return relativeDirection;
	}

	public boolean isInBounds(int x, int y) {
		return (x >= 0) && (y >= 0) && (x < getSize()) && (y < getSize());
	}

	public boolean isInBounds(Point location) {
		return isInBounds(location.x, location.y);
	}
}
