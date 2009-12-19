package edu.umich.soar.sproom.control;

import java.util.Arrays;

import lcmtypes.pose_t;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;

class WaypointIL {
	private final String name;
	private final String type;
	private final Identifier waypoints;
	private final OffsetPose opose;
	
	private Identifier waypoint;
	private PointDataIL pointData;
	
	WaypointIL(double[] waypointxyz, String name, String type, 
			Identifier waypoints, boolean useFloatWmes, OffsetPose opose) {
		this.name = name;
		this.type = type;
		this.waypoints = waypoints;
		this.opose = opose;
		this.pointData = new PointDataIL(waypointxyz, )
		
		yawWmes = useFloatWmes ? new YawFloatWmes() : new YawIntWmes();
	}

	boolean equals(String other) {
		return other.equals(name);
	}

	void update() {
		pose_t selfPose = opose.getPose();
		PointRelationship r = PointRelationship.calculate(selfPose, pos);

		if (waypoint == null) {
			waypoint = waypoints.CreateIdWME("waypoint");
			
			if (type.equals("int")) {
				waypoint.CreateIntWME("id", Integer.parseInt(name));
			} else if (type.equals("float")) {
				waypoint.CreateFloatWME("id", Double.parseDouble(name));
			} else {
				waypoint.CreateStringWME("id", name);
			}
			waypoint.CreateFloatWME("x", pos[0]);
			waypoint.CreateFloatWME("y", pos[1]);
			waypoint.CreateFloatWME("z", pos[2]);

			distance = waypoint.CreateFloatWME("distance", 0);

			yawWmes.create();
		}
		
		distance.Update(r.getDistance());
		
		double yawValueDeg = Math.toDegrees(r.getYaw());
		double relativeBearingValueDeg = Math.toDegrees(r.getRelativeBearing());
		double absRelativeBearingValueDeg = Math.abs(relativeBearingValueDeg);

		yawWmes.update(yawValueDeg, relativeBearingValueDeg, absRelativeBearingValueDeg);
	}

	void disable() {
		waypoint.DestroyWME();
		waypoint = null;
	}
	
	boolean isDisabled() {
		return waypoint == null;
	}
	
	double[] getPos() {
		return Arrays.copyOf(pos, pos.length);
	}
}

