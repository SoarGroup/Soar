package soar2d.map;

import java.awt.Point;
import java.awt.geom.Point2D;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Soar2D;
import soar2d.configuration.Configuration;
import soar2d.configuration.Configuration.SimType;
import soar2d.world.TankSoarWorld;

public class BookMap extends GridMap {

	public BookMap(Configuration config) {
		super(config);
	}

	@Override
	public void addObjectToCell(java.awt.Point location, CellObject object) {
		Cell cell = getCell(location);
		if (cell.hasObject(object.getName())) {
			CellObject old = cell.removeObject(object.getName());
			assert old != null;
			updatables.remove(old);
			updatablesLocations.remove(old);
			if (isBookObject(old)) {
				bookObjects.remove(old);
				bookObjectInfo.remove(old);
			}
			removalStateUpdate(old);
		}
		if (object.updatable()) {
			updatables.add(object);
			updatablesLocations.put(object, location);
		}
		if (isBookObject(object)) {
			bookObjects.add(object);
			BookObjectInfo info = new BookObjectInfo();
			info.location = new Point(location);
			info.floatLocation = new Point2D.Double();
			info.floatLocation.x = info.location.x * Soar2D.bConfig.getBookCellSize();
			info.floatLocation.y = info.location.y * Soar2D.bConfig.getBookCellSize();
			info.floatLocation.x += Soar2D.bConfig.getBookCellSize() / 2.0;
			info.floatLocation.y += Soar2D.bConfig.getBookCellSize() / 2.0;
			info.object = object;
			if (!object.hasProperty("object-id")) {
				object.addProperty("object-id", Integer.toString(newObjectId()));
			}
			if (getAllWithProperty(info.location, Names.kPropertyNumber).size() > 0) {
				info.area = getAllWithProperty(info.location, Names.kPropertyNumber).get(0).getIntProperty(Names.kPropertyNumber);
			}
			bookObjectInfo.put(object, info);
		}
		
		cell.addCellObject(object);
		setRedraw(cell);
	}

	public boolean isBookObject(CellObject co) {
		if (co.name.equals("mblock")) {
			return true;
		}
		return false;
	}
	
	HashSet<CellObject> bookObjects = new HashSet<CellObject>();
	HashMap<CellObject, BookObjectInfo> bookObjectInfo = new HashMap<CellObject, BookObjectInfo>();
	public HashSet<CellObject> getBookObjects() {
		return bookObjects;
	}
	public BookObjectInfo getBookObjectInfo(CellObject obj) {
		return bookObjectInfo.get(obj);
	}

	@Override
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
		if (isBookObject(object)) {
			bookObjects.remove(object);
			bookObjectInfo.remove(object);
		}
		removalStateUpdate(object);
		
		return object;
	}

	@Override
	public void removeAllWithProperty(java.awt.Point location, String name) {
		Cell cell = getCell(location);
		
		cell.iter = cell.cellObjects.values().iterator();
		CellObject cellObject;
		while (cell.iter.hasNext()) {
			cellObject = cell.iter.next();
			if (name == null || cellObject.hasProperty(name)) {
				if (cellObject.updatable()) {
					updatables.remove(cellObject);
					updatablesLocations.remove(cellObject);
				}
				if (isBookObject(cellObject)) {
					bookObjects.remove(cellObject);
					bookObjectInfo.remove(cellObject);
				}
				cell.iter.remove();
				removalStateUpdate(cellObject);
			}
		}
	}

	@Override
	public void updateObjects(TankSoarWorld tsWorld) {
		if (!updatables.isEmpty()) {
			Iterator<CellObject> iter = updatables.iterator();
			
			while (iter.hasNext()) {
				CellObject cellObject = iter.next();
				java.awt.Point location = updatablesLocations.get(cellObject);
				assert location != null;
				
				if (cellObject.update(location)) {
					Cell cell = getCell(location);
					
					cellObject = cell.removeObject(cellObject.getName());
					assert cellObject != null;
					
					setRedraw(cell);
					
					iter.remove();
					updatablesLocations.remove(cellObject);
					removalStateUpdate(cellObject);
				}
			}
		}
	}

	public int getLocationId(Point location) {
		assert location != null;
		assert config.getType() == SimType.kBook;

		ArrayList<CellObject> locationObjects = getAllWithProperty(location, Names.kPropertyNumber);
		assert locationObjects.size() == 1;
		return locationObjects.get(0).getIntProperty(Names.kPropertyNumber);
	}

	@Override
	public boolean isAvailable(java.awt.Point location) {
		Cell cell = getCell(location);
		boolean enterable = cell.enterable();
		boolean noPlayer = cell.getPlayer() == null;
		boolean noMBlock = cell.getAllWithProperty("mblock").size() <= 0;
		return enterable && noPlayer && noMBlock;
	}
	
	private int roomCount = 0;
	private int gatewayCount = 0;
	private int wallCount = 0;
	private int objectCount = 0;
	
	public class Barrier {
		public int id = -1;
		public boolean gateway = false;
		
		public java.awt.Point left;
		public java.awt.Point right;
		
		public int direction;

		private Point2D.Double centerpoint;
		public Point2D.Double centerpoint() {
			//  IMPORTANT! Assumes left/right won't change
			if (centerpoint != null) {
				return centerpoint;
			}
			
			int m = 0;
			int n = 0;
			centerpoint = new Point2D.Double(0,0);
			
			switch (direction) {
			case Direction.kNorthInt:
			case Direction.kSouthInt:
				// horizontal
				m = left.x;
				n = right.x;
				centerpoint.y = left.y * Soar2D.bConfig.getBookCellSize();
				break;
			case Direction.kEastInt:
			case Direction.kWestInt:
				// vertical
				m = left.y;
				n = right.y;
				centerpoint.x = left.x * Soar2D.bConfig.getBookCellSize();
				break;
			}
			
			int numberOfBlocks = n - m;
			java.awt.Point upperLeft = left;
			// take abs, also note that if negative then we need to calculate center from the upper-left block
			// which this case is the "right"
			if (numberOfBlocks < 0) {
				numberOfBlocks *= -1;
				upperLeft = right;
			}
			numberOfBlocks += 1; // endpoints 0,0 and 0,3 represent 4 blocks
			
			if (left.x == right.x) {
				// vertical
				// add half to y
				centerpoint.y = upperLeft.y * Soar2D.bConfig.getBookCellSize();
				centerpoint.y += (numberOfBlocks / 2.0) * Soar2D.bConfig.getBookCellSize();
				
				// if west, we gotta add a cell size to x
				if (direction == Direction.kWestInt) {
					centerpoint.x += Soar2D.bConfig.getBookCellSize();
				}
				
			} else {
				// horizontal
				// add half to x
				centerpoint.x = upperLeft.x * Soar2D.bConfig.getBookCellSize();
				centerpoint.x += (numberOfBlocks / 2.0) * Soar2D.bConfig.getBookCellSize();

				// if north, we gotta add a cell size to y
				if (direction == Direction.kNorthInt) {
					centerpoint.y += Soar2D.bConfig.getBookCellSize();
				}
			}
			return centerpoint;
		}
		
		@Override
		public String toString() {
			String output = new String(Integer.toString(id));
			output += " (" + Direction.stringOf[direction] + ")";
			Point2D.Double center = centerpoint();
			output += " (" + Integer.toString(left.x) + "," + Integer.toString(left.y) + ")-(" 
					+ Double.toString(center.x) + "," + Double.toString(center.y) + ")-(" 
					+ Integer.toString(right.x) + "," + Integer.toString(right.y) + ")";
			if (gateway) {
				output += " (gateway)";
			}
			return output;
		}
	}
	
	// Mapping of room id to the list of the barriers surrounding that room
	private HashMap<Integer, ArrayList<Barrier> > roomBarrierMap = new HashMap<Integer, ArrayList<Barrier> >();
	public ArrayList<Barrier> getRoomBarrierList(int roomID) {
		return roomBarrierMap.get(roomID);
	}

	// Mapping of gateway id to the list of the ids of rooms it connects
	private HashMap<Integer, ArrayList<Integer> > gatewayDestinationMap = new HashMap<Integer, ArrayList<Integer> >();
	public ArrayList<Integer> getGatewayDestinationList(int gatewayID) {
		return gatewayDestinationMap.get(gatewayID);
	}
	
	public boolean generateRoomStructure() {
		// Start in upper-left corner
		// if cell is enterable, flood fill to find boundaries of room
		// Go from left to right, then to the start of the next line
		LinkedList<java.awt.Point> floodQueue = new LinkedList<java.awt.Point>();
		HashSet<java.awt.Point> explored = new HashSet<java.awt.Point>((this.size-2)*2);
		
		// this is where we will store gateway barriers for conversion to rooms 
		// in the second phase of map structure generation. 
		// this will contain duplicates since two different gateways can 
		// be represented by the same squares
		ArrayList<Barrier> gatewayBarriers = new ArrayList<Barrier>();

		java.awt.Point location = new java.awt.Point();
		for (location.y = 1; location.y < (this.size - 1); ++location.y) {
			for (location.x = 1; location.x < (this.size - 1); ++location.x) {
				if (explored.contains(location)) {
					continue;
				}
				explored.add(location);
				
				Cell cell = getCell(location);
				if (!cell.enterable() || cell.hasObject(Names.kPropertyGateway)) {
					continue;
				}
				
				assert cell.getObject(Names.kRoomID) == null;

				// cell is enterable, we have a room
				int roomNumber = roomCount + gatewayCount + wallCount + objectCount;
				roomCount += 1;
				
				CellObject roomObject = cellObjectManager.createObject(Names.kRoomID);
				roomObject.addProperty(Names.kPropertyNumber, Integer.toString(roomNumber));

				HashSet<java.awt.Point> floodExplored = new HashSet<java.awt.Point>((this.size-2)*2);
				floodExplored.add(location);
				cell.addCellObject(roomObject);
				//System.out.print("Room " + roomNumber + ": (" + location.x + "," + location.y + ") ");
				
				// add the surrounding cells to the queue 
				floodQueue.add(new java.awt.Point(location.x+1,location.y));
				floodQueue.add(new java.awt.Point(location.x,location.y+1));
				floodQueue.add(new java.awt.Point(location.x-1,location.y));
				floodQueue.add(new java.awt.Point(location.x,location.y-1));
				
				// flood and mark all room cells and save walls
				HashSet<java.awt.Point> walls = new HashSet<java.awt.Point>();
				while(floodQueue.size() > 0) {
					java.awt.Point floodLocation = floodQueue.removeFirst();

					if (floodExplored.contains(floodLocation)) {
						continue;
					}
					floodExplored.add(floodLocation);
					
					cell = getCell(floodLocation);
					if (!cell.enterable() || cell.hasObject(Names.kPropertyGateway)) {
						walls.add(floodLocation);
						continue;
					}

					explored.add(floodLocation);

					cell.addCellObject(new CellObject(roomObject));
					//System.out.print("(" + floodLocation.x + "," + floodLocation.y + ") ");

					// add the four surrounding cells to the queue
					floodQueue.add(new java.awt.Point(floodLocation.x+1,floodLocation.y));
					floodQueue.add(new java.awt.Point(floodLocation.x-1,floodLocation.y));
					floodQueue.add(new java.awt.Point(floodLocation.x,floodLocation.y+1));
					floodQueue.add(new java.awt.Point(floodLocation.x,floodLocation.y-1));
				}
				
				// figure out walls going clockwise starting with the wall north of the first square in the room
				int direction = Direction.kEastInt;
				java.awt.Point startingWall = new java.awt.Point(location.x, location.y-1);
				java.awt.Point next = new java.awt.Point(startingWall);
				
				// Keep track of the current barrier information. When the wall ends, simply add it to the the room
				// barrier map.
				Barrier currentBarrier = null;
				
				// Keep track of barrier information
				ArrayList<Barrier> barrierList = new ArrayList<Barrier>();
				
				// I probably should have commented this more when I wrote it.
				// The comments have been inserted after the initial writing so they may be slightly wrong.
				
				while (true) {
					
					// This is used to figure out how to turn corners when walking along walls.
					// Also used with figuring out barrier endpoints.
					java.awt.Point previous = new java.awt.Point(next);
					
					// next is actually the location we're examining now
					cell = getCell(next);
					
					// used to detect turns
					java.awt.Point rightOfNext = null;
					
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
							currentBarrier.left = new java.awt.Point(next);
							currentBarrier.direction = Direction.leftOf[direction];
							
							// create a new gateway id
							currentBarrier.id = roomCount + gatewayCount + wallCount + objectCount;
							gatewayCount += 1;

							//System.out.println();
							//System.out.print("  Gateway " + currentBarrier.id + ": ");
							
							// add the current room to the gateway destination list
							ArrayList<Integer> gatewayDestinations = gatewayDestinationMap.get(new Integer(currentBarrier.id));
							if (gatewayDestinations == null) {
								gatewayDestinations = new ArrayList<Integer>();
							}
							gatewayDestinations.add(new Integer(roomNumber));
							gatewayDestinationMap.put(new Integer(currentBarrier.id), gatewayDestinations);
						}

						// is are noted by the direction of the wall
						gatewayObject.addProperty(Direction.stringOf[currentBarrier.direction], Integer.toString(currentBarrier.id));
						
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
							currentBarrier.left = new java.awt.Point(next);
							currentBarrier.direction = Direction.leftOf[direction];
							
							// create a new id
							currentBarrier.id = roomCount + gatewayCount + wallCount + objectCount;
							wallCount += 1;
							
							//System.out.println();
							//System.out.print("  Wall " + currentBarrier.id + ": (" + currentBarrier.direction + "): ");
						}
						
						// is are noted by the direction of the wall
						wallObject.addProperty(Direction.stringOf[currentBarrier.direction], Integer.toString(currentBarrier.id));

					} else {
						// the world is ending, check the asserts
						assert false;
					}
					
					//System.out.print("(" + next.x + "," + next.y + ") ");

					// since current barrier is the gateway we're walking, update it's endpoint before we translate it
					currentBarrier.right = new java.awt.Point(next);
					
					// walk to the next section of wall
					Direction.translate(next, direction);
					
					// we get the right of next here because if there is a next and
					// a wall to the right of it, that means next is a gateway but there is
					// a wall in the way so that gateway doesn't technically border our room,
					// so, the gateway ends and we continue on the next segment of wall.
					rightOfNext = new java.awt.Point(next);
					Direction.translate(rightOfNext, Direction.rightOf[direction]);

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
					Direction.translate(next, Direction.rightOf[direction]);
					if (walls.contains(next)) {
						// right worked
						direction = Direction.rightOf[direction];

					} else {
						// try turning left next
						next = new java.awt.Point(previous);
						Direction.translate(next, Direction.leftOf[direction]);
						
						if (walls.contains(next)) {
							// left worked
							direction = Direction.leftOf[direction];
							
							// need to stay on previous because it is included on the new wall
							next = previous;
							// the remove will silently fail and that's ok
							
						} else {
							// single length wall (perform "left" turn)
							direction = Direction.leftOf[direction];

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
		ArrayList<Integer> gatewayDestinations = gatewayDestinationMap.get(new Integer(gatewayId));
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
	private void doNewWall(java.awt.Point endPoint, int direction, ArrayList<Barrier> barrierList) {
		Barrier currentBarrier = new Barrier();
		currentBarrier.left = new java.awt.Point(endPoint);
		currentBarrier.left.x += Direction.xDelta[direction];
		currentBarrier.left.y += Direction.yDelta[direction];

		// this is a wall and is only size 1
		currentBarrier.right = new java.awt.Point(currentBarrier.left);
		
		// get a new wall id
		currentBarrier.id = roomCount + gatewayCount + wallCount + objectCount;
		wallCount += 1;
		// the direction is the direction we just traveled to get to the wall
		currentBarrier.direction = direction;
		
		// get the wall object
		Cell cell = getCell(currentBarrier.left);
		CellObject wallObject = cell.getObject(Names.kWallID);
		
		// walls don't share ids, they are noted by the direction of the wall
		wallObject.addProperty(Direction.stringOf[currentBarrier.direction], Integer.toString(currentBarrier.id));

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
	private void doNewGateway(java.awt.Point startPoint, java.awt.Point endPoint, int direction, int walkDirection, int roomNumber, ArrayList<Barrier> barrierList) {
		// next is the gateway to the left of our left endpoint
		Barrier currentBarrier = new Barrier();
		currentBarrier.gateway = true;
		currentBarrier.left = new java.awt.Point(startPoint);
		currentBarrier.left.x += Direction.xDelta[direction];
		currentBarrier.left.y += Direction.yDelta[direction];

		// get a new gateway id
		currentBarrier.id = roomCount + gatewayCount + wallCount + objectCount;
		gatewayCount += 1;
		// the direction is the direction we just traveled to get to the gateway
		currentBarrier.direction = direction;

		// this is a gateway of unknown size
		currentBarrier.right = new java.awt.Point(currentBarrier.left);
		
		// we know it ends left of the right endpoint
		java.awt.Point endOfGateway = new java.awt.Point(endPoint);
		endOfGateway.x += Direction.xDelta[direction];
		endOfGateway.y += Direction.yDelta[direction];
		
		while (true) {
			// create the gateway object
			CellObject gatewayObject = cellObjectManager.createObject(Names.kGatewayID);
			gatewayObject.removeProperty(Names.kPropertyGatewayRender);

			// gateway don't share ids, they are noted by the direction of the gateway
			gatewayObject.addProperty(Direction.stringOf[currentBarrier.direction], Integer.toString(currentBarrier.id));

			// put the object in the cell
			Cell cell = getCell(currentBarrier.right);
			cell.addCellObject(gatewayObject);
			
			// record the destinations which is the new room and the room the gateway is sitting on
			// add the current room to the gateway destination list
			ArrayList<Integer> gatewayDestinations = new ArrayList<Integer>();
			gatewayDestinations.add(new Integer(roomNumber));
			gatewayDestinations.add(new Integer(cell.getObject(Names.kRoomID).getIntProperty(Names.kPropertyNumber)));
			gatewayDestinationMap.put(new Integer(currentBarrier.id), gatewayDestinations);
			
			if (currentBarrier.right.equals(endOfGateway)) {
				break;
			}
			
			// increment and loop
			currentBarrier.right.x += Direction.xDelta[walkDirection];
			currentBarrier.right.y += Direction.yDelta[walkDirection];
		}

		// store the barrier
		barrierList.add(currentBarrier);
	}	

	
	private void gatewaysToAreasStep(ArrayList<Barrier> gatewayBarriers) {
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
				cell.addCellObject(theNewRoomObject);
			}

			int incrementDirection = -1;
			if (gatewayBarrier.left.x == gatewayBarrier.right.x) {
				// vertical gateway
				if (gatewayBarrier.left.y < gatewayBarrier.right.y) {
					// increasing to right, south
					incrementDirection = Direction.kSouthInt;

				} else if (gatewayBarrier.left.y > gatewayBarrier.right.y) {
					// decreasing to right, north
					incrementDirection = Direction.kNorthInt;
				}
			} else {
				// horizontal gateway
				if (gatewayBarrier.left.x < gatewayBarrier.right.x) {
					// increasing to right, east
					incrementDirection = Direction.kEastInt;

				} else if (gatewayBarrier.left.x > gatewayBarrier.right.x) {
					// decreasing to right, west
					incrementDirection = Direction.kWestInt;
				}
			}
			
			if (incrementDirection == -1) {
				// Direction depends on which ways have walls and which ways have rooms
				java.awt.Point feeler;
				feeler = new java.awt.Point(gatewayBarrier.left);
				feeler.x -= 1;
				Cell cell = getCell(feeler);
				if (cell.getObject(Names.kWallID) != null) {
					// horizontal gateway
					incrementDirection = Direction.kEastInt;
				} else {
					// vertical gateway
					incrementDirection = Direction.kSouthInt;
				}
			}
			
			// we need to walk to the right and assing the room id to everyone
			java.awt.Point current = new java.awt.Point(gatewayBarrier.left);
			while (current.equals(gatewayBarrier.right) == false) {
				current.x += Direction.xDelta[incrementDirection];
				current.y += Direction.yDelta[incrementDirection];

				Cell cell = getCell(current);
				cell.addCellObject(new CellObject(theNewRoomObject));
			}

			// now we need to round up the four barriers
			ArrayList<Barrier> barrierList = new ArrayList<Barrier>();
			
			////////////////////
			// we can start by walking the wrong direction off the left endpoint
			doNewWall(gatewayBarrier.left, Direction.backwardOf[incrementDirection], barrierList);
			////////////////////
			
			////////////////////
			// then to the left of our left endpoint, and walk down the gateway to the left of the right endpoint
			doNewGateway(gatewayBarrier.left, gatewayBarrier.right, Direction.leftOf[incrementDirection], incrementDirection, roomNumber, barrierList);
			////////////////////
			
			////////////////////
			// next is just off the right endpoint
			doNewWall(gatewayBarrier.right, incrementDirection, barrierList);
			////////////////////

			////////////////////
			// then to the right of our right endpoint, and walk backwards down the gateway to the right of the left endpoint
			doNewGateway(gatewayBarrier.right, gatewayBarrier.left, Direction.rightOf[incrementDirection], Direction.backwardOf[incrementDirection], roomNumber, barrierList);
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
	
	public void generateCenterpoints(int roomNumber, ArrayList<Barrier> barrierList) {
		System.out.println("Room " + roomNumber + ":");
		Iterator<Barrier> iter = barrierList.iterator();
		while (iter.hasNext()) {
			Barrier barrier = iter.next();
			
			// generate centerpoint
			barrier.centerpoint();

			System.out.println(barrier);
		}
	}
	
	public CellObject getInObject(Point location) {
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
	void removalStateUpdate(CellObject object) {
		// nothing to do here
	}
}
