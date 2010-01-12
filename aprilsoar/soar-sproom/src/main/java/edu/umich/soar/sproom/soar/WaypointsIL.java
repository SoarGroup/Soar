package edu.umich.soar.sproom.soar;

import java.util.HashMap;
import java.util.Map;

import sml.Identifier;
import edu.umich.soar.FloatWme;
import edu.umich.soar.IntWme;
import edu.umich.soar.StringWme;
import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.Waypoint;
import edu.umich.soar.sproom.command.Waypoints;

public class WaypointsIL implements InputLinkElement {

	private final Identifier root;
	
	private Map<Waypoint, PointDataIL> waypointMap = new HashMap<Waypoint, PointDataIL>();
	
	public WaypointsIL(Identifier root, Adaptable app) {
		this.root = root;
	}

	@Override
	public void update(Adaptable app) {
		Map<Waypoint, PointDataIL> newWaypointMap = new HashMap<Waypoint, PointDataIL>(waypointMap.size());
		Waypoints waypoints = (Waypoints)app.getAdapter(Waypoints.class);
		
		for(Waypoint waypoint : waypoints) {
			PointDataIL pointData = waypointMap.remove(waypoint);
			if (pointData == null) {
				Identifier wme = root.CreateIdWME(SharedNames.WAYPOINT);
				pointData = new PointDataIL(wme, waypoint.getPose().pos);
				
				if (waypoint.getType().equals(SharedNames.INT)) {
					IntWme.newInstance(wme, SharedNames.ID, Integer.parseInt(waypoint.getId()));
				} else if (waypoint.getType().equals(SharedNames.FLOAT)) {
					FloatWme.newInstance(wme, SharedNames.ID, Double.parseDouble(waypoint.getId()));
				} else {
					StringWme.newInstance(wme, SharedNames.ID, waypoint.getId());
				}
			}
			
			pointData.update(app);
			
			newWaypointMap.put(waypoint, pointData);
		}

		for(PointDataIL pointData : waypointMap.values()) {
			pointData.destroy();
		}
		
		waypointMap = newWaypointMap;
	}

}
