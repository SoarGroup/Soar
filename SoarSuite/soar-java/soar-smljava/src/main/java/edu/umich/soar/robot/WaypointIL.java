package edu.umich.soar.robot;

import jmat.LinAlg;
import jmat.MathUtil;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;

final class WaypointIL {
	private final double[] xyz = new double[3];
	private final String name;
	private final Identifier waypoints;
	private final OffsetPose opose;
	
	private Identifier waypoint;
	private FloatElement distance;
	private YawWmes yawWmes;
	private double yawValueRad = 0;
	private double relativeBearingValueRad = 0;
	
	WaypointIL(double[] waypointxyz, String name, Identifier waypoints, 
			boolean useFloatWmes, OffsetPose opose) {
		System.arraycopy(waypointxyz, 0, this.xyz, 0, waypointxyz.length);
		this.name = new String(name);
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
		double distanceValue = LinAlg.distance(opose.getPose().pos, xyz, 2);
		double[] delta = LinAlg.subtract(xyz, opose.getPose().pos);
		yawValueRad = Math.atan2(delta[1], delta[0]);
		relativeBearingValueRad = yawValueRad - LinAlg.quatToRollPitchYaw(opose.getPose().orientation)[2];
		relativeBearingValueRad = MathUtil.mod2pi(relativeBearingValueRad);

		if (waypoint == null) {
			waypoint = waypoints.CreateIdWME("waypoint");
			
			waypoint.CreateStringWME("id", name);
			waypoint.CreateFloatWME("x", xyz[0]);
			waypoint.CreateFloatWME("y", xyz[1]);
			waypoint.CreateFloatWME("z", xyz[2]);

			distance = waypoint.CreateFloatWME("distance", 0);

			yawWmes.create();
		}
		
		distance.Update(distanceValue);
		
		double yawValueDeg = Math.toDegrees(yawValueRad);
		double relativeBearingValueDeg = Math.toDegrees(relativeBearingValueRad);
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
}

