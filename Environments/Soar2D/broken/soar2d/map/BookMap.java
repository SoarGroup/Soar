package broken.soar2d.map;

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

import soar2d.Direction;
import soar2d.Names;
import soar2d.Soar2D;
import soar2d.config.SimConfig;

public class BookMap implements GridMap {

	public BookMap() {
	}

	Set<CellObject> bookObjects = new HashSet<CellObject>();
	Map<CellObject, BookObjectInfo> bookObjectInfo = new HashMap<CellObject, BookObjectInfo>();
	public Set<CellObject> getBookObjects() {
		return bookObjects;
	}
	public BookObjectInfo getBookObjectInfo(CellObject obj) {
		return bookObjectInfo.get(obj);
	}

	public int getLocationId(int [] location) {
		assert location != null;

		List<CellObject> locationObjects = getAllWithProperty(location, Names.kPropertyNumber);
		assert locationObjects != null;
		assert locationObjects.size() == 1;
		return locationObjects.get(0).getIntProperty(Names.kPropertyNumber);
	}

	@Override
	public boolean isAvailable(int [] location) {
		Cell cell = getCell(location);
		boolean enterable = !cell.hasAnyWithProperty(Names.kPropertyBlock);
		boolean noPlayer = cell.getPlayer() == null;
		boolean mblock = cell.hasAnyWithProperty(Names.kBookObjectName);
		return enterable && noPlayer && !mblock;
	}

	public boolean isInBounds(int[] xy) {
		return data.cells.isInBounds(xy);
	}
	
	private int roomCount = 0;
	private int gatewayCount = 0;
	private int wallCount = 0;
	private int objectCount = 0;
	
	public class Barrier {
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
			
			if (left.equals(right) == false) {
				switch (direction) {
				case NORTH:
				case SOUTH:
					// horizontal
					m = left[0];
					n = right[0];
					centerpoint[1] = left[1] * Soar2D.config.roomConfig().cell_size;
					break;
				case EAST:
				case WEST:
					// vertical
					m = left[1];
					n = right[1];
					centerpoint[0] = left[0] * Soar2D.config.roomConfig().cell_size;
					break;
				}
			} else {
				// single block
				centerpoint[0] = left[0] * Soar2D.config.roomConfig().cell_size;
				centerpoint[1] = left[1] * Soar2D.config.roomConfig().cell_size;

				switch (direction) {
				case NORTH:
					centerpoint[1] += Soar2D.config.roomConfig().cell_size;
				case SOUTH:
					centerpoint[0] += Soar2D.config.roomConfig().cell_size / 2;
					break;
				case WEST:
					centerpoint[0] += Soar2D.config.roomConfig().cell_size;
				case EAST:
					centerpoint[1] += Soar2D.config.roomConfig().cell_size / 2;
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
				centerpoint[1] = upperLeft[1] * Soar2D.config.roomConfig().cell_size;
				centerpoint[1] += (numberOfBlocks / 2.0) * Soar2D.config.roomConfig().cell_size;
				
				// if west, we gotta add a cell size to x
				if (direction == Direction.WEST) {
					centerpoint[0] += Soar2D.config.roomConfig().cell_size;
				}
				
			} else {
				// horizontal
				// add half to x
				centerpoint[0] = upperLeft[0] * Soar2D.config.roomConfig().cell_size;
				centerpoint[0] += (numberOfBlocks / 2.0) * Soar2D.config.roomConfig().cell_size;

				// if north, we gotta add a cell size to y
				if (direction == Direction.NORTH) {
					centerpoint[1] += Soar2D.config.roomConfig().cell_size;
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
	
	// Mapping of room id to the list of the barriers surrounding that room
	private Map<Integer, List<Barrier> > roomBarrierMap = new HashMap<Integer, List<Barrier> >();
	public List<Barrier> getRoomBarrierList(int roomID) {
		return roomBarrierMap.get(roomID);
	}

	// Mapping of gateway id to the list of the ids of rooms it connects
	private Map<Integer, List<Integer> > gatewayDestinationMap = new HashMap<Integer, List<Integer> >();
	public List<Integer> getGatewayDestinationList(int gatewayID) {
		return gatewayDestinationMap.get(gatewayID);
	}
	
	public boolean generateRoomStructure() {
		// Start in upper-left corner
		// if cell is enterable, flood fill to find boundaries of room
		// Go from left to right, then to the start of the next line
		Queue<int []> floodQueue = new LinkedList<int []>();
		Set<Integer> explored = new HashSet<Integer>((this.size-2)*2);
		
		// this is where we will store gateway barriers for conversion to rooms 
		// in the second phase of map structure generation. 
		// this will contain duplicates since two different gateways can 
		// be represented by the same squares
		List<Barrier> gatewayBarriers = new ArrayList<Barrier>();

		int [] location = new int [2];
		for (location[1] = 1; location[1] < (this.size - 1); ++location[1]) {
			for (location[0] = 1; location[0] < (this.size - 1); ++location[0]) {
				if (explored.contains(location)) {
					continue;
				}
				explored.add(Arrays.hashCode(location));
				
				Cell cell = getCell(location);
				if (!cell.hasAnyWithProperty(Names.kPropertyBlock) || cell.hasObject(Names.kPropertyGateway)) {
					continue;
				}
				
				assert cell.getObject(Names.kRoomID) == null;

				// cell is enterable, we have a room
				int roomNumber = roomCount + gatewayCount + wallCount + objectCount;
				roomCount += 1;
				
				CellObject roomObject = cellObjectManager.createObject(Names.kRoomID);
				roomObject.addProperty(Names.kPropertyNumber, Integer.toString(roomNumber));

				Set<Integer> floodExplored = new HashSet<Integer>((this.size-2)*2);
				floodExplored.add(Arrays.hashCode(location));
				cell.addObject(roomObject);
				//System.out.print("Room " + roomNumber + ": (" + location[0] + "," + location[1] + ") ");
				
				// add the surrounding cells to the queue 
				floodQueue.add(new int [] { location[0]+1,location[1] });
				floodQueue.add(new int [] { location[0],location[1]+1 });
				floodQueue.add(new int [] { location[0]-1,location[1] });
				floodQueue.add(new int [] { location[0],location[1]-1 });
				
				// flood and mark all room cells and save walls
				Set<Integer> walls = new HashSet<Integer>();
				while(floodQueue.size() > 0) {
					int [] floodLocation = floodQueue.remove();

					if (floodExplored.contains(floodLocation)) {
						continue;
					}
					floodExplored.add(Arrays.hashCode(floodLocation));
					
					cell = getCell(floodLocation);
					if (cell.hasAnyWithProperty(Names.kPropertyBlock) || cell.hasObject(Names.kPropertyGateway)) {
						walls.add(Arrays.hashCode(floodLocation));
						continue;
					}

					explored.add(Arrays.hashCode(floodLocation));

					cell.addObject(new CellObject(roomObject));
					//System.out.print("(" + floodLocation[0] + "," + floodLocation[1] + ") ");

					// add the four surrounding cells to the queue
					floodQueue.add(new int [] { floodLocation[0]+1,floodLocation[1] });
					floodQueue.add(new int [] { floodLocation[0]-1,floodLocation[1] });
					floodQueue.add(new int [] { floodLocation[0],floodLocation[1]+1 });
					floodQueue.add(new int [] { floodLocation[0],floodLocation[1]-1 });
				}
				
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
				
				while (true) {
					
					// This is used to figure out how to turn corners when walking along walls.
					// Also used with figuring out barrier endpoints.
					int [] previous = Arrays.copyOf( next, next.length );
					
					// next is actually the location we're examining now
					cell = getCell(next);
					
					// used to detect turns
					int [] rightOfNext = null;
					
					// Get the wall and gateway objects. The can't both exist.
					CellObject gatewayObject = cell.getObject(Names.kGatewayID);
					CellObject wallObject = cell.getObject(Names.kWallID);
					
					// One must exist, but not both
					assert ((gatewayObject == null) && (wallObject != null)) || ((gatewayObject != null) && (wallObject == null));

					if (gatewayObject != null) {
						if (currentBarrier != null) {
							// If we were just walking a wall, end and add the barrier
							if (currentBarrier.gateway == false) {
								barrierList.add(currentBarrier);
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
							currentBarrier.id = roomCount + gatewayCount + wallCount + objectCount;
							gatewayCount += 1;

							//System.out.println();
							//System.out.print("  Gateway " + currentBarrier.id + ": ");
							
							// add the current room to the gateway destination list
							List<Integer> gatewayDestinations = gatewayDestinationMap.get(new Integer(currentBarrier.id));
							if (gatewayDestinations == null) {
								gatewayDestinations = new ArrayList<Integer>();
							}
							gatewayDestinations.add(new Integer(roomNumber));
							gatewayDestinationMap.put(new Integer(currentBarrier.id), gatewayDestinations);
						}

						// is are noted by the direction of the wall
						gatewayObject.addProperty(currentBarrier.direction.id(), Integer.toString(currentBarrier.id));
						
					} else if (wallObject != null) /*redundant*/ {
					
						if (currentBarrier != null) {
							// If we were just walking a gateway, end and add the barrier
							if (currentBarrier.gateway) {
								// keep track of gateway barriers
								gatewayBarriers.add(currentBarrier);
								barrierList.add(currentBarrier);
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
							currentBarrier.id = roomCount + gatewayCount + wallCount + objectCount;
							wallCount += 1;
							
							//System.out.println();
							//System.out.print("  Wall " + currentBarrier.id + ": (" + currentBarrier.direction + "): ");
						}
						
						// is are noted by the direction of the wall
						wallObject.addProperty(currentBarrier.direction.id(), Integer.toString(currentBarrier.id));

					} else {
						// the world is ending, check the asserts
						assert false;
					}
					
					//System.out.print("(" + next[0] + "," + next[1] + ") ");

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
					if (walls.contains(next) && !walls.contains(rightOfNext)) {
						continue;
					}

					// gateway or wall stops here
					//System.out.print("(turn)");

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
					if (walls.contains(next)) {
						// right worked
						direction = direction.right();

					} else {
						// try turning left next
						next = Arrays.copyOf(previous, previous.length);
						Direction.translate(next, direction.left());
						
						if (walls.contains(next)) {
							// left worked
							direction = direction.left();
							
							// need to stay on previous because it is included on the new wall
							next = previous;
							// the remove will silently fail and that's ok
							
						} else {
							// single length wall (perform "left" turn)
							direction = direction.left();

							// need to stay on previous because it is included on the new wall
							next = previous;
							// the remove will silently fail and that's ok
						}
					}
					
					// See if our turn leads us home
					if (next.equals(startingWall)) {
						break;
					}
				}
				//System.out.println();
				
				// Generate centerpoints and store room information
				generateCenterpoints(roomNumber, barrierList);
				this.roomBarrierMap.put(roomNumber, barrierList);
			}
		}
		
		// convert all the gateways to areas
		gatewaysToAreasStep(gatewayBarriers);
		
		// Assign areas for all objects
		Iterator<BookObjectInfo> infoIter = bookObjectInfo.values().iterator();
		while (infoIter.hasNext()) {
			BookObjectInfo info = infoIter.next();
			
			info.area = getAllWithProperty(info.location, Names.kPropertyNumber).get(0).getIntProperty(Names.kPropertyNumber);
		}
		
		// print gateway information
		Iterator<Integer> gatewayKeyIter = gatewayDestinationMap.keySet().iterator();
		while (gatewayKeyIter.hasNext()) {
			Integer gatewayId = gatewayKeyIter.next();
			String toList = "";
			Iterator<Integer> gatewayDestIter = gatewayDestinationMap.get(gatewayId).iterator();
			while (gatewayDestIter.hasNext()) {
				toList += gatewayDestIter.next() + " ";
			}
			System.out.println("Gateway " + gatewayId + ": " + toList);
		}
		
		return true;
	}

	private void addDestinationToGateway(int roomNumber, int gatewayId) {
		List<Integer> gatewayDestinations = gatewayDestinationMap.get(new Integer(gatewayId));
		assert gatewayDestinations != null;
		gatewayDestinations.add(new Integer(roomNumber));
		gatewayDestinationMap.put(new Integer(gatewayId), gatewayDestinations);
	}
	
	/**
	 * Create a wall for the new rooms created by the initial gateways.
	 * 
	 * @param endPoint The square representing the side of the room butting up to this wall
	 * @param direction The direction to go to reach the wall
	 * @param barrierList The barrier list for our new room
	 */
	private void doNewWall(int [] endPoint, Direction direction, List<Barrier> barrierList) {
		Barrier currentBarrier = new Barrier();
		currentBarrier.left = Direction.translate(endPoint, direction, new int[2]);

		// this is a wall and is only size 1
		currentBarrier.right = Arrays.copyOf(currentBarrier.left, currentBarrier.left.length);
		
		// get a new wall id
		currentBarrier.id = roomCount + gatewayCount + wallCount + objectCount;
		wallCount += 1;
		// the direction is the direction we just traveled to get to the wall
		currentBarrier.direction = direction;
		
		// get the wall object
		Cell cell = getCell(currentBarrier.left);
		CellObject wallObject = cell.getObject(Names.kWallID);
		
		// walls don't share ids, they are noted by the direction of the wall
		wallObject.addProperty(currentBarrier.direction.id(), Integer.toString(currentBarrier.id));

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
	private void doNewGateway(int [] startPoint, int [] endPoint, Direction direction, Direction walkDirection, int roomNumber, List<Barrier> barrierList) {
		// next is the gateway to the left of our left endpoint
		Barrier currentBarrier = new Barrier();
		currentBarrier.gateway = true;
		currentBarrier.left = Direction.translate(startPoint, direction, new int[2]);

		// get a new gateway id
		currentBarrier.id = roomCount + gatewayCount + wallCount + objectCount;
		gatewayCount += 1;
		// the direction is the direction we just traveled to get to the gateway
		currentBarrier.direction = direction;

		// this is a gateway of unknown size
		currentBarrier.right = Arrays.copyOf(currentBarrier.left, currentBarrier.left.length);
		
		// we know it ends left of the right endpoint
		int [] endOfGateway = Direction.translate(endPoint, direction, new int[2]);
		
		while (true) {
			// create the gateway object
			CellObject gatewayObject = cellObjectManager.createObject(Names.kGatewayID);
			gatewayObject.removeProperty(Names.kPropertyGatewayRender);

			// gateway don't share ids, they are noted by the direction of the gateway
			gatewayObject.addProperty(currentBarrier.direction.id(), Integer.toString(currentBarrier.id));

			// put the object in the cell
			Cell cell = getCell(currentBarrier.right);
			cell.addObject(gatewayObject);
			
			// record the destinations which is the new room and the room the gateway is sitting on
			// add the current room to the gateway destination list
			List<Integer> gatewayDestinations = new ArrayList<Integer>();
			gatewayDestinations.add(new Integer(roomNumber));
			gatewayDestinations.add(new Integer(cell.getObject(Names.kRoomID).getIntProperty(Names.kPropertyNumber)));
			gatewayDestinationMap.put(new Integer(currentBarrier.id), gatewayDestinations);
			
			if (currentBarrier.right.equals(endOfGateway)) {
				break;
			}
			
			// increment and loop
			currentBarrier.right = Direction.translate(currentBarrier.right, walkDirection);
		}

		// store the barrier
		barrierList.add(currentBarrier);
	}	

	
	private void gatewaysToAreasStep(List<Barrier> gatewayBarriers) {
		// make the gateway also a room
		// add new room to current gateway destination list
		// create new barrier list for this new room: 2 walls and 2 gateways
		// update gateway destination list for new gateways
		
		Iterator<Barrier> iter = gatewayBarriers.iterator();
		while (iter.hasNext()) {
			Barrier gatewayBarrier = iter.next();
			
			// duplicates exist in this list, check to see if we're already a room
			{
				Cell cell = getCell(gatewayBarrier.left);
				if (cell.hasObject(Names.kRoomID)) {
					// we have already processed this room, just need to add the room id
					// to the gateway's destination list
					addDestinationToGateway(cell.getObject(Names.kRoomID).getIntProperty(Names.kPropertyNumber), gatewayBarrier.id);
					continue;
				}
			}
			
			// get a new room id
			int roomNumber = roomCount + gatewayCount + wallCount + objectCount;
			roomCount += 1;
			
			// add new id to current gateway destination list
			addDestinationToGateway(roomNumber, gatewayBarrier.id);
			
			CellObject theNewRoomObject = cellObjectManager.createObject(Names.kRoomID);
			theNewRoomObject.addProperty(Names.kPropertyNumber, Integer.toString(roomNumber));
			{
				Cell cell = getCell(gatewayBarrier.left);
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
				Cell cell = getCell(feeler);
				if (cell.getObject(Names.kWallID) != null) {
					// horizontal gateway
					incrementDirection = Direction.EAST;
				} else {
					// vertical gateway
					incrementDirection = Direction.SOUTH;
				}
			}
			
			// we need to walk to the right and assing the room id to everyone
			int [] current = Arrays.copyOf(gatewayBarrier.left, gatewayBarrier.left.length);
			while (current.equals(gatewayBarrier.right) == false) {
				current = Direction.translate(current, incrementDirection);

				Cell cell = getCell(current);
				cell.addObject(new CellObject(theNewRoomObject));
			}

			// now we need to round up the four barriers
			List<Barrier> barrierList = new ArrayList<Barrier>();
			
			////////////////////
			// we can start by walking the wrong direction off the left endpoint
			doNewWall(gatewayBarrier.left, incrementDirection.backward(), barrierList);
			////////////////////
			
			////////////////////
			// then to the left of our left endpoint, and walk down the gateway to the left of the right endpoint
			doNewGateway(gatewayBarrier.left, gatewayBarrier.right, incrementDirection.left(), incrementDirection, roomNumber, barrierList);
			////////////////////
			
			////////////////////
			// next is just off the right endpoint
			doNewWall(gatewayBarrier.right, incrementDirection, barrierList);
			////////////////////

			////////////////////
			// then to the right of our right endpoint, and walk backwards down the gateway to the right of the left endpoint
			doNewGateway(gatewayBarrier.right, gatewayBarrier.left, incrementDirection.right(), incrementDirection.backward(), roomNumber, barrierList);
			////////////////////

			// Generate centerpoints and store room information
			generateCenterpoints(roomNumber, barrierList);
			this.roomBarrierMap.put(roomNumber, barrierList);
		}
	}
	
	int newObjectId() {
		int objectNumber = roomCount + gatewayCount + wallCount + objectCount;
		objectCount += 1;
		return objectNumber; 
	}
	
	public void generateCenterpoints(int roomNumber, List<Barrier> barrierList) {
		System.out.println("Room " + roomNumber + ":");
		Iterator<Barrier> iter = barrierList.iterator();
		while (iter.hasNext()) {
			Barrier barrier = iter.next();
			
			// generate centerpoint
			barrier.centerpoint();

			System.out.println(barrier);
		}
	}
	
	public CellObject getInObject(int [] location) {
		if (!this.isInBounds(location)) {
			return null;
		}
		
		CellObject cellObject = getObject(location, Names.kRoomID);
		if (cellObject == null) {
			cellObject = getObject(location, Names.kGatewayID);
			if (cellObject == null) {
				return null;
			}
		}
		
		return cellObject;
	}
	
	@Override
	void addStateUpdate(int [] location, CellObject added) {
		super.addStateUpdate(location, added);
		if (added.getName() != Names.kBookObjectName) {
			return;
		}
		bookObjects.add(added);
		BookObjectInfo info = new BookObjectInfo();
		info.location = Arrays.copyOf(location, location.length);
		info.floatLocation = new double [] { 
				info.location[0] * Soar2D.config.roomConfig().cell_size, 
				info.location[1] * Soar2D.config.roomConfig().cell_size 
				};
		info.floatLocation[0] += Soar2D.config.roomConfig().cell_size / 2.0;
		info.floatLocation[1] += Soar2D.config.roomConfig().cell_size / 2.0;
		info.object = added;
		if (!added.hasProperty("object-id")) {
			added.addProperty("object-id", Integer.toString(newObjectId()));
		}
		List<CellObject> numbered = getAllWithProperty(info.location, Names.kPropertyNumber);
		if (numbered != null) {
			info.area = numbered.get(0).getIntProperty(Names.kPropertyNumber);
		}
		bookObjectInfo.put(added, info);
	}

	@Override
	void removalStateUpdate(CellObject removed) {
		super.removalStateUpdate(removed);
		if (removed.getName() == Names.kBookObjectName) {
			bookObjects.remove(removed);
			bookObjectInfo.remove(removed);
		}
	}

	public CellObject createObjectByName(String name) {
		return data.cellObjectManager.createObject(name);
	}
}
