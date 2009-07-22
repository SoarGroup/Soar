package edu.umich.soar.gridmap2d.map;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Queue;
import java.util.Set;

import org.apache.log4j.Logger;

import edu.umich.soar.config.Config;
import edu.umich.soar.config.ConfigFile;
import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Simulation;
import edu.umich.soar.gridmap2d.map.RoomMap.RoomObjectInfo;
import edu.umich.soar.gridmap2d.world.RoomWorld;


public class GridMapUtil {
	private static Logger logger = Logger.getLogger(GridMapUtil.class);

	static void loadFromConfigFile(GridMapData data, String mapPath, CellObjectObserver observer) throws Exception {
		loadFromConfigFile(data, mapPath, observer, 0, 0);
	}
		
	static void loadFromConfigFile(GridMapData data, String mapPath, CellObjectObserver observer, double lowProbability, double highProbability) throws Exception {
		File mapFile = new File(mapPath);
		if (!mapFile.exists()) {
			throw new Exception("Map file doesn't exist: " + mapFile.getAbsolutePath());
		}

		data.cellObjectManager = new CellObjectManager();
		
		String mapFilePath = mapFile.getAbsolutePath();
		Config mapConfig = new Config(new ConfigFile(mapFilePath));
		
		String objectsFileString = mapConfig.getString("objects_file");
		String mapFileDirectory = mapFilePath.substring(0, mapFilePath.lastIndexOf(File.separatorChar) + 1);
		
		Config objectsConfig = new Config(new ConfigFile(mapFileDirectory + objectsFileString));
		for (String id : mapConfig.getStrings("objects")) {
			CellObject template = new CellObject(objectsConfig.getChild("objects." + id));
			data.cellObjectManager.registerTemplate(template);
		}
		
		cellsConfig(data, mapConfig.getChild("cells"), objectsConfig, observer, lowProbability, highProbability);
		
		if (mapConfig.hasKey("metadata")) {
			data.metadataFile = new File(mapConfig.getString("metadata"));
		}
		
		buildReferenceMap(data);
	}

	private static void cellsConfig(GridMapData data, Config cellsConfig, Config objectsConfig, CellObjectObserver observer, double lowProbability, double highProbability) throws Exception {
		data.cells = new GridMapCells(cellsConfig.requireInt("size"), new CellObjectObserver[] { data, observer });
		
		data.randomWalls = cellsConfig.getBoolean("random_walls", false);
		data.randomFood = cellsConfig.getBoolean("random_food", false);
		
		// Generate map unless both are true
		if (!data.randomWalls || !data.randomFood) {
			Config rows = cellsConfig.getChild("rows");
			int[] xy = new int[2];
			for (xy[1] = 0; xy[1] < data.cells.size(); ++xy[1]) {

				String[] cellStrings = rows.getStrings(Integer.toString(xy[1]));
				if (cellStrings.length != data.cells.size()) {
					throw new Exception("Not enough cells, row " + xy[1]);
				}
				
				for (xy[0] = 0; xy[0] < data.cells.size(); ++xy[0]) {
					String contents = cellStrings[xy[0]];
					for (String objectID : contents.split("-")) {
						String objectName = objectsConfig.getString("objects." + objectID + ".name");
						logger.trace(Arrays.toString(xy) + ": " + objectName);
						
						if (!data.cellObjectManager.hasTemplate(objectName)) {
							throw new Exception("object \"" + objectName + "\" does not map to a cell object");
						}
						
						CellObject cellObject = data.cellObjectManager.createObject(objectName);
						if (cellObject.getBooleanProperty("apply.reward-info", false)) {
							data.rewardInfoObject = cellObject;
						}
						data.cells.getCell(xy).addObject(cellObject);
					}
				}
			}
		}
		
		// override walls if necessary
		if (data.randomWalls) {
			generateRandomWalls(data, observer, lowProbability, highProbability);
		}
		
		// override food if necessary
		if (data.randomFood) {
			generateRandomFood(data, observer);
		}
		
		// pick positive box
		if (data.rewardInfoObject != null) {
			assert data.positiveRewardID == 0;
			data.positiveRewardID = Simulation.random.nextInt(data.cellObjectManager.rewardObjects.size());
			data.positiveRewardID += 1;
			logger.trace("reward-info.positive-id: " + data.positiveRewardID);
			data.rewardInfoObject.setIntProperty("apply.reward-info.positive-id", data.positiveRewardID);
			
			// assigning colors like this helps us see the task happen
			// colors[0] = info box (already assigned)
			// colors[1] = positive reward box
			// colors[2..n] = negative reward boxes
			int negativeColor = 2;
			for (CellObject aBox : data.cellObjectManager.rewardObjects) {
				if (aBox.getIntProperty("box-id", 0) == data.positiveRewardID) {
					aBox.setProperty("apply.reward.correct", "ignored");
					aBox.setProperty(edu.umich.soar.gridmap2d.Names.kPropertyColor, Gridmap2D.simulation.kColors[1]);
				} else {
					aBox.setProperty(edu.umich.soar.gridmap2d.Names.kPropertyColor, Gridmap2D.simulation.kColors[negativeColor]);
					negativeColor += 1;
					assert negativeColor < Gridmap2D.simulation.kColors.length;
				}
			}
		}
	}
	
	private static void buildReferenceMap(GridMapData data) {
		// Build cell reference map
		assert data.cells != null;
		int [] xy = new int[2];
		for (xy[0] = 0; xy[0] < data.cells.size(); ++xy[0]) {
			for (xy[1] = 0; xy[1] < data.cells.size(); ++xy[1]) {
				Cell cell = data.cells.getCell(xy);
				for (Direction dir : Direction.values()) {
					if (dir == Direction.NONE) {
						continue;
					}
					int[] neighborLoc = Direction.translate(cell.location, dir, new int[2]);
					if (data.cells.isInBounds(neighborLoc)) {
						Cell neighbor = data.cells.getCell(neighborLoc);
						cell.neighbors[dir.index()] = neighbor;
					}
				}
			}
		}
	}

	private static void generateRandomWalls(GridMapData data, CellObjectObserver observer, double lowProbability, double highProbability) throws Exception {
		if (!data.cellObjectManager.hasTemplatesWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyBlock)) {
			throw new Exception("tried to generate random walls with no blocking types");
		}
		
		assert data.cells != null;
		int size = data.cells.size();
		
		logger.trace("Confirming perimeter wall.");
		int[] xy = new int[2];
		for (xy[0] = 0; xy[0] < size; ++xy[0]) {
			for (xy[1] = 0; xy[1] < size; ++xy[1]) {
				if (xy[0] == 0 || xy[0] == size - 1 || xy[1] == 0 || xy[1] == size - 1) {
					if (!data.cells.getCell(xy).hasAnyWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyBlock)) {
						removeFoodAndAddWall(data, xy, observer);
					}
					continue;
				}
			}
		}

		logger.trace("Generating random walls.");
		for (xy[0] = 2; xy[0] < size - 3; ++xy[0]) {
			for (xy[1] = 2; xy[1] < size - 3; ++xy[1]) {

				if (noWallsOnCorners(data, xy)) {
					double probability = lowProbability;
					if (wallOnAnySide(data, xy)) {
						probability = highProbability;					
					}
					if (Simulation.random.nextDouble() < probability) {
						removeFoodAndAddWall(data, xy, observer);
					}
				}
			}
		}
	}
	
	private static void removeFoodAndAddWall(GridMapData data, int[] xy, CellObjectObserver observer) {
		logger.trace(Arrays.toString(xy) + ": Changing to wall.");
		Cell cell = data.cells.getCell(xy);
		if (cell == null) {
			cell = Cell.createCell(xy);
			cell.addObserver(data);
			cell.addObserver(observer);
		}
		if (cell.hasAnyWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyEdible)) {
			cell.removeAllByProperty(edu.umich.soar.gridmap2d.Names.kPropertyEdible);
		}
		CellObject wall = data.cellObjectManager.createRandomObjectWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyBlock);
		cell.addObject(wall);
	}
	
	private static boolean noWallsOnCorners(GridMapData data, int[] xy) {
		Cell cell;
		
		cell = data.cells.getCell(new int[] {xy[0] + 1, xy[1] + 1});
		if (cell != null && cell.hasAnyWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyBlock)) {
			return false;
		}
		cell = data.cells.getCell(new int[] {xy[0] - 1, xy[1] + 1});
		if (cell != null && cell.hasAnyWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyBlock)) {
			return false;
		}
		cell = data.cells.getCell(new int[] {xy[0] - 1, xy[1] - 1});
		if (cell != null && cell.hasAnyWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyBlock)) {
			return false;
		}
		cell = data.cells.getCell(new int[] {xy[0] + 1, xy[1] - 1});
		if (cell != null && cell.hasAnyWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyBlock)) {
			return false;
		}
		return true;
	}
	
	private static boolean wallOnAnySide(GridMapData data, int[] xy) {
		Cell cell;
		
		cell = data.cells.getCell(new int[] {xy[0] + 1, xy[1]});
		if (cell != null && cell.hasAnyWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyBlock)) {
			return true;
		}
		cell = data.cells.getCell(new int[] {xy[0], xy[1] + 1});
		if (cell != null && cell.hasAnyWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyBlock)) {
			return true;
		}
		cell = data.cells.getCell(new int[] {xy[0] - 1, xy[1]});
		if (cell != null && cell.hasAnyWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyBlock)) {
			return true;
		}
		cell = data.cells.getCell(new int[] {xy[0], xy[1] - 1});
		if (cell != null && cell.hasAnyWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyBlock)) {
			return true;
		}
		return false;
	}
	
	private static void generateRandomFood(GridMapData data, CellObjectObserver observer) throws Exception {
		if (!data.cellObjectManager.hasTemplatesWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyEdible)) {
			throw new Exception("tried to generate random walls with no food types");
		}

		logger.trace("Generating random food.");
		
		int[] xy = new int[2];
		for (xy[0] = 1; xy[0] < data.cells.size() - 1; ++xy[0]) {
			for (xy[1] = 1; xy[1] < data.cells.size() - 1; ++xy[1]) {
				Cell cell = data.cells.getCell(xy);
				if (cell == null) {
					cell = Cell.createCell(xy);
					cell.addObserver(data);
					cell.addObserver(observer);
				}
				
				if (!cell.hasAnyWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyBlock)) {
					logger.trace(Arrays.toString(xy) + "Adding random food.");
					cell.removeAllByProperty(edu.umich.soar.gridmap2d.Names.kPropertyEdible);
					CellObject wall = data.cellObjectManager.createRandomObjectWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyEdible);
					cell.addObject(wall);
				}
			}
		}		
	}
	
	static int[] getAvailableLocationAmortized(GridMap map) {
		int size = map.size();
		
		// Loop in case there are no free spots, the 100 is totally arbitrary
		int [] xy = new int [2];
		for (int counter = 0; counter < 100; ++counter) {
			xy[0] = Simulation.random.nextInt(size - 2) + 1;
			xy[1] = Simulation.random.nextInt(size - 2) + 1;
			
			if (map.isAvailable(xy)) {
				return xy;
			}
		}
		List<int []> locations = new ArrayList<int []>();
		for (xy[0] = 0; xy[0] < size; ++xy[0]) {
			for (xy[1] = 0; xy[1] < size; ++ xy[1]) {
				if (map.isAvailable(xy)) {
					locations.add(xy);
				}
			}
		}
		if (locations.size() == 0) {
			return null;
		}
		return locations.get(Simulation.random.nextInt(locations.size()));
	}
	
	static String toString(GridMapData data) {
		StringBuilder output = new StringBuilder();
		int size = data.cells.size();

		int [] xy = new int [2];
		for (xy[0] = 0; xy[0] < size; ++xy[0]) {
			for (xy[1] = 0; xy[1] < size; ++ xy[1]) {
				output.append(xy[0]);
				output.append(",");
				output.append(xy[1]);
				output.append(":\n");
				
				Cell cell = data.cells.getCell(xy);
				for (CellObject object : cell.getAll()) {
					output.append("\t");
					output.append(object);
					output.append("\n");
				}
			}
		}
		return output.toString();
	}

	static void lingerUpdate(CellObject cellObject, Cell cell) {
		if (cellObject.hasProperty("update.linger")) {
			int linger = cellObject.getIntProperty("update.linger", 0);
			linger -= 1;
			if (linger <= 0) {
				cell.removeObject(cellObject.getName());
			} else {
				cellObject.setIntProperty("update.linger", linger);
			}
		}
	}

	public static String getMapName(String mapPath) {
		// can't just use system separator, could come from config file which needs to work across systems
		int index = mapPath.lastIndexOf("/");
		if (index == -1) {
			index = mapPath.lastIndexOf("\\");
			if (index == -1) {
				return mapPath;
			}
		}
		return mapPath.substring(index + 1);
	}
	
	public static class Barrier {
		public int id = -1;
		public boolean gateway = false;
		
		public int [] left;
		public int [] right;
		
		public Direction direction;

		private double [] centerpoint;
		public double [] centerpoint() {
			//  IMPORTANT! Assumes left/right won't change
			if (centerpoint != null) {
				return centerpoint;
			}
			
			int m = 0;
			int n = 0;
			centerpoint = new double [] { 0, 0 };
			
			if (Arrays.equals(left, right) == false) {
				switch (direction) {
				case NORTH:
				case SOUTH:
					// horizontal
					m = left[0];
					n = right[0];
					centerpoint[1] = left[1] * RoomWorld.cell_size;
					break;
				case EAST:
				case WEST:
					// vertical
					m = left[1];
					n = right[1];
					centerpoint[0] = left[0] * RoomWorld.cell_size;
					break;
				}
			} else {
				// single block
				centerpoint[0] = left[0] * RoomWorld.cell_size;
				centerpoint[1] = left[1] * RoomWorld.cell_size;

				switch (direction) {
				case NORTH:
					centerpoint[1] += RoomWorld.cell_size;
				case SOUTH:
					centerpoint[0] += RoomWorld.cell_size / 2;
					break;
				case WEST:
					centerpoint[0] += RoomWorld.cell_size;
				case EAST:
					centerpoint[1] += RoomWorld.cell_size / 2;
					break;
				}
				return centerpoint;
			}
			int numberOfBlocks = n - m;
			int [] upperLeft = left;
			// take abs, also note that if negative then we need to calculate center from the upper-left block
			// which this case is the "right"
			if (numberOfBlocks < 0) {
				numberOfBlocks *= -1;
				upperLeft = right;
			}
			numberOfBlocks += 1; // endpoints 0,0 and 0,3 represent 4 blocks
			assert numberOfBlocks > 1;
			
			if (left[0] == right[0]) {
				// vertical
				// add half to y
				centerpoint[1] = upperLeft[1] * RoomWorld.cell_size;
				centerpoint[1] += (numberOfBlocks / 2.0) * RoomWorld.cell_size;
				
				// if west, we gotta add a cell size to x
				if (direction == Direction.WEST) {
					centerpoint[0] += RoomWorld.cell_size;
				}
				
			} else {
				// horizontal
				// add half to x
				centerpoint[0] = upperLeft[0] * RoomWorld.cell_size;
				centerpoint[0] += (numberOfBlocks / 2.0) * RoomWorld.cell_size;

				// if north, we gotta add a cell size to y
				if (direction == Direction.NORTH) {
					centerpoint[1] += RoomWorld.cell_size;
				}
			}
			return centerpoint;
		}
		
		@Override
		public String toString() {
			String output = new String(Integer.toString(id));
			output += " (" + direction.id() + ")";
			double [] center = centerpoint();
			output += " (" + Integer.toString(left[0]) + "," + Integer.toString(left[1]) + ")-(" 
					+ Double.toString(center[0]) + "," + Double.toString(center[1]) + ")-(" 
					+ Integer.toString(right[0]) + "," + Integer.toString(right[1]) + ")";
			if (gateway) {
				output += " (gateway)";
			}
			return output;
		}
	}
	
	static class RoomMapBuildData {
		int roomCount = 0;
		int gatewayCount = 0;
		int wallCount = 0;
		int objectCount = 0;
		// Mapping of gateway id to the list of the ids of rooms it connects
		Map<Integer, List<Integer> > gatewayDestinationMap = new HashMap<Integer, List<Integer> >();
		// Mapping of room id to the list of the barriers surrounding that room
		Map<Integer, List<Barrier> > roomBarrierMap = new HashMap<Integer, List<Barrier> >();
		Set<CellObject> roomObjects = new HashSet<CellObject>();
		Map<CellObject, RoomObjectInfo> roomObjectInfoMap = new HashMap<CellObject, RoomObjectInfo>();

		private static void addDestinationToGateway(RoomMapBuildData roomData, int roomNumber, int gatewayId) {
			List<Integer> gatewayDestinations = roomData.gatewayDestinationMap.get(new Integer(gatewayId));
			assert gatewayDestinations != null;
			gatewayDestinations.add(new Integer(roomNumber));
			roomData.gatewayDestinationMap.put(new Integer(gatewayId), gatewayDestinations);
		}
	}
	
	static boolean generateRoomStructure(GridMapData data, RoomMapBuildData roomData) {
		// Start in upper-left corner
		// if cell is enterable, flood fill to find boundaries of room
		// Go from left to right, then to the start of the next line
		Queue<int []> floodQueue = new LinkedList<int []>();
		Set<Integer> explored = new HashSet<Integer>((data.cells.size()-2)*2);
		
		// this is where we will store gateway barriers for conversion to rooms 
		// in the second phase of map structure generation. 
		// this will contain duplicates since two different gateways can 
		// be represented by the same squares
		List<Barrier> gatewayBarriers = new ArrayList<Barrier>();

		int [] location = new int [2];
		for (location[1] = 1; location[1] < (data.cells.size() - 1); ++location[1]) {
			for (location[0] = 1; location[0] < (data.cells.size() - 1); ++location[0]) {
				logger.trace("Location: " + Arrays.toString(location));
				if (explored.contains(Arrays.hashCode(location))) {
					logger.trace("explored");
					continue;
				}
				explored.add(Arrays.hashCode(location));
				
				Cell cell = data.cells.getCell(location);
				if (cell.hasAnyWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyBlock) || cell.hasObject(edu.umich.soar.gridmap2d.Names.kPropertyGateway)) {
					logger.trace("not a room candidate");
					continue;
				}
				
				assert cell.getObject(edu.umich.soar.gridmap2d.Names.kRoomID) == null;

				// cell is enterable, we have a room
				int roomNumber = roomData.roomCount + roomData.gatewayCount + roomData.wallCount + roomData.objectCount;
				roomData.roomCount += 1;
				
				CellObject roomObject = data.cellObjectManager.createObject(edu.umich.soar.gridmap2d.Names.kRoomID);
				roomObject.setProperty(edu.umich.soar.gridmap2d.Names.kPropertyNumber, Integer.toString(roomNumber));

				Set<Integer> floodExplored = new HashSet<Integer>((data.cells.size()-2)*2);
				floodExplored.add(Arrays.hashCode(location));
				cell.addObject(roomObject);
				logger.trace("room id " + roomNumber + " " + cell);
				
				// add the surrounding cells to the queue 
				floodQueue.add(new int [] { location[0]+1,location[1] });
				floodQueue.add(new int [] { location[0],location[1]+1 });
				floodQueue.add(new int [] { location[0]-1,location[1] });
				floodQueue.add(new int [] { location[0],location[1]-1 });
				
				// flood and mark all room cells and save walls
				Set<Integer> walls = new HashSet<Integer>();
				logger.trace("processing flood queue");
				while(floodQueue.size() > 0) {
					int [] floodLocation = floodQueue.remove();

					if (floodExplored.contains(Arrays.hashCode(floodLocation))) {
						continue;
					}
					floodExplored.add(Arrays.hashCode(floodLocation));
					
					cell = data.cells.getCell(floodLocation);
					if (cell.hasAnyWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyBlock) || cell.hasObject(edu.umich.soar.gridmap2d.Names.kPropertyGateway)) {
						walls.add(Arrays.hashCode(floodLocation));
						logger.trace("added wall " + Arrays.toString(floodLocation));
						continue;
					}

					explored.add(Arrays.hashCode(floodLocation));

					cell.addObject(new CellObject(roomObject));
					logger.trace("added room object to " + cell);

					// add the four surrounding cells to the queue
					floodQueue.add(new int [] { floodLocation[0]+1,floodLocation[1] });
					floodQueue.add(new int [] { floodLocation[0]-1,floodLocation[1] });
					floodQueue.add(new int [] { floodLocation[0],floodLocation[1]+1 });
					floodQueue.add(new int [] { floodLocation[0],floodLocation[1]-1 });
				}
				logger.trace("done with flood queue");
				
				// figure out walls going clockwise starting with the wall north of the first square in the room
				Direction direction = Direction.EAST;
				int [] startingWall = new int [] { location[0], location[1]-1 };
				int [] next = Arrays.copyOf(startingWall, startingWall.length);
				
				// Keep track of the current barrier information. When the wall ends, simply add it to the the room
				// barrier map.
				Barrier currentBarrier = null;
				
				// Keep track of barrier information
				List<Barrier> barrierList = new ArrayList<Barrier>();
				
				// I probably should have commented this more when I wrote it.
				// The comments have been inserted after the initial writing so they may be slightly wrong.
				logger.trace("processing barriers");
				while (true) {
					
					// This is used to figure out how to turn corners when walking along walls.
					// Also used with figuring out barrier endpoints.
					int [] previous = Arrays.copyOf( next, next.length );
					
					// next is actually the location we're examining now
					cell = data.cells.getCell(next);
					
					// used to detect turns
					int [] rightOfNext = null;
					
					// Get the wall and gateway objects. The can't both exist.
					CellObject gatewayObject = cell.getObject(edu.umich.soar.gridmap2d.Names.kGatewayID);
					CellObject wallObject = cell.getObject(edu.umich.soar.gridmap2d.Names.kWallID);
					
					// One must exist, but not both
					assert ((gatewayObject == null) && (wallObject != null)) || ((gatewayObject != null) && (wallObject == null));

					if (gatewayObject != null) {
						logger.trace(cell + " is gateway");

						if (currentBarrier != null) {
							// If we were just walking a wall, end and add the barrier
							if (currentBarrier.gateway == false) {
								barrierList.add(currentBarrier);
								logger.trace("ended wall");
								currentBarrier = null;
							}
						}
						
						// At this point, the currentBarrier, if it exists, is the gateway we're walking
						if (currentBarrier == null) {
							
							// getting here means we're starting a new section of gateway
							
							// create a barrier
							currentBarrier = new Barrier();
							currentBarrier.gateway = true;
							currentBarrier.left = Arrays.copyOf(next, next.length);
							currentBarrier.direction = direction.left();
							
							// create a new gateway id
							currentBarrier.id = roomData.roomCount + roomData.gatewayCount + roomData.wallCount + roomData.objectCount;
							roomData.gatewayCount += 1;

							logger.trace("new gateway");
							
							// add the current room to the gateway destination list
							List<Integer> gatewayDestinations = roomData.gatewayDestinationMap.get(new Integer(currentBarrier.id));
							if (gatewayDestinations == null) {
								gatewayDestinations = new ArrayList<Integer>();
							}
							gatewayDestinations.add(new Integer(roomNumber));
							roomData.gatewayDestinationMap.put(new Integer(currentBarrier.id), gatewayDestinations);
						}

						// id are noted by the direction of the wall
						gatewayObject.setProperty(currentBarrier.direction.id(), Integer.toString(currentBarrier.id));
						
					} else if (wallObject != null) /*redundant*/ {
						logger.trace(cell + " is wall");

						if (currentBarrier != null) {
							// If we were just walking a gateway, end and add the barrier
							if (currentBarrier.gateway) {
								// keep track of gateway barriers
								gatewayBarriers.add(currentBarrier);
								barrierList.add(currentBarrier);
								logger.trace("ended gateway");
								currentBarrier = null;
							}
						}
						
						// At this point, the currentBarrier, if it exists, is the wall we're walking
						if (currentBarrier == null) {
							
							// getting here means we're starting a new section of wall
							currentBarrier = new Barrier();
							currentBarrier.left = Arrays.copyOf(next, next.length);
							currentBarrier.direction = direction.left();
							
							// create a new id
							currentBarrier.id = roomData.roomCount + roomData.gatewayCount + roomData.wallCount + roomData.objectCount;
							roomData.wallCount += 1;
							
							logger.trace("new wall: " + currentBarrier.id);
						}
						
						// id are noted by the direction of the wall
						wallObject.setProperty(currentBarrier.direction.id(), Integer.toString(currentBarrier.id));

					} else {
						// the world is ending, check the asserts
						assert false;
					}
					
					// since current barrier is the gateway we're walking, update it's endpoint before we translate it
					currentBarrier.right = Arrays.copyOf(next, next.length);
					
					// walk to the next section of wall
					Direction.translate(next, direction);
					
					// we get the right of next here because if there is a next and
					// a wall to the right of it, that means next is a gateway but there is
					// a wall in the way so that gateway doesn't technically border our room,
					// so, the gateway ends and we continue on the next segment of wall.
					rightOfNext = Arrays.copyOf(next, next.length);
					Direction.translate(rightOfNext, direction.right());

					// if there isn't a next, we're done anyway.
					// continue if we're moving on with this section of wall, or fall
					// through to terminate the wall.
					if (walls.contains(Arrays.hashCode(next)) && !walls.contains(Arrays.hashCode(rightOfNext))) {
						continue;
					}

					// gateway or wall stops here
					logger.trace("ended (turn)");
					
					// and the barrier to the list
					
					if (currentBarrier.gateway) {
						// keep track of gateway barriers
						gatewayBarriers.add(currentBarrier);
					}
					
					barrierList.add(currentBarrier);
					currentBarrier = null;
					
					// when going clockwise, when we turn right, we go to the cell adjacent to "next",
					// when we turn left, we go to the cell adjacent to "previous"
					//
					//  going east
					// +---+---+---+  Direction travelling along wall: east
					// |   | L |   |  W = wall we've seen
					// +---+---+---+  P = "previous"
					// | W | P | N |  N = "next" (we just tested this and it isn't a wall)
					// +---+---+---+  L = possible next, indicates left turn (P included on the new wall)
					// |   |   | R |  R = possible next, indicates right turn
					// +---+---+---+    = irrelevant cell
					
					//  going west    going north   going south
					// +---+---+---+ +---+---+---+ +---+---+---+
					// | R |   |   | |   | N | R | |   | W |   |
					// +---+---+---+ +---+---+---+ +---+---+---+
					// | N | P | W | | L | P |   | |   | P | L |
					// +---+---+---+ +---+---+---+ +---+---+---+
					// |   | L |   | |   | W |   | | R | N |   |
					// +---+---+---+ +---+---+---+ +---+---+---+
					
					// try turning right first
					Direction.translate(next, direction.right());
					if (walls.contains(Arrays.hashCode(next))) {
						logger.trace("right turn");
						
						// right worked
						direction = direction.right();

					} else {
						// try turning left next
						next = Arrays.copyOf(previous, previous.length);
						Direction.translate(next, direction.left());
						
						if (walls.contains(Arrays.hashCode(next))) {
							logger.trace("left turn");

							// left worked
							direction = direction.left();
							
							// need to stay on previous because it is included on the new wall
							next = previous;
							// the remove will silently fail and that's ok
							
						} else {
							logger.trace("single length wall, left turn");

							// single length wall (perform "left" turn)
							direction = direction.left();

							// need to stay on previous because it is included on the new wall
							next = previous;
							// the remove will silently fail and that's ok
						}
					}
					
					// See if our turn leads us home
					if (Arrays.equals(next, startingWall)) {
						break;
					}
				}
				logger.trace("done processing barriers");
				
				// Generate centerpoints and store room information
				generateCenterpoints(roomNumber, barrierList);
				roomData.roomBarrierMap.put(roomNumber, barrierList);
			}
		}
		
		// convert all the gateways to areas
		gatewaysToAreasStep(data, gatewayBarriers, roomData);
		
		// Assign areas for all objects
		for (RoomObjectInfo info : roomData.roomObjectInfoMap.values()) {
			info.area = data.cells.getCell(info.location).getAllWithProperty(edu.umich.soar.gridmap2d.Names.kPropertyNumber).get(0).getIntProperty(edu.umich.soar.gridmap2d.Names.kPropertyNumber, -1);
		}
		
		// print gateway information
		Iterator<Integer> gatewayKeyIter = roomData.gatewayDestinationMap.keySet().iterator();
		while (gatewayKeyIter.hasNext()) {
			Integer gatewayId = gatewayKeyIter.next();
			String toList = "";
			Iterator<Integer> gatewayDestIter = roomData.gatewayDestinationMap.get(gatewayId).iterator();
			while (gatewayDestIter.hasNext()) {
				toList += gatewayDestIter.next() + " ";
			}
			logger.info("Gateway " + gatewayId + ": " + toList);
		}
		
		return true;
	}

	static void generateCenterpoints(int roomNumber, List<Barrier> barrierList) {
		logger.info("Room " + roomNumber + ":");
		Iterator<Barrier> iter = barrierList.iterator();
		while (iter.hasNext()) {
			Barrier barrier = iter.next();
			
			// generate centerpoint
			barrier.centerpoint();

			logger.info(barrier);
		}
	}
	
	private static void gatewaysToAreasStep(GridMapData data, List<Barrier> gatewayBarriers, RoomMapBuildData roomData) {
		// make the gateway also a room
		// add new room to current gateway destination list
		// create new barrier list for this new room: 2 walls and 2 gateways
		// update gateway destination list for new gateways
		
		Iterator<Barrier> iter = gatewayBarriers.iterator();
		while (iter.hasNext()) {
			Barrier gatewayBarrier = iter.next();
			
			// duplicates exist in this list, check to see if we're already a room
			{
				Cell cell = data.cells.getCell(gatewayBarrier.left);
				if (cell.hasObject(edu.umich.soar.gridmap2d.Names.kRoomID)) {
					// we have already processed this room, just need to add the room id
					// to the gateway's destination list
					RoomMapBuildData.addDestinationToGateway(roomData, cell.getObject(edu.umich.soar.gridmap2d.Names.kRoomID).getIntProperty(edu.umich.soar.gridmap2d.Names.kPropertyNumber, -1), gatewayBarrier.id);
					continue;
				}
			}
			
			// get a new room id
			int roomNumber = roomData.roomCount + roomData.gatewayCount + roomData.wallCount + roomData.objectCount;
			roomData.roomCount += 1;
			
			// add new id to current gateway destination list
			RoomMapBuildData.addDestinationToGateway(roomData, roomNumber, gatewayBarrier.id);
			
			CellObject theNewRoomObject = data.cellObjectManager.createObject(edu.umich.soar.gridmap2d.Names.kRoomID);
			theNewRoomObject.setProperty(edu.umich.soar.gridmap2d.Names.kPropertyNumber, Integer.toString(roomNumber));
			{
				Cell cell = data.cells.getCell(gatewayBarrier.left);
				cell.addObject(theNewRoomObject);
			}

			Direction incrementDirection = Direction.NONE;
			if (gatewayBarrier.left[0] == gatewayBarrier.right[0]) {
				// vertical gateway
				if (gatewayBarrier.left[1] < gatewayBarrier.right[1]) {
					// increasing to right, south
					incrementDirection = Direction.SOUTH;

				} else if (gatewayBarrier.left[1] > gatewayBarrier.right[1]) {
					// decreasing to right, north
					incrementDirection = Direction.NORTH;
				}
			} else {
				// horizontal gateway
				if (gatewayBarrier.left[0] < gatewayBarrier.right[0]) {
					// increasing to right, east
					incrementDirection = Direction.EAST;

				} else if (gatewayBarrier.left[0] > gatewayBarrier.right[0]) {
					// decreasing to right, west
					incrementDirection = Direction.WEST;
				}
			}
			
			if (incrementDirection == Direction.NONE) {
				// Direction depends on which ways have walls and which ways have rooms
				int [] feeler;
				feeler = Arrays.copyOf(gatewayBarrier.left, gatewayBarrier.left.length);
				feeler[0] -= 1;
				Cell cell = data.cells.getCell(feeler);
				if (cell.getObject(edu.umich.soar.gridmap2d.Names.kWallID) != null) {
					// horizontal gateway
					incrementDirection = Direction.EAST;
				} else {
					// vertical gateway
					incrementDirection = Direction.SOUTH;
				}
			}
			
			// we need to walk to the right and assing the room id to everyone
			int [] current = Arrays.copyOf(gatewayBarrier.left, gatewayBarrier.left.length);
			while (Arrays.equals(current, gatewayBarrier.right) == false) {
				current = Direction.translate(current, incrementDirection);

				Cell cell = data.cells.getCell(current);
				cell.addObject(new CellObject(theNewRoomObject));
			}

			// now we need to round up the four barriers
			List<Barrier> barrierList = new ArrayList<Barrier>();
			
			////////////////////
			// we can start by walking the wrong direction off the left endpoint
			doNewWall(roomData, data, gatewayBarrier.left, incrementDirection.backward(), barrierList);
			////////////////////
			
			////////////////////
			// then to the left of our left endpoint, and walk down the gateway to the left of the right endpoint
			doNewGateway(roomData, data, gatewayBarrier.left, gatewayBarrier.right, incrementDirection.left(), incrementDirection, roomNumber, barrierList);
			////////////////////
			
			////////////////////
			// next is just off the right endpoint
			doNewWall(roomData, data, gatewayBarrier.right, incrementDirection, barrierList);
			////////////////////

			////////////////////
			// then to the right of our right endpoint, and walk backwards down the gateway to the right of the left endpoint
			doNewGateway(roomData, data, gatewayBarrier.right, gatewayBarrier.left, incrementDirection.right(), incrementDirection.backward(), roomNumber, barrierList);
			////////////////////

			// Generate centerpoints and store room information
			generateCenterpoints(roomNumber, barrierList);
			roomData.roomBarrierMap.put(roomNumber, barrierList);
		}
	}

	/**
	 * Create a wall for the new rooms created by the initial gateways.
	 * 
	 * @param endPoint The square representing the side of the room butting up to this wall
	 * @param direction The direction to go to reach the wall
	 * @param barrierList The barrier list for our new room
	 */
	private static void doNewWall(RoomMapBuildData roomData, GridMapData data, int [] endPoint, Direction direction, List<Barrier> barrierList) {
		Barrier currentBarrier = new Barrier();
		currentBarrier.left = Direction.translate(endPoint, direction, new int[2]);

		// this is a wall and is only size 1
		currentBarrier.right = Arrays.copyOf(currentBarrier.left, currentBarrier.left.length);
		
		// get a new wall id
		currentBarrier.id = roomData.roomCount + roomData.gatewayCount + roomData.wallCount + roomData.objectCount;
		roomData.wallCount += 1;
		// the direction is the direction we just traveled to get to the wall
		currentBarrier.direction = direction;
		
		// get the wall object
		Cell cell = data.cells.getCell(currentBarrier.left);
		CellObject wallObject = cell.getObject(edu.umich.soar.gridmap2d.Names.kWallID);
		
		// walls don't share ids, they are noted by the direction of the wall
		wallObject.setProperty(currentBarrier.direction.id(), Integer.toString(currentBarrier.id));

		// store the barrier
		barrierList.add(currentBarrier);
	}
	
	/**
	 * Creates a gateway for the new rooms created by the initial gateways.
	 * 
	 * @param startPoint Travelling clockwise around the room, this is the end of the room adjacent to the start gateway we're creating
	 * @param endPoint This is the end of the room adjacent to the end of the gateway.
	 * @param direction This is the side of the room the gateway is on, or (alternatively), what direction we have to turn to face the gateway.
	 * @param walkDirection This is the direction we go to walk down the gateway (parallel to the room)
	 * @param roomNumber This is the id number of the room we're in
	 * @param barrierList This is the list of barriers for the room we're in
	 */
	private static void doNewGateway(RoomMapBuildData roomData, GridMapData data, int [] startPoint, int [] endPoint, Direction direction, Direction walkDirection, int roomNumber, List<Barrier> barrierList) {
		// next is the gateway to the left of our left endpoint
		Barrier currentBarrier = new Barrier();
		currentBarrier.gateway = true;
		currentBarrier.left = Direction.translate(startPoint, direction, new int[2]);

		// get a new gateway id
		currentBarrier.id = roomData.roomCount + roomData.gatewayCount + roomData.wallCount + roomData.objectCount;
		roomData.gatewayCount += 1;
		// the direction is the direction we just traveled to get to the gateway
		currentBarrier.direction = direction;

		// this is a gateway of unknown size
		currentBarrier.right = Arrays.copyOf(currentBarrier.left, currentBarrier.left.length);
		
		// we know it ends left of the right endpoint
		int [] endOfGateway = Direction.translate(endPoint, direction, new int[2]);
		
		while (true) {
			// create the gateway object
			CellObject gatewayObject = data.cellObjectManager.createObject(edu.umich.soar.gridmap2d.Names.kGatewayID);
			gatewayObject.removeProperty(edu.umich.soar.gridmap2d.Names.kPropertyGatewayRender);

			// gateway don't share ids, they are noted by the direction of the gateway
			gatewayObject.setProperty(currentBarrier.direction.id(), Integer.toString(currentBarrier.id));

			// put the object in the cell
			Cell cell = data.cells.getCell(currentBarrier.right);
			cell.addObject(gatewayObject);
			
			// record the destinations which is the new room and the room the gateway is sitting on
			// add the current room to the gateway destination list
			List<Integer> gatewayDestinations = new ArrayList<Integer>();
			gatewayDestinations.add(new Integer(roomNumber));
			gatewayDestinations.add(new Integer(cell.getObject(edu.umich.soar.gridmap2d.Names.kRoomID).getIntProperty(edu.umich.soar.gridmap2d.Names.kPropertyNumber, -1)));
			roomData.gatewayDestinationMap.put(new Integer(currentBarrier.id), gatewayDestinations);
			
			if (Arrays.equals(currentBarrier.right, endOfGateway)) {
				break;
			}
			
			// increment and loop
			currentBarrier.right = Direction.translate(currentBarrier.right, walkDirection);
		}

		// store the barrier
		barrierList.add(currentBarrier);
	}	
}
