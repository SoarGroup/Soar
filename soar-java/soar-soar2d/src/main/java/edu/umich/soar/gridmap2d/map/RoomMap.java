package edu.umich.soar.gridmap2d.map;

import java.io.File;
import java.util.Arrays;
import java.util.List;
import java.util.Set;

import lcmtypes.pose_t;


import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.map.GridMapUtil.Barrier;
import edu.umich.soar.gridmap2d.map.GridMapUtil.RoomMapBuildData;
import edu.umich.soar.gridmap2d.world.RoomWorld;

public class RoomMap implements GridMap, CellObjectObserver {
	//private static Logger logger = Logger.getLogger(RoomMap.class);

	private String mapPath;
	private GridMapData data;
	
	private RoomMapBuildData roomData;

	public static class RoomObjectInfo {
		public CellObject object;
		public int[] location;
		public pose_t pose;
		public int area = -1;
	}
	
	public RoomMap(String mapPath) throws Exception {
		this.mapPath = new String(mapPath);
		
		reset();
	}

	@Override
	public CellObject createObjectByName(String name) {
		return data.cellObjectManager.createObject(name);
	}

	@Override
	public int[] getAvailableLocationAmortized() {
		return GridMapUtil.getAvailableLocationAmortized(this);
	}

	@Override
	public Cell getCell(int[] xy) {
		return data.cells.getCell(xy);
	}

	@Override
	public String getCurrentMapName() {
		return GridMapUtil.getMapName(this.mapPath);
	}

	@Override
	public File getMetadataFile() {
		return data.metadataFile;
	}

	@Override
	public List<CellObject> getTemplatesWithProperty(String name) {
		return data.cellObjectManager.getTemplatesWithProperty(name);
	}

	@Override
	public boolean isAvailable(int[] location) {
		Cell cell = data.cells.getCell(location);
		boolean enterable = !cell.hasAnyWithProperty(Names.kPropertyBlock);
		boolean noPlayer = cell.getPlayer() == null;
		boolean mblock = cell.hasAnyWithProperty(Names.kRoomObjectName);
		return enterable && noPlayer && !mblock;
	}

	@Override
	public boolean isInBounds(int[] xy) {
		return data.cells.isInBounds(xy);
	}

	@Override
	public void reset() throws Exception {
		this.roomData = new RoomMapBuildData();
		this.data = new GridMapData();
		GridMapUtil.loadFromConfigFile(data, mapPath, this);
		if (GridMapUtil.generateRoomStructure(data, roomData) == false) {
			throw new Exception("generateRoomStructure failed");
		}
	}

	public Set<CellObject> getRoomObjects() {
		return roomData.roomObjects;
	}
	
	public RoomObjectInfo getRoomObjectInfo(CellObject obj) {
		return roomData.roomObjectInfoMap.get(obj);
	}

	@Override
	public int size() {
		return data.cells.size();
	}

	@Override
	public void addStateUpdate(int[] location, CellObject added) {
		if (!added.getName().equals(Names.kRoomObjectName)) {
			return;
		}
		if (roomData.roomObjects == null) {
			// building map
			return;
		}
		roomData.roomObjects.add(added);
		RoomObjectInfo info = new RoomObjectInfo();
		info.location = Arrays.copyOf(location, location.length);
		info.pose = new pose_t();
		info.pose.pos[0] = info.location[0] * RoomWorld.CELL_SIZE;
		info.pose.pos[1] = info.location[1] * RoomWorld.CELL_SIZE; 
		info.pose.pos[0] += RoomWorld.CELL_SIZE / 2.0;
		info.pose.pos[1] += RoomWorld.CELL_SIZE / 2.0;
		info.object = added;
		if (!added.hasProperty("object-id")) {
			added.setProperty("object-id", Integer.toString(newObjectId()));
		}
		List<CellObject> numbered = data.cells.getCell(info.location).getAllWithProperty(Names.kPropertyNumber);
		if (numbered != null) {
			info.area = numbered.get(0).getIntProperty(Names.kPropertyNumber, -1);
		}
		roomData.roomObjectInfoMap.put(added, info);
	}

	int newObjectId() {
		int objectNumber = roomData.roomCount + roomData.gatewayCount + roomData.wallCount + roomData.objectCount;
		roomData.objectCount += 1;
		return objectNumber; 
	}
	
	@Override
	public void removalStateUpdate(int[] xy, CellObject object) {
	}

	public int getLocationId(int[] location) {
		assert location != null;

		List<CellObject> locationObjects = data.cells.getCell(location).getAllWithProperty(Names.kPropertyNumber);
		assert locationObjects != null;
		assert locationObjects.size() == 1;
		return locationObjects.get(0).getIntProperty(Names.kPropertyNumber, -1);
	}

	public List<Barrier> getRoomBarrierList(int oldLocationId) {
		return roomData.roomBarrierMap.get(oldLocationId);
	}

	public List<Integer> getGatewayDestinationList(int id) {
		return roomData.gatewayDestinationMap.get(id);
	}

}
