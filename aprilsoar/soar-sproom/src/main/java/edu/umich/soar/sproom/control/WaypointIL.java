package edu.umich.soar.sproom.control;

import java.util.Arrays;

import lcmtypes.pose_t;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;

final class WaypointIL {
	private final double[] pos = new double[3];
	private final String name;
	private final String type;
	private final Identifier waypoints;
	private final OffsetPose opose;
	
	private Identifier waypoint;
	private FloatElement distance;
	private YawWmes yawWmes;
	
	WaypointIL(double[] waypointxyz, String name, String type, 
			Identifier waypoints, boolean useFloatWmes, OffsetPose opose) {
		System.arraycopy(waypointxyz, 0, this.pos, 0, waypointxyz.length);
		this.name = name;
		this.type = type;
		this.waypoints = waypoints;
		this.opose = opose;
		
		yawWmes = useFloatWmes ? new YawFloatWmes() : new YawIntWmes();
	}

	private interface YawWmes {
		public void create();
		public void update(double yawValueDeg, double relativeBearingValueDeg, double absRelativeBearingValueDeg);
	}
	
	private class YawIntWmes implements YawWmes {
		private IntElement yawWmeI;
		private IntElement relativeBearingWmeI;
		private IntElement absRelativeBearingWmeI;

		public void create() {
			yawWmeI = waypoint.CreateIntWME("yaw", 0);
			relativeBearingWmeI = waypoint.CreateIntWME("relative-bearing", 0);
			absRelativeBearingWmeI = waypoint.CreateIntWME("abs-relative-bearing", 0);
		}
		
		public void update(double yawValueDeg, double relativeBearingValueDeg, double absRelativeBearingValueDeg) {
			yawWmeI.Update((int)Math.round(yawValueDeg));
			relativeBearingWmeI.Update((int)Math.round(relativeBearingValueDeg));
			absRelativeBearingWmeI.Update((int)Math.round(absRelativeBearingValueDeg));
		}
	}

	private class YawFloatWmes implements YawWmes {
		private FloatElement yawWmeF;
		private FloatElement relativeBearingWmeF;
		private FloatElement absRelativeBearingWmeF;

		public void create() {
			yawWmeF = waypoint.CreateFloatWME("yaw", 0);
			relativeBearingWmeF = waypoint.CreateFloatWME("relative-bearing", 0);
			absRelativeBearingWmeF = waypoint.CreateFloatWME("abs-relative-bearing", 0);
		}
		
		public void update(double yawValueDeg, double relativeBearingValueDeg, double absRelativeBearingValueDeg) {
			yawWmeF.Update(yawValueDeg);
			relativeBearingWmeF.Update(relativeBearingValueDeg);
			absRelativeBearingWmeF.Update(absRelativeBearingValueDeg);
		}
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

