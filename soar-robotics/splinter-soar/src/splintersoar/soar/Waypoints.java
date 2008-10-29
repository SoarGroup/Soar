package splintersoar.soar;

import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.logging.Level;
import java.util.logging.Logger;

import jmat.LinAlg;
import jmat.MathUtil;

import lcm.lcm.LCM;
import lcmtypes.pose_t;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import splintersoar.LCMInfo;
import splintersoar.LogFactory;
import lcmtypes.waypoints_t;
import lcmtypes.xy_t;

/**
 * @author voigtjr
 * Manages waypoint state initiated by the Soar agent.
 */
public class Waypoints {

	Agent agent;
	Identifier waypoints;
	pose_t robotPose;

	HashMap<String, Waypoint> waypointList = new HashMap<String, Waypoint>();

	class Waypoint {
		double[] xyz = new double[3];
		String name;

		Identifier waypoint;
		FloatElement absRelativeBearing;
		FloatElement distance;
		FloatElement relativeBearing;
		FloatElement yaw;

		public Waypoint(double[] waypointxyz, String name) {
			System.arraycopy(waypointxyz, 0, this.xyz, 0, waypointxyz.length);
			this.name = new String(name);

			createWmes();
			updateWmes();
		}

		String getName() {
			return name;
		}

		boolean equals(String other) {
			return other.equals(name);
		}

		void createWmes() {
			waypoint = agent.CreateIdWME(waypoints, "waypoint");
			agent.CreateStringWME(waypoint, "id", name);
			agent.CreateFloatWME(waypoint, "x", xyz[0]);
			agent.CreateFloatWME(waypoint, "y", xyz[1]);

			distance = agent.CreateFloatWME(waypoint, "distance", 0);
			yaw = agent.CreateFloatWME(waypoint, "yaw", 0);
			relativeBearing = agent.CreateFloatWME(waypoint, "relative-bearing", 0);
			absRelativeBearing = agent.CreateFloatWME(waypoint, "abs-relative-bearing", 0);
		}

		void updateWmes() {
			double distanceValue = LinAlg.distance(robotPose.pos, xyz, 2);
			agent.Update(distance, distanceValue);

			double[] delta = LinAlg.subtract(xyz, robotPose.pos);

			double yawValue = Math.atan2(delta[1], delta[0]);
			agent.Update(yaw, Math.toDegrees(yawValue));

			double relativeBearingValue = yawValue - LinAlg.quatToRollPitchYaw(robotPose.orientation)[2];
			relativeBearingValue = MathUtil.mod2pi(relativeBearingValue);

			agent.Update(relativeBearing, Math.toDegrees(relativeBearingValue));
			agent.Update(absRelativeBearing, Math.abs(Math.toDegrees(relativeBearingValue)));
		}

		void enable() {
			if (waypoint != null) {
				return;
			}

			createWmes();
			updateWmes();
		}

		void disable() {
			if (waypoint == null) {
				return;
			}

			agent.DestroyWME(waypoint);

			waypoint = null;
			absRelativeBearing = null;
			distance = null;
			relativeBearing = null;
			yaw = null;
		}
	}

	LCM lcmGG;
	Logger logger;
	
	Waypoints(Agent agent) {
		this.agent = agent;
		logger = LogFactory.createSimpleLogger("Waypoints", Level.INFO);

		try {
			logger.info(String.format("Using %s for %s provider URL.", LCMInfo.GG_NETWORK, LCMInfo.WAYPOINTS_CHANNEL));
			lcmGG = new LCM(LCMInfo.GG_NETWORK);

		} catch (IOException e) {
			logger.severe("Error creating lcmGG.");
			e.printStackTrace();
			System.exit(1);
		}
	}

	void setRootIdentifier(Identifier waypoints) {
		this.waypoints = waypoints;
	}

	public void add(double[] waypointxyz, String name) {
		Waypoint waypoint = waypointList.remove(name);
		if (waypoint != null) {
			waypoint.disable();
		}

		waypointList.put(name, new Waypoint(waypointxyz, name));
	}

	public boolean remove(String name) {
		Waypoint waypoint = waypointList.remove(name);
		if (waypoint == null) {
			return false;
		}
		waypoint.disable();
		return true;
	}

	public boolean enable(String name) {
		Waypoint waypoint = waypointList.get(name);
		if (name == null) {
			return false;
		}

		waypoint.enable();
		return true;
	}

	public boolean disable(String name) {
		Waypoint waypoint = waypointList.get(name);
		if (name == null) {
			return false;
		}

		waypoint.disable();
		return true;
	}

	public void setNewRobotPose(pose_t robotpose) {
		this.robotPose = robotpose;
	}

	public void update() {
		waypoints_t waypoints = new waypoints_t();
		waypoints.utime = System.nanoTime() / 1000;
		waypoints.nwaypoints = waypointList.size();

		if (waypointList.size() != 0) {
			waypoints.names = new String[waypointList.size()];
			waypoints.locations = new xy_t[waypointList.size()];

			// System.out.format( "%16s %10s %10s %10s %10s %10s%n", "name",
			// "x", "y", "distance", "yaw", "bearing" );
			Waypoint[] waypointArray = waypointList.values().toArray(new Waypoint[0]);
			for (int index = 0; index < waypointArray.length; ++index) {
				waypointArray[index].updateWmes();

				waypoints.names[index] = waypointArray[index].name;
				waypoints.locations[index] = new xy_t();
				waypoints.locations[index].utime = waypoints.utime;
				waypoints.locations[index].xy = Arrays.copyOf(waypointArray[index].xyz, 2);
			}
		}
		lcmGG.publish(LCMInfo.WAYPOINTS_CHANNEL, waypoints);
	}

	public void beforeInitSoar() {
		while (waypointList.isEmpty() == false) {
			Iterator<String> iter = waypointList.keySet().iterator();
			remove(iter.next());
		}
	}
}
