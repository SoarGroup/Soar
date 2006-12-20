package soar2d.world;

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
	IdentityHashMap<CellObject, java.awt.Point> updatablesLocations = new IdentityHashMap<CellObject, java.awt.Point>();
	
	public int getSize() {
		return size;
	}
	
	public int getScoreCount() {
		return scoreCount;
	}
	
	public int getFoodCount() {
		return foodCount;
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
		if (updatables.isEmpty()) {
			return;
		}
		
		Iterator<CellObject> iter = updatables.iterator();
		while (iter.hasNext()) {
			CellObject cellObject = iter.next();
			java.awt.Point location = updatablesLocations.get(cellObject);
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
	}
	
	private boolean shouldRemoveMissile(World world, java.awt.Point location, Cell cell, CellObject cellObject) {
		// instead of removing missiles, move them

		// what direction is it going
		int missileDir = cellObject.getIntProperty(Names.kPropertyDirection);
		
		while (true) {
			// move it
			Direction.translate(location, missileDir);
			
			// check destination
			cell = getCell(location);
			
			if (!cell.enterable()) {
				// missile is destroyed
				return true;
			}
			
			Player player = cell.getPlayer();
			
			if (player != null) {
				// player is hit
				cellObject.apply(player);
				
				// TODO: check for kills
		
				// missile is destroyed
				return true;
			}
	
			// missile didn't hit anything
			
			// if the missile is not in phase 2, return
			if (cellObject.getIntProperty(Names.kPropertyFlyPhase) != 2) {
				cell.addCellObject(cellObject);
				updatablesLocations.put(cellObject, location);
				return false;
			}
			
			// we are in phase 2, call update again, this will move us out of phase 2 to phase 3
			cellObject.update(world, location);
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
	
	public void addObjectToCell(java.awt.Point location, CellObject object) {
		if (object.updatable()) {
			updatables.add(object);
			updatablesLocations.put(object, location);
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
		Cell cell = getCell(location);
		cell.addCellObject(object);
		setRedraw(cell);
	}
	
	private void setRedraw(Cell cell) {
		cell.addCellObject(new CellObject(Names.kRedraw, false));
	}
	
	public void setExplosion(java.awt.Point location) {
		//addObjectToCell(location, new CellObject(Names.kExplosion, true));
	}
	
	public void shutdown() {
		mapCells = null;
		cellObjectManager = null;
		size = 0;
	}
}
