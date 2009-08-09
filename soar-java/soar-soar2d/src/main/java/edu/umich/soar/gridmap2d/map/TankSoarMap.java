package edu.umich.soar.gridmap2d.map;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.Set;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.RadarCell;
import edu.umich.soar.gridmap2d.players.Tank;
import edu.umich.soar.gridmap2d.players.TankState;
import edu.umich.soar.gridmap2d.world.PlayersManager;
import edu.umich.soar.gridmap2d.world.TankSoarWorld;

public class TankSoarMap extends GridMapBase implements GridMap, CellObjectObserver {
	private static Logger logger = Logger.getLogger(TankSoarMap.class);
	public static TankSoarMap generateInstance(String mapPath, int maxSoundDistance) {
		return new TankSoarMap(mapPath, maxSoundDistance);
	}

	SearchData[][] searchData;
	boolean energy;
	int missilePacks;
	boolean health;
	int maxSoundDistance;
	boolean usingMissilePacks;
	
	private TankSoarMap(String mapPath, int maxSoundDistance) {
		super(mapPath);
		this.maxSoundDistance = maxSoundDistance;
		
		reset();
	}

	@Override
	public void reset() {
		energy = false;
		missilePacks = 0;
		health = false;
		super.reload();
		usingMissilePacks = getData().cellObjectManager.hasTemplate("missiles");
		
		// Add ground to cells that don't have a background.
		int size = getData().cells.size();
		int [] xy = new int[2];
		for (xy[0] = 0; xy[0] < size; ++xy[0]) {
			for (xy[1] = 0; xy[1] < size; ++xy[1]) {
				Cell cell = getData().cells.getCell(xy);
				if (!cellHasBackground(cell)) {
					// add ground
					CellObject cellObject = getData().cellObjectManager.createObject(Names.kGround);
					addObject(xy, cellObject);
				}
			}
		}
		
		searchData = SearchData.newMap(getData().cells);
	}
	
	private boolean cellHasBackground(Cell cell) {
		for (CellObject cellObject : cell.getAllObjects()) {
			if (cellObject.hasProperty(Names.kGround)
					|| cellObject.hasProperty(Names.kPropertyBlock)
					|| cellObject.hasProperty(Names.kPropertyCharger)) {
				return true;
			}
		}
		return false;
	}

	@Override
	public boolean isAvailable(int[] xy) {
		Cell cell = getData().cells.getCell(xy);
		boolean enterable = !cell.hasAnyObjectWithProperty(Names.kPropertyBlock);
		boolean noPlayer = !cell.hasPlayers();
		boolean noMissilePack = !cell.hasAnyObjectWithProperty("missiles");
		boolean noCharger = !cell.hasAnyObjectWithProperty(Names.kPropertyCharger);
		return enterable && noPlayer && noMissilePack && noCharger;
	}
	
	@Override
	public void addStateUpdate(CellObject added) {
		// Update state we keep track of specific to game type
		if (added.hasProperty(Names.kPropertyCharger)) {
			if (!health && added.hasProperty(Names.kPropertyHealth)) {
				health = true;
			}
			if (!energy && added.hasProperty(Names.kPropertyEnergy)) {
				energy = true;
			}
		}
		if (added.hasProperty("missiles")) {
			missilePacks += 1;
		}
	}

	@Override
	public void removalStateUpdate(CellObject removed) {
		if (removed.hasProperty(Names.kPropertyCharger)) {
			if (health && removed.hasProperty(Names.kPropertyHealth)) {
				health = false;
			}
			if (energy && removed.hasProperty(Names.kPropertyEnergy)) {
				energy = false;
			}
		}
		if (removed.hasProperty("missiles")) {
			missilePacks -= 1;
		}
	}
	
	public boolean usingMissilePacks() {
		return usingMissilePacks;
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
		
	private static class MissileData {
		MissileData(int[] xy, CellObject missile) {
			this.xy = Arrays.copyOf(xy, xy.length);
			this.missile = missile;
		}
		int[] xy;
		CellObject missile;
	}
	
	public void updateObjects(TankSoarWorld tsWorld) {
		Set<CellObject> copy = new HashSet<CellObject>(getData().updatables);
		List<int []> explosions = new ArrayList<int []>();
		List<MissileData> newMissiles = new ArrayList<MissileData>();
		for (CellObject cellObject : copy) {
			int [] xy = cellObject.getLocation();
			Cell cell = getData().cells.getCell(xy);

			if (cellObject.hasProperty("update.fly-missile")) {
				// Remove it from the cell
				cell.removeObject(cellObject);

				// what direction is it going
				Direction missileDir = Direction.parse(cellObject.getProperty(Names.kPropertyDirection));

				int phase = cellObject.getIntProperty("update.fly-missile", 0);

				while (true) {
					// increment its phase
					phase += 1;
					phase %= 4;
					
					// |*  | * |  *|  <|>  |*  | * |  *|  <|>  |
					//  0    1    2    3    0    1    2    3
					// phase 3 threatens two squares
					// we're in phase 3 when detected in phase 2

					if (phase == 0) {
						SearchData overlapCell = SearchData.getCell(searchData, xy);
						overlapCell = overlapCell.getNeighbor(missileDir.backward());
						overlapCell.getCell().forceRedraw();
					}
					
					// move it
					Direction.translate(xy, missileDir);
					
					logger.trace("Flying missile " + cellObject.getProperty("owner") + "-" + cellObject.getProperty("missile-id") + " entering " + Arrays.toString(xy));
					
					// check destination
					cell = getData().cells.getCell(xy);
					
					if (cell.hasAnyObjectWithProperty(Names.kPropertyBlock)) {
						// missile is destroyed
						explosions.add(xy);
						phase = -1; // don't reset phase on object
						break;
					}
					
					Tank tank = (Tank)cell.getFirstPlayer();
					
					if (tank != null) {
						// missile is destroyed
						tsWorld.missileHit(tank, cellObject);
						explosions.add(xy);
						phase = -1; // don't reset phase on object
						break;
					}
			
					// missile didn't hit anything
					
					// if the missile is not in phase 2, done
					if (phase != 2) {
						newMissiles.add(new MissileData(xy, cellObject));
						break;
					}
					
					// we are in phase 2, update again, this will move us out of phase 2 to phase 3
				}
							
				// done flying, update phase unless destroyed
				if (phase >= 0) {
					cellObject.setIntProperty("update.fly-missile", phase);
				}
			}

			lingerUpdate(cellObject, cell);
		}

		for (int[] location : explosions) {
			tsWorld.setExplosion(location);
		}
		
		for (MissileData mdata : newMissiles) {
			addObject(mdata.xy, mdata.missile);
		}
	}

	@Override
	public CellObject createObjectByName(String name) {
		return getData().cellObjectManager.createObject(name);
	}

	@Override
	public List<CellObject> getTemplatesWithProperty(String name) {
		return getData().cellObjectManager.getTemplatesWithProperty(name);
	}

	public Direction getSoundNear(Tank tank, PlayersManager<Tank> players) {
		if (players.numberOfPlayers() < 2) {
			return Direction.NONE;
		}
		
		// Set all cells unexplored.
		int[] xy = new int[2];
		for (xy[0] = 0; xy[0] < getData().cells.size(); ++xy[0]) {
			for (xy[1] = 0; xy[1] < getData().cells.size(); ++xy[1]) {
				SearchData.getCell(searchData, xy).setExplored(false);
			}
		}
		
		Queue<SearchData> searchList = new LinkedList<SearchData>();
		{
			SearchData start = SearchData.getCell(searchData, players.getLocation(tank));
			start.setExplored(true);
			start.setDistance(0);
			start.setParent(null);
			searchList.add(start);
		}
		
		Direction finalDirection = Direction.NONE;
		
		while (searchList.size() > 0) {
			SearchData parentCell = searchList.poll();

			if (logger.isTraceEnabled()) {
				logger.trace("Sound: new parent " + parentCell);
			}
			
			// subtract 1 because we add one later (exploring neighbors)
			if (parentCell.getDistance() >= maxSoundDistance) {
				if (logger.isTraceEnabled()) {
					logger.trace("Sound: parent distance " + parentCell.getDistance() + " is too far");
				}
				continue;
			}

			// Explore cell.
			for (Direction exploreDir : Direction.values()) {
				if (exploreDir == Direction.NONE) {
					continue;
				}
				
				SearchData neighbor = parentCell.getNeighbor(exploreDir);
				if (neighbor == null) {
					continue;
				}

				if (neighbor.isExplored()) {
					continue;
				}

				if (logger.isTraceEnabled()) {
					logger.trace("Sound: exploring " + neighbor);
				}
				neighbor.setExplored(true);
				
				if (neighbor.getCell().hasAnyObjectWithProperty(Names.kPropertyBlock)) {
					logger.trace("Sound: blocked");
					continue;
				}
							
				neighbor.setDistance(parentCell.getDistance() + 1);
				
				if (logger.isTraceEnabled()) {
					logger.trace("Sound: distance " + neighbor.getDistance());
				}
				
				Tank targetPlayer = (Tank)neighbor.getCell().getFirstPlayer();
				if ((targetPlayer != null) && recentlyMovedOrRotated(targetPlayer, players)) {
					if (logger.isTraceEnabled()) {
						logger.trace("Sound: found recently moved player " + targetPlayer.getName());
					}
					
					// found a sound! walk home
					// I'm its parent, so see if I'm the top here
					while(parentCell.getParent() != null) {
						// the new cell becomes me
						neighbor = parentCell;
						
						// I become my parent
						parentCell = parentCell.getParent();
					}

					// Find direction to new sound
					boolean found = false;
					for (Direction dir : Direction.values()) {
						if (dir == Direction.NONE) {
							continue;
						}
						if (neighbor == parentCell.getNeighbor(dir)) {
							finalDirection = dir;
							found = true;
							break;
						}
					}
					
					// shouldn't happen
					if (found) {
						if (logger.isTraceEnabled()) {
							logger.trace("Sound: done, originated from " + finalDirection.id());
						}
					} else {
						// didn't find direction to new sound
						logger.trace("Sound: error: didn't find direction to sound!");
						assert false;
						finalDirection = Direction.NONE;
					}
					
				}
				
				// end condition: this is not Direction.NONE if we found someone
				if (finalDirection != Direction.NONE) {
					break;
				}

				neighbor.setParent(parentCell);
				
				// add the new cell to the search list
				searchList.add(neighbor);
			}
			
			// end condition: this is not Direction.NONE if we found someone
			if (finalDirection != Direction.NONE) {
				break;
			}
		}
		return finalDirection;
	}

	private boolean recentlyMovedOrRotated(Tank tank, PlayersManager<Tank> players) {
		CommandInfo command = players.getCommand(tank);
		return command != null && (command.move || command.rotate);
	}

	public CellObject createRandomObjectWithProperty(String property) {
		return getData().cellObjectManager.createRandomObjectWithProperty(property);
	}

	public void handleIncoming() {
		// note: a couple of optimizations possible here
		// like marking cells that have been checked, depends on direction though
		// probably more work than it is worth as this should only be slow when there are
		// a ton of missiles flying
		
		for (CellObject missile : getData().updatables) {
			if (!missile.hasProperty(Names.kPropertyMissile)) {
				continue;
			}
	
			SearchData threatenedCell = SearchData.getCell(searchData, missile.getLocation());
			Direction direction = Direction.parse(missile.getProperty(Names.kPropertyDirection));
			while (true) {
				threatenedCell = threatenedCell.getNeighbor(direction);
				
				// stops at wall
				if (threatenedCell.getCell().hasAnyObjectWithProperty(Names.kPropertyBlock)) {
					break;
				}
				
				Tank tank = (Tank)threatenedCell.getCell().getFirstPlayer();
				if (tank != null) {
					TankState state = tank.getState();
					state.setIncoming(direction.backward());
					break;
				}
			}
		}
	}

	public CellObject createRandomObjectWithProperties(String p1, String p2) {
		return getData().cellObjectManager.createRandomObjectWithProperties(p1, p2);
	}

	public int getRadar(RadarCell[][] radar, int[] newLocation, Direction facing, int radarPower) {
		if (radarPower == 0) {
			return 0;
		}
		
		assert radar.length == 3;

		int distance = 0;
		
		distance = radarProbe(radar, newLocation, facing, distance, radarPower);
		
		return distance;
	}

	private int radarProbe(RadarCell[][] radar, int [] myLocation, Direction facing, int distance, int maxDistance) {
		assert maxDistance < radar[1].length;
		assert distance >= 0;
		assert distance + 1 < radar[1].length;
		assert distance < maxDistance;
		assert facing != Direction.NONE;
		
		int [] location;
		
		location = Arrays.copyOf(myLocation, myLocation.length);
		Direction.translate(location, facing.left());
		radar[0][distance] = getRadarCell(location);
		if (radar[0][distance].player != null) {
			Tank tank = radar[0][distance].player;
			
			if (distance != 0) {
				tank.getState().radarTouch(facing.backward());
			} else {
				tank.getState().radarTouch(facing.right());
			}
		}
		
		location = Arrays.copyOf(myLocation, myLocation.length);
		Direction.translate(location, facing.right());
		radar[2][distance] = getRadarCell(location);
		if (radar[2][distance].player != null) {
			Tank tank = (Tank)radar[2][distance].player;
			
			if (distance != 0) {
				tank.getState().radarTouch(facing.backward());
			} else {
				tank.getState().radarTouch(facing.left());
			}
		}

		distance += 1;

		location = Arrays.copyOf(myLocation, myLocation.length);
		Direction.translate(location, facing);
		radar[1][distance] = getRadarCell(location);
		if (radar[1][distance].player != null) {
			Tank tank = radar[1][distance].player;
			tank.getState().radarTouch(facing.backward());
		}

		boolean enterable = radar[1][distance].obstacle == false;
		boolean noPlayer = radar[1][distance].player == null;
		
		if (enterable && noPlayer) {
			CellObject radarWaves = getData().cellObjectManager.createObject("radar-" + facing.id());
			radarWaves.setProperty(Names.kPropertyDirection, facing.id());
			logger.trace("Adding " + radarWaves.getProperty("name") + " to " + Arrays.toString(location));
			addObject(location, radarWaves);
		}

		if (distance == maxDistance) {
			return distance;
		}
		
		if (enterable && noPlayer) {
			return radarProbe(radar, location, facing, distance, maxDistance);
		}
		
		return distance;
	}

	private RadarCell getRadarCell(int [] xy) {
		// note: cache these each frame!!
		
		Cell cell;
		RadarCell radarCell;

		cell = getData().cells.getCell(xy);
		radarCell = new RadarCell();
		radarCell.player = (Tank)cell.getFirstPlayer();
		if (!cell.hasAnyObjectWithProperty(Names.kPropertyBlock)) {
			for (CellObject object : cell.getAllObjectsWithProperty(Names.kPropertyMiniImage)) {
				if (object.hasProperty("energy")) {
					radarCell.energy = true;
				} else if (object.hasProperty("health")) {
					radarCell.health = true;
				} else if (object.hasProperty("missiles")) {
					radarCell.missiles = true;
				} 
			}
		} else {
			radarCell.obstacle = true;
		}
		return radarCell;
	}
	
	public int getBlocked(int[] location) {
		Cell cell;
		int blocked = 0;
		
		cell = getData().cells.getCell(new int [] { location[0]+1, location[1] });
		if (cell.hasAnyObjectWithProperty(Names.kPropertyBlock) || cell.hasPlayers()) {
			blocked |= Direction.EAST.indicator();
		}
		cell = getData().cells.getCell(new int [] { location[0]-1, location[1] });
		if (cell.hasAnyObjectWithProperty(Names.kPropertyBlock) || cell.hasPlayers()) {
			blocked |= Direction.WEST.indicator();
		}
		cell = getData().cells.getCell(new int [] { location[0], location[1]+1 });
		if (cell.hasAnyObjectWithProperty(Names.kPropertyBlock) || cell.hasPlayers()) {
			blocked |= Direction.SOUTH.indicator();
		}
		cell = getData().cells.getCell(new int [] { location[0], location[1]-1 });
		if (cell.hasAnyObjectWithProperty(Names.kPropertyBlock) || cell.hasPlayers()) {
			blocked |= Direction.NORTH.indicator();
		}
		return blocked;
	}

}
