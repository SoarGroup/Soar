package splintersoar.soar;

import java.util.logging.Level;
import java.util.logging.Logger;

import erp.geom.Geometry;
import erp.math.MathUtil;
import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import splintersoar.LogFactory;
import splintersoar.lcmtypes.splinterstate_t;
import splintersoar.ranger.RangerState;

public class InputLinkManager {

	private class TimeIL {
		IntElement time_seconds;
		IntElement time_microseconds;

		final static long nanosecondsPerSecond = 1000000000;

		TimeIL(Identifier time) {
			time_seconds = agent.CreateIntWME(time, "seconds", 0);
			time_microseconds = agent.CreateIntWME(time, "microseconds", 0);

			update();
		}

		void update() {
			long current = System.nanoTime();
			int seconds = (int) (current / nanosecondsPerSecond);
			int microseconds = (int) (current % nanosecondsPerSecond);
			microseconds /= 1000;

			agent.Update(time_seconds, seconds);
			agent.Update(time_microseconds, microseconds);
		}
	}

	private class RangerIL {

		class Range {
			int idNumber;

			Identifier range;
			FloatElement start;
			FloatElement end;
			FloatElement distance;

			Range(int id) {
				idNumber = id;

				range = agent.CreateIdWME(ranges, "range");
				agent.CreateIntWME(range, "id", idNumber);

				start = agent.CreateFloatWME(range, "start", 0);
				end = agent.CreateFloatWME(range, "end", 0);
				distance = agent.CreateFloatWME(range, "distance", 0);
			}

			void update(RangerState.RangerData data) {
				agent.Update(start, Math.toDegrees(data.start));
				agent.Update(end, Math.toDegrees(data.end));
				agent.Update(distance, data.distance);
			}
		}

		long last_ranger_utime = 0;
		Range[] slices;
		Identifier ranges;

		RangerIL(Identifier ranges, RangerState rangerState) {
			this.ranges = ranges;

			if (rangerState != null) {
				createSlices(rangerState);
				updateSlices(rangerState);
			}
		}

		void createSlices(RangerState rangerState) {
			assert rangerState != null;
			assert slices == null;

			slices = new Range[rangerState.ranger.length];
			for (int index = 0; index < rangerState.ranger.length; ++index) {
				slices[index] = new Range(index
						- (rangerState.ranger.length / 2));
			}
		}

		void update(RangerState rangerState) {
			if (rangerState == null) {
				return;
			}

			if (rangerState.utime == last_ranger_utime) {
				return;
			}
			last_ranger_utime = rangerState.utime;

			if (slices == null) {
				createSlices(rangerState);
			} else {
				assert rangerState.ranger.length == slices.length;
			}

			updateSlices(rangerState);
		}

		void updateSlices(RangerState rangerState) {
			for (int index = 0; index < rangerState.ranger.length; ++index) {
				slices[index].update(rangerState.ranger[index]);
			}
		}
	}

	private class SelfIL {
		IntElement self_motor_left_position;
		IntElement self_motor_right_position;
		FloatElement self_pose_x;
		FloatElement self_pose_y;
		FloatElement self_pose_z;
		FloatElement self_pose_yaw;
		Identifier self_waypoints;

		long last_splinter_uime = 0;

		SelfIL(Identifier self, splinterstate_t splinterState) {
			Identifier self_motor = agent.CreateIdWME(self, "motor");
			Identifier self_motor_left = agent.CreateIdWME(self_motor, "left");
			self_motor_left_position = agent.CreateIntWME(self_motor_left,
					"position", 0);
			Identifier self_motor_right = agent
					.CreateIdWME(self_motor, "right");
			self_motor_right_position = agent.CreateIntWME(self_motor_right,
					"position", 0);

			agent.CreateStringWME(self, "name", agent.GetAgentName());

			Identifier self_pose = agent.CreateIdWME(self, "pose");
			self_pose_x = agent.CreateFloatWME(self_pose, "x", 0);
			self_pose_y = agent.CreateFloatWME(self_pose, "y", 0);
			self_pose_yaw = agent.CreateFloatWME(self_pose, "yaw", 0);

			self_waypoints = agent.CreateIdWME(self, "waypoints");
			waypoints.setRootIdentifier(self_waypoints);

			update(splinterState);
		}

		void update(splinterstate_t splinterState) {
			// update robot state if we have new state
			if (splinterState.utime == last_splinter_uime) {
				return;
			}

			last_splinter_uime = splinterState.utime;

			agent.Update(self_motor_left_position, splinterState.leftodom);
			agent.Update(self_motor_right_position, splinterState.rightodom);

			agent.Update(self_pose_x, splinterState.pose.pos[0]);
			agent.Update(self_pose_y, splinterState.pose.pos[1]);
			agent.Update(self_pose_yaw, Math.toDegrees(MathUtil.mod2pi(Geometry
					.quatToRollPitchYaw(splinterState.pose.orientation)[2])));

			waypoints.setNewRobotPose(splinterState.pose);
		}
	}

	Agent agent;
	Waypoints waypoints;

	TimeIL timeIL;
	RangerIL rangerIL;
	SelfIL selfIL;

	Logger logger;

	public InputLinkManager(Agent agent, Waypoints waypoints,
			splinterstate_t splinterState, RangerState rangerState) {
		this.logger = LogFactory.createSimpleLogger("InputLinkManager",
				Level.INFO);

		this.agent = agent;
		this.agent.SetBlinkIfNoChange(false);

		this.waypoints = waypoints;

		initialize(splinterState, rangerState);
	}

	private void initialize(splinterstate_t splinterState,
			RangerState rangerState) {
		logger.fine("initializing input link");

		Identifier inputLink = agent.GetInputLink();

		// Please see default-robot.vsa for input link definition and comments!

		Identifier time = agent.CreateIdWME(inputLink, "time");
		timeIL = new TimeIL(time);

		Identifier ranges = agent.CreateIdWME(inputLink, "ranges");
		rangerIL = new RangerIL(ranges, rangerState);

		Identifier self = agent.CreateIdWME(inputLink, "self");
		selfIL = new SelfIL(self, splinterState);

		agent.Commit();
	}

	public void update(splinterstate_t splinterState, RangerState rangerState) {
		timeIL.update();
		rangerIL.update(rangerState);
		selfIL.update(splinterState);
	}
}
