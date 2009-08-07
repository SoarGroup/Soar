package edu.umich.soar.gridmap2d.map;

import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class RoomMapData {
	int roomCount = 0;
	int gatewayCount = 0;
	int wallCount = 0;
	int objectCount = 0;
	// Mapping of gateway id to the list of the ids of rooms it connects
	Map<Integer, List<Integer> > gatewayDestinationMap = new HashMap<Integer, List<Integer> >();
	// Mapping of room id to the list of the barriers surrounding that room
	Map<Integer, List<RoomBarrier> > roomBarrierMap = new HashMap<Integer, List<RoomBarrier> >();
	Set<RoomObject> roomObjects = new HashSet<RoomObject>();

	static void addDestinationToGateway(RoomMapData roomData, int roomNumber, int gatewayId) {
		List<Integer> gatewayDestinations = roomData.gatewayDestinationMap.get(new Integer(gatewayId));
		assert gatewayDestinations != null;
		gatewayDestinations.add(new Integer(roomNumber));
		roomData.gatewayDestinationMap.put(new Integer(gatewayId), gatewayDestinations);
	}
}

