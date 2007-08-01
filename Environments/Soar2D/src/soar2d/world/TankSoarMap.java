package soar2d.world;

import java.awt.Point;
import java.util.ArrayList;
import java.util.Iterator;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Soar2D;
import soar2d.TankSoarWorld;
import soar2d.player.Player;

public class TankSoarMap implements IGridMap {

	public void postCell(boolean background, GridMap map, Point location) {
		if (!background) {
			// add ground
			CellObject cellObject = map.cellObjectManager.createObject(Names.kGround);
			map.addObjectToCell(location, cellObject);
		}
	}

	public void addObjectToCell(GridMap map, CellObject object) {
		if (object.hasProperty(Names.kPropertyCharger)) {
			if (!map.health && object.hasProperty(Names.kPropertyHealth)) {
				map.health = true;
			}
			if (!map.energy && object.hasProperty(Names.kPropertyEnergy)) {
				map.energy = true;
			}
		}
		if (object.hasProperty(Names.kPropertyMissiles)) {
			map.missilePacks += 1;
		}
	}

	public int getLocationId(GridMap gridMap, Point location) {
		return -1;
	}

	public boolean isAvailable(GridMap map, Point location) {
		Cell cell = map.getCell(location);
		boolean enterable = cell.enterable();
		boolean noPlayer = cell.getPlayer() == null;
		boolean noMissilePack = cell.getAllWithProperty(Names.kPropertyMissiles).size() <= 0;
		boolean noCharger = cell.getAllWithProperty(Names.kPropertyCharger).size() <= 0;
		return enterable && noPlayer && noMissilePack && noCharger;
	}

	public boolean objectIsBackground(CellObject cellObject) {
		if (cellObject.hasProperty(Names.kPropertyBlock) 
				|| (cellObject.getName() == Names.kGround)
				|| (cellObject.hasProperty(Names.kPropertyCharger))) {
			return true;
		}
		return false;
	}

	public void removalStateUpdate(GridMap map, CellObject object) {
		if (object.hasProperty(Names.kPropertyCharger)) {
			if (map.health && object.hasProperty(Names.kPropertyHealth)) {
				map.health = false;
			}
			if (map.energy && object.hasProperty(Names.kPropertyEnergy)) {
				map.energy = false;
			}
		}
		if (object.hasProperty(Names.kPropertyMissiles)) {
			map.missilePacks -= 1;
		}
		
	}

	public CellObject createExplosion(GridMap map) {
		return map.cellObjectManager.createObject(Names.kExplosion);
	}

	public void updateObjects(GridMap map, TankSoarWorld tsWorld) {
		if (!map.updatables.isEmpty()) {
			Iterator<CellObject> iter = map.updatables.iterator();
			
			ArrayList<java.awt.Point> explosions = new ArrayList<java.awt.Point>();
			while (iter.hasNext()) {
				CellObject cellObject = iter.next();
				java.awt.Point location = map.updatablesLocations.get(cellObject);
				assert location != null;
				
				if (cellObject.update(location)) {
					Cell cell = map.getCell(location);
					
					cellObject = cell.removeObject(cellObject.getName());
					assert cellObject != null;
					
					map.setRedraw(cell);
					
					// if the cell is not a missile or if shouldRemoveMissile returns true
					if (!cellObject.hasProperty(Names.kPropertyMissile) 
							|| shouldRemoveMissile(map, tsWorld, location, cell, cellObject)) {
						
						// we need an explosion if it was a missile
						if (cellObject.hasProperty(Names.kPropertyMissile)) {
							explosions.add(location);
						}
						iter.remove();
						map.updatablesLocations.remove(cellObject);
						map.removalStateUpdate(cellObject);
					}
				}
			}
			
			Iterator<java.awt.Point> explosion = explosions.iterator();
			while (explosion.hasNext()) {
				map.setExplosion(explosion.next());
			}
		}
	}
	
	private boolean shouldRemoveMissile(GridMap map, TankSoarWorld tsWorld, java.awt.Point location, Cell cell, CellObject missile) {
		// instead of removing missiles, move them

		// what direction is it going
		int missileDir = missile.getIntProperty(Names.kPropertyDirection);
		
		while (true) {
			// move it
			Direction.translate(location, missileDir);
			
			// check destination
			cell = map.getCell(location);
			
			if (!cell.enterable()) {
				// missile is destroyed
				return true;
			}
			
			Player player = cell.getPlayer();
			
			if (player != null) {
				// missile is destroyed
				tsWorld.missileHit(player, map, location, missile, Soar2D.simulation.world.getPlayers());
				return true;
			}
	
			// missile didn't hit anything
			
			// if the missile is not in phase 2, return
			if (missile.getIntProperty(Names.kPropertyFlyPhase) != 2) {
				cell.addCellObject(missile);
				map.updatablesLocations.put(missile, location);
				return false;
			}
			
			// we are in phase 2, call update again, this will move us out of phase 2 to phase 3
			missile.update(location);
		}
	}
		

}
