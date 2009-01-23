package soar2d.map;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;

import org.jdom.Element;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Soar2D;
import soar2d.players.Player;
import soar2d.world.TankSoarWorld;

public class TankSoarMap extends GridMap {
	
	public TankSoarMap() {
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
	protected void cell(Element cell, int [] location) throws LoadError {
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
	protected boolean object(Element object, int [] location) throws LoadError {
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
	public boolean isAvailable(int [] location) {
		Cell cell = getCell(location);
		boolean enterable = !cell.hasAnyWithProperty(Names.kPropertyBlock);
		boolean noPlayer = cell.getPlayer() == null;
		boolean noMissilePack = !cell.hasAnyWithProperty(Names.kPropertyMissiles);
		boolean noCharger = !cell.hasAnyWithProperty(Names.kPropertyCharger);
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
	public void setExplosion(int [] location) {
		addObjectToCell(location, cellObjectManager.createObject(Names.kExplosion));
	}

	private static class MissileData {
		MissileData(int [] location, CellObject missile) {
			this.location = location;
			this.missile = missile;
		}
		int [] location;
		CellObject missile;
	}
	@Override
	public void updateObjects(TankSoarWorld tsWorld) {
		HashSet<CellObject> copy = new HashSet<CellObject>(updatables);
		ArrayList<int []> explosions = new ArrayList<int []>();
		ArrayList<MissileData> newMissiles = new ArrayList<MissileData>();
		for (CellObject cellObject : copy) {
			int [] location = Arrays.copyOf(updatablesLocations.get(cellObject), updatablesLocations.get(cellObject).length);
			
			if (cellObject.update(location)) {

				// Remove it from the cell
				removalStateUpdate(getCell(location).removeObject(cellObject.getName()));

				// Missiles fly, handle that
				if (cellObject.hasProperty(Names.kPropertyMissile)) {
					
					// |*  | * |  *|  <|>  |*  | * |  *|  <|>  |
					//  0    1    2    3    0    1    2    3
					// phase 3 threatens two squares
					// we're in phase 3 when detected in phase 2
	
					// what direction is it going
					Direction missileDir = Direction.parse(cellObject.getProperty(Names.kPropertyDirection));
					
					while (true) {
						int phase = cellObject.getIntProperty(Names.kPropertyFlyPhase);
						
						if (phase == 0) {
							Cell overlapCell = getCell(location);
							overlapCell = overlapCell.neighbors[missileDir.backward().index()];
							overlapCell.forceRedraw();
						}
						
						// move it
						Direction.translate(location, missileDir);
						
						// check destination
						Cell cell = getCell(location);
						
						if (cell.hasAnyWithProperty(Names.kPropertyBlock)) {
							// missile is destroyed
							explosions.add(location);
							break;
						}
						
						Player player = cell.getPlayer();
						
						if (player != null) {
							// missile is destroyed
							tsWorld.missileHit(player, this, location, cellObject, Soar2D.simulation.world.getPlayers());
							explosions.add(location);
							break;
						}
				
						// missile didn't hit anything
						
						// if the missile is not in phase 2, return
						if (phase != 2) {
							newMissiles.add(new MissileData(location, cellObject));
							break;
						}
						
						// we are in phase 2, call update again, this will move us out of phase 2 to phase 3
						cellObject.update(location);
					}
				}
			}
		}

		for (int[] location : explosions) {
			setExplosion(location);
		}
		for (MissileData data : newMissiles) {
			addObjectToCell(data.location, data.missile);
		}
	}
	
	@Override
	void addStateUpdate(int [] location, CellObject added) {
		super.addStateUpdate(location, added);
		// Update state we keep track of specific to game type
		if (added.hasProperty(Names.kPropertyCharger)) {
			if (!health && added.hasProperty(Names.kPropertyHealth)) {
				health = true;
			}
			if (!energy && added.hasProperty(Names.kPropertyEnergy)) {
				energy = true;
			}
		}
		if (added.hasProperty(Names.kPropertyMissiles)) {
			missilePacks += 1;
		}
	}
	@Override
	void removalStateUpdate(CellObject removed) {
		super.removalStateUpdate(removed);
		if (removed.hasProperty(Names.kPropertyCharger)) {
			if (health && removed.hasProperty(Names.kPropertyHealth)) {
				health = false;
			}
			if (energy && removed.hasProperty(Names.kPropertyEnergy)) {
				energy = false;
			}
		}
		if (removed.hasProperty(Names.kPropertyMissiles)) {
			missilePacks -= 1;
		}
	}
}
