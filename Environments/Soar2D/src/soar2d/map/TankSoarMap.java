package soar2d.map;

import java.awt.Point;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.jdom.Element;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Soar2D;
import soar2d.configuration.Configuration;
import soar2d.player.Player;
import soar2d.world.TankSoarWorld;

public class TankSoarMap extends GridMap {
	
	public TankSoarMap(Configuration config) {
		super(config);
	}

	int missilePacks = 0;	// returns the number of missile packs on the map
	public int numberMissilePacks() {
		return missilePacks;
	}
	
	boolean health = false;	// true if there is a health charger
	public boolean hasHealthCharger() {
		return health;
	}
	
	boolean energy = false;	// true if there is an energy charger
	public boolean hasEnergyCharger() {
		return energy;
	}
	
	@Override
	protected void cell(Element cell, java.awt.Point location) throws LoadError {
		boolean background = false;
		
		List<Element> children = (List<Element>)cell.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (!child.getName().equalsIgnoreCase(kTagObject)) {
				throw new LoadError("unrecognized tag: " + child.getName());
			}
			
			background = object(child, location);
		}
		
		if (!background) {
			// add ground
			CellObject cellObject = cellObjectManager.createObject(Names.kGround);
			addObjectToCell(location, cellObject);
		}
	}
	
	@Override
	protected boolean object(Element object, java.awt.Point location) throws LoadError {
		String name = object.getTextTrim();
		if (name.length() <= 0) {
			throw new LoadError("object doesn't have name");
		}
		
		if (!cellObjectManager.hasTemplate(name)) {
			throw new LoadError("object \"" + name + "\" does not map to a cell object");
		}
		
		CellObject cellObject = cellObjectManager.createObject(name);
		
		// false for all but tanksoar, tanksoar checks
		boolean background = objectIsBackground(cellObject);
		addObjectToCell(location, cellObject);

		if (cellObject.rewardInfoApply) {
			assert rewardInfoObject == null;
			rewardInfoObject = cellObject;
		}
		
		return background;
	}

	@Override
	public void addObjectToCell(Point location, CellObject object) {
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
		
		// Update state we keep track of specific to game type
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

		cell.addCellObject(object);
		setRedraw(cell);
	}

	@Override
	public boolean isAvailable(Point location) {
		Cell cell = getCell(location);
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

	@Override
	public void removalStateUpdate(CellObject object) {
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
	}

	@Override
	public void setExplosion(Point location) {
		addObjectToCell(location, cellObjectManager.createObject(Names.kExplosion));
	}

	@Override
	public void updateObjects(TankSoarWorld tsWorld) {
		if (!updatables.isEmpty()) {
			Iterator<CellObject> iter = updatables.iterator();
			
			ArrayList<java.awt.Point> explosions = new ArrayList<java.awt.Point>();
			while (iter.hasNext()) {
				CellObject cellObject = iter.next();
				java.awt.Point location = updatablesLocations.get(cellObject);
				assert location != null;
				
				if (cellObject.update(location)) {
					Cell cell = getCell(location);
					
					cellObject = cell.removeObject(cellObject.getName());
					assert cellObject != null;
					
					setRedraw(cell);
					
					// if the cell is not a missile or if shouldRemoveMissile returns true
					if (!cellObject.hasProperty(Names.kPropertyMissile) 
							|| shouldRemoveMissile(tsWorld, location, cell, cellObject)) {
						
						// we need an explosion if it was a missile
						if (cellObject.hasProperty(Names.kPropertyMissile)) {
							explosions.add(location);
						}
						iter.remove();
						updatablesLocations.remove(cellObject);
						removalStateUpdate(cellObject);
					}
				}
			}
			
			Iterator<java.awt.Point> explosion = explosions.iterator();
			while (explosion.hasNext()) {
				setExplosion(explosion.next());
			}
		}
	}
	
	private boolean shouldRemoveMissile(TankSoarWorld tsWorld, java.awt.Point location, Cell cell, CellObject missile) {
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
				return true;
			}
			
			Player player = cell.getPlayer();
			
			if (player != null) {
				// missile is destroyed
				tsWorld.missileHit(player, this, location, missile, Soar2D.simulation.world.getPlayers());
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
			missile.update(location);
		}
	}
		

}
