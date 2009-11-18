package edu.umich.soar.room.map;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Queue;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.config.Config;
import edu.umich.soar.config.ConfigFile;
import edu.umich.soar.config.ParseError;
import edu.umich.soar.room.config.SimConfig;
import edu.umich.soar.room.core.Direction;
import edu.umich.soar.room.core.Names;
import edu.umich.soar.room.core.Simulation;

public class RoomMap implements CellObjectObserver {
	private static Log logger = LogFactory.getLog(RoomMap.class);

	private static class Counts {
		private int roomCount;
		private int gatewayCount;
		private int wallCount;
		private int objectCount;

		Counts() {
			reset();
		}

		void reset() {
			roomCount = 1;
			gatewayCount = 1;
			wallCount = 1;
			objectCount = 1;
		}

		int nextRoom() {
			return roomCount++/* + gatewayCount + wallCount + objectCount*/;
		}

		int nextGateway() {
			return /*roomCount + */gatewayCount++/* + wallCount + objectCount*/;
		}

		int nextWall() {
			return /*roomCount + gatewayCount + */wallCount++/* + objectCount*/;
		}

		int nextObject() {
			return /*roomCount + gatewayCount + wallCount + */objectCount++;
		}
	}

	private final Counts counts = new Counts();
	// Mapping of gateway id to the list of the ids of rooms it connects
	private final Map<Integer, List<Integer>> gatewayDestinationMap = new HashMap<Integer, List<Integer>>();
	// Mapping of room id to the list of the barriers surrounding that room
	private final Map<Integer, List<RoomBarrier>> roomBarrierMap = new HashMap<Integer, List<RoomBarrier>>();
	private final Map<CellObject, RoomObject> roomObjects = new HashMap<CellObject, RoomObject>();

	private boolean generatePhase = false;
	private final List<CellObject> mobs = new ArrayList<CellObject>();
	private final SimConfig simConfig;
	
	public RoomMap(String mapPath, SimConfig simConfig) {
		this.mapPath = mapPath;
		this.simConfig = simConfig;

		reset();
	}

	public boolean isAvailable(int[] location) {
		Cell cell = getData().cells.getCell(location);
		boolean enterable = cell
				.getAllObjectsWithProperty(Names.kPropertyBlock).isEmpty();
		boolean noPlayer = !cell.hasPlayers();
		boolean movable = !cell.getAllObjectsWithProperty(
				Names.kRoomObjectMovable).isEmpty();
		return enterable && noPlayer && !movable;
	}

	public void reset() {
		counts.reset();
		gatewayDestinationMap.clear();
		roomBarrierMap.clear();
		roomObjects.clear();
		mobs.clear();

		generatePhase = true;
		reload();
		generateRoomStructure();
		generatePhase = false;
	}

	public Collection<RoomObject> getRoomObjects() {
		return roomObjects.values();
	}

	@Override
	public void addStateUpdate(CellObject added) {
		if (!added.hasProperty(Names.kRoomObjectMovable)) {
			return;
		}

		if (generatePhase) {
			mobs.add(added);
			return;
		}

		RoomObject ro = roomObjects.get(added);
		if (ro == null) {
			ro = new RoomObject(added, counts.nextObject());
			roomObjects.put(added, ro);
			logger.info("created new room object " + ro);
		} else {
			logger.info("re-adding " + ro);
		}
		ro.update(getData().cells);
	}

	@Override
	public void removalStateUpdate(CellObject removed) {
		if (!removed.hasProperty(Names.kRoomObjectMovable)) {
			return;
		}
	}

	public int getLocationId(int[] location) {
		assert location != null;

		CellObject locationObject = getData().cells.getCell(location)
				.getFirstObjectWithProperty(Names.kPropertyNumber);
		assert locationObject != null;
		return locationObject.getProperty(Names.kPropertyNumber, -1,
				Integer.class);
	}

	public List<RoomBarrier> getRoomBarrierList(int oldLocationId) {
		return roomBarrierMap.get(oldLocationId);
	}

	public List<Integer> getGatewayDestinationList(int id) {
		return gatewayDestinationMap.get(id);
	}

	boolean generateRoomStructure() {
		// Start in upper-left corner
		// if cell is enterable, flood fill to find boundaries of room
		// Go from left to right, then to the start of the next line
		Queue<int[]> floodQueue = new LinkedList<int[]>();
		Set<Integer> explored = new HashSet<Integer>(
				(getData().cells.size() - 2) * 2);

		// this is where we will store gateway barriers for conversion to rooms
		// in the second phase of map structure generation.
		// this will contain duplicates since two different gateways can
		// be represented by the same squares
		List<RoomBarrier> gatewayBarriers = new ArrayList<RoomBarrier>();

		int[] location = new int[2];
		for (location[1] = 1; location[1] < (getData().cells.size() - 1); ++location[1]) {
			for (location[0] = 1; location[0] < (getData().cells.size() - 1); ++location[0]) {
				logger.trace("Location: " + Arrays.toString(location));
				if (explored.contains(Arrays.hashCode(location))) {
					logger.trace("explored");
					continue;
				}
				explored.add(Arrays.hashCode(location));

				Cell cell = getData().cells.getCell(location);
				if (cell.hasObjectWithProperty(Names.kPropertyBlock)
						|| cell.hasObjectWithProperty(Names.kPropertyGateway)) {
					logger.trace("not a room candidate");
					continue;
				}

				assert !cell.hasObjectWithProperty(Names.kRoomID);

				// cell is enterable, we have a room
				int roomNumber = counts.nextRoom();

				CellObject roomObject = getData().cellObjectManager
						.createObject(Names.kRoomID);
				roomObject.setProperty(Names.kPropertyNumber, Integer
						.toString(roomNumber));

				Set<Integer> floodExplored = new HashSet<Integer>(
						(getData().cells.size() - 2) * 2);
				floodExplored.add(Arrays.hashCode(location));
				getCell(location).addObject(roomObject);
				logger.trace("room id " + roomNumber + " " + cell);

				// add the surrounding cells to the queue
				floodQueue.add(new int[] { location[0] + 1, location[1] });
				floodQueue.add(new int[] { location[0], location[1] + 1 });
				floodQueue.add(new int[] { location[0] - 1, location[1] });
				floodQueue.add(new int[] { location[0], location[1] - 1 });

				// flood and mark all room cells and save walls
				Set<Integer> walls = new HashSet<Integer>();
				logger.trace("processing flood queue");
				while (floodQueue.size() > 0) {
					int[] floodLocation = floodQueue.remove();

					if (floodExplored.contains(Arrays.hashCode(floodLocation))) {
						continue;
					}
					floodExplored.add(Arrays.hashCode(floodLocation));

					cell = getData().cells.getCell(floodLocation);
					if (cell.hasObjectWithProperty(Names.kPropertyBlock)
							|| cell
									.hasObjectWithProperty(Names.kPropertyGateway)) {
						walls.add(Arrays.hashCode(floodLocation));
						logger.trace("added wall "
								+ Arrays.toString(floodLocation));
						continue;
					}

					explored.add(Arrays.hashCode(floodLocation));

					getCell(floodLocation).addObject(new CellObject(roomObject));
					logger.trace("added room object to " + cell);

					// add the four surrounding cells to the queue
					floodQueue.add(new int[] { floodLocation[0] + 1,
							floodLocation[1] });
					floodQueue.add(new int[] { floodLocation[0] - 1,
							floodLocation[1] });
					floodQueue.add(new int[] { floodLocation[0],
							floodLocation[1] + 1 });
					floodQueue.add(new int[] { floodLocation[0],
							floodLocation[1] - 1 });
				}
				logger.trace("done with flood queue");

				// figure out walls going clockwise starting with the wall north
				// of the first square in the room
				Direction direction = Direction.EAST;
				int[] startingWall = new int[] { location[0], location[1] - 1 };
				int[] next = Arrays.copyOf(startingWall, startingWall.length);

				// Keep track of the current barrier information. When the wall
				// ends, simply add it to the the room
				// barrier map.
				RoomBarrier currentBarrier = null;

				// Keep track of barrier information
				List<RoomBarrier> barrierList = new ArrayList<RoomBarrier>();

				// I probably should have commented this more when I wrote it.
				// The comments have been inserted after the initial writing so
				// they may be slightly wrong.
				logger.trace("processing barriers");
				while (true) {

					// This is used to figure out how to turn corners when
					// walking along walls.
					// Also used with figuring out barrier endpoints.
					int[] previous = Arrays.copyOf(next, next.length);

					// next is actually the location we're examining now
					cell = getData().cells.getCell(next);

					// used to detect turns
					int[] rightOfNext = null;

					// Get the wall and gateway objects. The can't both exist.
					CellObject gatewayObject = cell
							.getFirstObjectWithProperty(Names.kGatewayID);
					CellObject wallObject = cell
							.getFirstObjectWithProperty(Names.kWallID);

					// One must exist, but not both
					assert (gatewayObject == null && wallObject != null)
							|| (gatewayObject != null && wallObject == null);

					if (gatewayObject != null) {
						logger.trace(cell + " is gateway");

						if (currentBarrier != null) {
							// If we were just walking a wall, end and add the
							// barrier
							if (currentBarrier.gateway == false) {
								barrierList.add(currentBarrier);
								logger.trace("ended wall");
								currentBarrier = null;
							}
						}

						// At this point, the currentBarrier, if it exists, is
						// the gateway we're walking
						if (currentBarrier == null) {

							// getting here means we're starting a new section
							// of gateway

							// create a barrier
							currentBarrier = new RoomBarrier();
							currentBarrier.gateway = true;
							currentBarrier.left = Arrays.copyOf(next,
									next.length);
							currentBarrier.direction = direction.left();

							// create a new gateway id
							currentBarrier.id = counts.nextGateway();

							logger.trace("new gateway");

							// add the current room to the gateway destination
							// list
							List<Integer> gatewayDestinations = gatewayDestinationMap
									.get(new Integer(currentBarrier.id));
							if (gatewayDestinations == null) {
								gatewayDestinations = new ArrayList<Integer>();
							}
							gatewayDestinations.add(new Integer(roomNumber));
							gatewayDestinationMap.put(new Integer(
									currentBarrier.id), gatewayDestinations);
						}

						// id are noted by the direction of the wall
						gatewayObject.setProperty(
								currentBarrier.direction.id(), Integer
										.toString(currentBarrier.id));

					} else /* if (!wallObjects.isEmpty()) */{
						logger.trace(cell + " is wall");

						if (currentBarrier != null) {
							// If we were just walking a gateway, end and add
							// the barrier
							if (currentBarrier.gateway) {
								// keep track of gateway barriers
								gatewayBarriers.add(currentBarrier);
								barrierList.add(currentBarrier);
								logger.trace("ended gateway");
								currentBarrier = null;
							}
						}

						// At this point, the currentBarrier, if it exists, is
						// the wall we're walking
						if (currentBarrier == null) {

							// getting here means we're starting a new section
							// of wall
							currentBarrier = new RoomBarrier();
							currentBarrier.left = Arrays.copyOf(next,
									next.length);
							currentBarrier.direction = direction.left();

							// create a new id
							currentBarrier.id = counts.nextWall();

							logger.trace("new wall: " + currentBarrier.id);
						}

						// id are noted by the direction of the wall
						wallObject.setProperty(currentBarrier.direction.id(),
								Integer.toString(currentBarrier.id));
					}

					// since current barrier is the gateway we're walking,
					// update it's endpoint before we translate it
					currentBarrier.right = Arrays.copyOf(next, next.length);

					// walk to the next section of wall
					Direction.translate(next, direction);

					// we get the right of next here because if there is a next
					// and
					// a wall to the right of it, that means next is a gateway
					// but there is
					// a wall in the way so that gateway doesn't technically
					// border our room,
					// so, the gateway ends and we continue on the next segment
					// of wall.
					rightOfNext = Arrays.copyOf(next, next.length);
					Direction.translate(rightOfNext, direction.right());

					// if there isn't a next, we're done anyway.
					// continue if we're moving on with this section of wall, or
					// fall
					// through to terminate the wall.
					if (walls.contains(Arrays.hashCode(next))
							&& !walls.contains(Arrays.hashCode(rightOfNext))) {
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

					// when going clockwise, when we turn right, we go to the
					// cell adjacent to "next",
					// when we turn left, we go to the cell adjacent to
					// "previous"
					//
					// going east
					// +---+---+---+ Direction travelling along wall: east
					// | | L | | W = wall we've seen
					// +---+---+---+ P = "previous"
					// | W | P | N | N = "next" (we just tested this and it
					// isn't a wall)
					// +---+---+---+ L = possible next, indicates left turn (P
					// included on the new wall)
					// | | | R | R = possible next, indicates right turn
					// +---+---+---+ = irrelevant cell

					// going west going north going south
					// +---+---+---+ +---+---+---+ +---+---+---+
					// | R | | | | | N | R | | | W | |
					// +---+---+---+ +---+---+---+ +---+---+---+
					// | N | P | W | | L | P | | | | P | L |
					// +---+---+---+ +---+---+---+ +---+---+---+
					// | | L | | | | W | | | R | N | |
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

							// need to stay on previous because it is included
							// on the new wall
							next = previous;
							// the remove will silently fail and that's ok

						} else {
							logger.trace("single length wall, left turn");

							// single length wall (perform "left" turn)
							direction = direction.left();

							// need to stay on previous because it is included
							// on the new wall
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
				roomBarrierMap.put(roomNumber, barrierList);
			}
		}

		// convert all the gateways to areas
		gatewaysToAreasStep(gatewayBarriers);

		// Create RoomObjects for all movables
		for (CellObject o : mobs) {
			RoomObject ro = new RoomObject(o, counts.nextObject());
			ro.update(getData().cells);
			roomObjects.put(o, ro);
		}
		mobs.clear();

		// print gateway information
		Iterator<Integer> gatewayKeyIter = gatewayDestinationMap.keySet()
				.iterator();
		while (gatewayKeyIter.hasNext()) {
			Integer gatewayId = gatewayKeyIter.next();
			String toList = "";
			Iterator<Integer> gatewayDestIter = gatewayDestinationMap.get(
					gatewayId).iterator();
			while (gatewayDestIter.hasNext()) {
				toList += gatewayDestIter.next() + " ";
			}
			logger.info("Gateway " + gatewayId + ": " + toList);
		}

		return true;
	}

	private void gatewaysToAreasStep(List<RoomBarrier> gatewayBarriers) {
		// make the gateway also a room
		// add new room to current gateway destination list
		// create new barrier list for this new room: 2 walls and 2 gateways
		// update gateway destination list for new gateways

		Iterator<RoomBarrier> iter = gatewayBarriers.iterator();
		while (iter.hasNext()) {
			RoomBarrier gatewayBarrier = iter.next();

			// duplicates exist in this list, check to see if we're already a
			// room
			{
				Cell cell = getData().cells.getCell(gatewayBarrier.left);
				if (cell.hasObjectWithProperty(Names.kRoomID)) {
					// we have already processed this room, just need to add the
					// room id
					// to the gateway's destination list
					addDestinationToGateway(cell.getFirstObjectWithProperty(
							Names.kRoomID).getProperty(Names.kPropertyNumber,
							-1, Integer.class), gatewayBarrier.id);
					continue;
				}
			}

			// get a new room id
			int roomNumber = counts.nextRoom();

			// add new id to current gateway destination list
			addDestinationToGateway(roomNumber, gatewayBarrier.id);

			CellObject theNewRoomObject = getData().cellObjectManager
					.createObject(Names.kRoomID);
			theNewRoomObject.setProperty(Names.kPropertyNumber, Integer
					.toString(roomNumber));
			{
				getCell(gatewayBarrier.left).addObject(theNewRoomObject);
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
				// Direction depends on which ways have walls and which ways
				// have rooms
				int[] feeler;
				feeler = Arrays.copyOf(gatewayBarrier.left,
						gatewayBarrier.left.length);
				feeler[0] -= 1;
				Cell cell = getData().cells.getCell(feeler);
				if (cell.hasObjectWithProperty(Names.kWallID)) {
					// horizontal gateway
					incrementDirection = Direction.EAST;
				} else {
					// vertical gateway
					incrementDirection = Direction.SOUTH;
				}
			}

			// we need to walk to the right and assing the room id to everyone
			int[] current = Arrays.copyOf(gatewayBarrier.left,
					gatewayBarrier.left.length);
			while (Arrays.equals(current, gatewayBarrier.right) == false) {
				current = Direction.translate(current, incrementDirection);

				getCell(current).addObject(new CellObject(theNewRoomObject));
			}

			// now we need to round up the four barriers
			List<RoomBarrier> barrierList = new ArrayList<RoomBarrier>();

			// //////////////////
			// we can start by walking the wrong direction off the left endpoint
			doNewWall(gatewayBarrier.left, incrementDirection.backward(),
					barrierList);
			// //////////////////

			// //////////////////
			// then to the left of our left endpoint, and walk down the gateway
			// to the left of the right endpoint
			doNewGateway(gatewayBarrier.left, gatewayBarrier.right,
					incrementDirection.left(), incrementDirection, roomNumber,
					barrierList);
			// //////////////////

			// //////////////////
			// next is just off the right endpoint
			doNewWall(gatewayBarrier.right, incrementDirection, barrierList);
			// //////////////////

			// //////////////////
			// then to the right of our right endpoint, and walk backwards down
			// the gateway to the right of the left endpoint
			doNewGateway(gatewayBarrier.right, gatewayBarrier.left,
					incrementDirection.right(), incrementDirection.backward(),
					roomNumber, barrierList);
			// //////////////////

			// Generate centerpoints and store room information
			generateCenterpoints(roomNumber, barrierList);
			roomBarrierMap.put(roomNumber, barrierList);
		}
	}

	/**
	 * Create a wall for the new rooms created by the initial gateways.
	 * 
	 * @param endPoint
	 *            The square representing the side of the room butting up to
	 *            this wall
	 * @param direction
	 *            The direction to go to reach the wall
	 * @param barrierList
	 *            The barrier list for our new room
	 */
	private void doNewWall(int[] endPoint, Direction direction,
			List<RoomBarrier> barrierList) {
		RoomBarrier currentBarrier = new RoomBarrier();
		currentBarrier.left = Direction.translate(endPoint, direction,
				new int[2]);

		// this is a wall and is only size 1
		currentBarrier.right = Arrays.copyOf(currentBarrier.left,
				currentBarrier.left.length);

		// get a new wall id
		currentBarrier.id = counts.nextWall();

		// the direction is the direction we just traveled to get to the wall
		currentBarrier.direction = direction;

		// get the wall object
		Cell cell = getData().cells.getCell(currentBarrier.left);
		CellObject wallObject = cell.getFirstObjectWithProperty(Names.kWallID);

		// walls don't share ids, they are noted by the direction of the wall
		wallObject.setProperty(currentBarrier.direction.id(), Integer
				.toString(currentBarrier.id));

		// store the barrier
		barrierList.add(currentBarrier);
	}

	/**
	 * Creates a gateway for the new rooms created by the initial gateways.
	 * 
	 * @param startPoint
	 *            Travelling clockwise around the room, this is the end of the
	 *            room adjacent to the start gateway we're creating
	 * @param endPoint
	 *            This is the end of the room adjacent to the end of the
	 *            gateway.
	 * @param direction
	 *            This is the side of the room the gateway is on, or
	 *            (alternatively), what direction we have to turn to face the
	 *            gateway.
	 * @param walkDirection
	 *            This is the direction we go to walk down the gateway (parallel
	 *            to the room)
	 * @param roomNumber
	 *            This is the id number of the room we're in
	 * @param barrierList
	 *            This is the list of barriers for the room we're in
	 */
	private void doNewGateway(int[] startPoint, int[] endPoint,
			Direction direction, Direction walkDirection, int roomNumber,
			List<RoomBarrier> barrierList) {
		// next is the gateway to the left of our left endpoint
		RoomBarrier currentBarrier = new RoomBarrier();
		currentBarrier.gateway = true;
		currentBarrier.left = Direction.translate(startPoint, direction,
				new int[2]);

		// get a new gateway id
		currentBarrier.id = counts.nextGateway();

		// the direction is the direction we just traveled to get to the gateway
		currentBarrier.direction = direction;

		// this is a gateway of unknown size
		currentBarrier.right = Arrays.copyOf(currentBarrier.left,
				currentBarrier.left.length);

		// we know it ends left of the right endpoint
		int[] endOfGateway = Direction.translate(endPoint, direction,
				new int[2]);

		while (true) {
			// create the gateway object
			CellObject gatewayObject = getData().cellObjectManager
					.createObject(Names.kGatewayID);
			gatewayObject.removeProperty(Names.kPropertyGatewayRender);

			// gateway don't share ids, they are noted by the direction of the
			// gateway
			gatewayObject.setProperty(currentBarrier.direction.id(), Integer
					.toString(currentBarrier.id));

			// put the object in the cell
			Cell cell = getData().cells.getCell(currentBarrier.right);
			cell.addObject(gatewayObject);

			// record the destinations which is the new room and the room the
			// gateway is sitting on
			// add the current room to the gateway destination list
			List<Integer> gatewayDestinations = new ArrayList<Integer>();
			gatewayDestinations.add(new Integer(roomNumber));
			gatewayDestinations.add(new Integer(cell
					.getFirstObjectWithProperty(Names.kRoomID).getProperty(
							Names.kPropertyNumber, -1, Integer.class)));
			gatewayDestinationMap.put(new Integer(currentBarrier.id),
					gatewayDestinations);

			if (Arrays.equals(currentBarrier.right, endOfGateway)) {
				break;
			}

			// increment and loop
			currentBarrier.right = Direction.translate(currentBarrier.right,
					walkDirection);
		}

		// store the barrier
		barrierList.add(currentBarrier);
	}

	private void generateCenterpoints(int roomNumber,
			List<RoomBarrier> barrierList) {
		logger.info("Room " + roomNumber + ":");
		Iterator<RoomBarrier> iter = barrierList.iterator();
		while (iter.hasNext()) {
			RoomBarrier barrier = iter.next();

			// generate centerpoint
			barrier.centerpoint();

			logger.info(barrier);
		}
	}

	void addDestinationToGateway(int roomNumber, int gatewayId) {
		List<Integer> gatewayDestinations = gatewayDestinationMap
				.get(new Integer(gatewayId));
		assert gatewayDestinations != null;
		gatewayDestinations.add(new Integer(roomNumber));
		gatewayDestinationMap.put(new Integer(gatewayId), gatewayDestinations);
	}
	
	private GridMapData data;
	private final String mapPath;

	protected GridMapData getData() {
		return data;
	}
	
	protected String getMapPath() {
		return mapPath;
	}
	
	public boolean isInBounds(int[] xy) {
		return data.cells.isInBounds(xy);
	}
	
	public String getCurrentMapName() {
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

	public int size() {
		return data.cells.size();
	}

	public int[] getAvailableLocationAmortized() {
		int size = size();
		
		// Loop in case there are no free spots, the 100 is totally arbitrary
		int [] xy = new int [2];
		for (int counter = 0; counter < 100; ++counter) {
			xy[0] = Simulation.random.nextInt(size - 2) + 1;
			xy[1] = Simulation.random.nextInt(size - 2) + 1;
			
			if (isAvailable(xy)) {
				return xy;
			}
		}
		List<int []> locations = new ArrayList<int []>();
		for (xy[0] = 0; xy[0] < size; ++xy[0]) {
			for (xy[1] = 0; xy[1] < size; ++ xy[1]) {
				if (isAvailable(xy)) {
					locations.add(xy);
				}
			}
		}
		if (locations.size() == 0) {
			return null;
		}
		return locations.get(Simulation.random.nextInt(locations.size()));
	}

	public CellObject createObjectByName(String name) {
		return data.cellObjectManager.createObject(name);
	}

	public List<CellObject> getTemplatesWithProperty(String name) {
		return data.cellObjectManager.getTemplatesWithProperty(name);
	}

	public List<CellObject> getAllTemplates() {
		return data.cellObjectManager.getTemplatesWithProperty(null);
	}

	protected boolean reload() {
		data = new GridMapData();
		
		File mapFile = new File(mapPath);
		if (!mapFile.exists()) {
			throw new IllegalStateException("Map file doesn't exist during reload");
		}

		data.cellObjectManager = new CellObjectManager();
		
		String mapFilePath = mapFile.getAbsolutePath();
		try {
			Config mapConfig = new Config(new ConfigFile(mapFilePath));
			String objectsFileString = mapConfig.getString("objects_file");
			String mapFileDirectory = mapFilePath.substring(0, mapFilePath.lastIndexOf(File.separatorChar) + 1);
			
			Config objectsConfig = new Config(new ConfigFile(mapFileDirectory + objectsFileString));
			
			for (String id : mapConfig.getStrings("objects")) {
				CellObject template = new CellObject(objectsConfig.getChild("objects." + id));
				data.cellObjectManager.registerTemplate(template);
				String name = template.getProperty("name");
				String fileString = template.getProperty("image");
				if (name != null && fileString != null) {
					this.simConfig.mapImage(name, fileString);
				}
			}
			
			cellsConfig(mapConfig.getChild("cells"), objectsConfig);
			return true;
		} 
		catch (IOException e) {
			e.printStackTrace();
			throw new IllegalStateException("During reload: " + e.getMessage(), e);
		} 
		catch (ParseError e) {
			e.printStackTrace();
			throw new IllegalStateException("During reload: " + e.getMessage(), e);
		}
		catch (IllegalStateException e) {
			e.printStackTrace();
			throw new IllegalStateException("During reload: " + e.getMessage(), e);
		} 
		catch (IndexOutOfBoundsException e) {
			e.printStackTrace();
			throw new IllegalStateException("During reload: " + e.getMessage(), e);
		}
	}

	/**
	 * @param cellsConfig
	 * @param objectsConfig
	 * 
	 * @throws IndexOutOfBoundsException If rows do not contain enough cell data
	 * @throws IllegalArgumentException If encountered object that isn't registered
	 */
	private void cellsConfig(Config cellsConfig, Config objectsConfig) {
		data.cells = new GridMapCells(cellsConfig.requireInt("size"), new CellObjectObserver[] { data, this });
		
		// Generate map unless both are true
		Config rows = cellsConfig.getChild("rows");
		int[] xy = new int[2];
		for (xy[1] = 0; xy[1] < data.cells.size(); ++xy[1]) {

			String[] cellStrings = rows.getStrings(Integer.toString(xy[1]));
			if (cellStrings.length != data.cells.size()) {
				throw new IndexOutOfBoundsException("Not enough cells, row " + xy[1]);
			}
			
			for (xy[0] = 0; xy[0] < data.cells.size(); ++xy[0]) {
				String contents = cellStrings[xy[0]];
				for (String objectID : contents.split("-")) {
					String objectName = objectsConfig.getString("objects." + objectID + ".name");
					logger.trace(Arrays.toString(xy) + ": " + objectName);
					
					if (!data.cellObjectManager.hasTemplate(objectName)) {
						throw new IllegalArgumentException("object \"" + objectName + "\" does not map to a cell object");
					}
					
					CellObject cellObject = data.cellObjectManager.createObject(objectName);
					data.cells.getCell(xy).addObject(cellObject);
				}
			}
		}
	}
	
	protected void lingerUpdate(CellObject cellObject, Cell cell) {
		if (cellObject.hasProperty("update.linger")) {
			int linger = cellObject.getProperty("update.linger", 0, Integer.class);
			linger -= 1;
			if (linger <= 0) {
				cell.removeObject(cellObject);
			} else {
				cellObject.setProperty("update.linger", linger);
			}
		}
	}
	
	public Cell getCell(int[] xy) {
		return data.cells.getCell(xy);
	}
}
