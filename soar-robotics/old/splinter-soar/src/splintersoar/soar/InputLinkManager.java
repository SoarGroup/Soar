package splintersoar.soar;

import java.util.logging.Logger;

import jmat.LinAlg;
import jmat.MathUtil;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import splintersoar.Configuration;
import splintersoar.LogFactory;
import lcmtypes.splinterstate_t;
import splintersoar.ranger.RangerState;

/**
 * @author voigtjr
 * Soar input link management. Also handles some updating of waypoint state.
 */
public class InputLinkManager {

	private class TimeIL {
		IntElement secondswme;
		IntElement microsecondswme;

		final static long nanosecondsPerSecond = 1000000000;

		TimeIL(Identifier time) {
			secondswme = agent.CreateIntWME(time, "seconds", 0);
			microsecondswme = agent.CreateIntWME(time, "microseconds", 0);

			update();
		}

		void update() {
			long current = System.nanoTime();
			int seconds = (int) (current / nanosecondsPerSecond);
			int microseconds = (int) (current % nanosecondsPerSecond);
			microseconds /= 1000;

			agent.Update(secondswme, seconds);
			agent.Update(microsecondswme, microseconds);
		}
	}

	private class RangerIL {

		class Range {
			int idNumber;

			Identifier rangewme;
			FloatElement startwme;
			FloatElement endwme;
			FloatElement distancewme;

			Range(int id) {
				idNumber = id;

				rangewme = agent.CreateIdWME(rangeswme, "range");
				agent.CreateIntWME(rangewme, "id", idNumber);

				startwme = agent.CreateFloatWME(rangewme, "start", 0);
				endwme = agent.CreateFloatWME(rangewme, "end", 0);
				distancewme = agent.CreateFloatWME(rangewme, "distance", 0);
			}

			void update(RangerState.RangerData data) {
				agent.Update(startwme, Math.toDegrees(data.start));
				agent.Update(endwme, Math.toDegrees(data.end));
				agent.Update(distancewme, data.distance);
			}
		}

		long lastRangerutime = 0;
		Range[] slices;
		Identifier rangeswme;

		RangerIL(Identifier ranges, RangerState rangerState) {
			this.rangeswme = ranges;

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
				slices[index] = new Range(index - (rangerState.ranger.length / 2));
			}
		}

		void update(RangerState rangerState) {
			if (rangerState == null) {
				return;
			}

			if (rangerState.utime == lastRangerutime) {
				return;
			}
			lastRangerutime = rangerState.utime;

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
		IntElement leftPositionwme;
		IntElement rightPositionwme;
		FloatElement xwme;
		FloatElement ywme;
		FloatElement zwme;
		FloatElement yawwme;
		Identifier waypointswme;

		long lastSplinterutime = 0;

		SelfIL(Identifier self, splinterstate_t splinterState) {
			Identifier motorwme = agent.CreateIdWME(self, "motor");
			Identifier leftMotorwme = agent.CreateIdWME(motorwme, "left");
			leftPositionwme = agent.CreateIntWME(leftMotorwme, "position", 0);
			Identifier rightMotorwme = agent.CreateIdWME(motorwme, "right");
			rightPositionwme = agent.CreateIntWME(rightMotorwme, "position", 0);

			agent.CreateStringWME(self, "name", agent.GetAgentName());

			Identifier posewme = agent.CreateIdWME(self, "pose");
			xwme = agent.CreateFloatWME(posewme, "x", 0);
			ywme = agent.CreateFloatWME(posewme, "y", 0);
			yawwme = agent.CreateFloatWME(posewme, "yaw", 0);

			waypointswme = agent.CreateIdWME(self, "waypoints");
			waypoints.setRootIdentifier(waypointswme);

			update(splinterState);
		}

		void update(splinterstate_t splinterState) {
			// update robot state if we have new state
			if (splinterState.utime == lastSplinterutime) {
				return;
			}

			lastSplinterutime = splinterState.utime;

			agent.Update(leftPositionwme, splinterState.leftodom);
			agent.Update(rightPositionwme, splinterState.rightodom);

			agent.Update(xwme, splinterState.pose.pos[0]);
			agent.Update(ywme, splinterState.pose.pos[1]);
			agent.Update(yawwme, Math.toDegrees(MathUtil.mod2pi(LinAlg.quatToRollPitchYaw(splinterState.pose.orientation)[2])));

			waypoints.setNewRobotPose(splinterState.pose);
		}
	}

	Agent agent;
	Waypoints waypoints;

	TimeIL timeIL;
	RangerIL rangerIL;
	SelfIL selfIL;

	Logger logger;

	public InputLinkManager(Agent agent, Waypoints waypoints, splinterstate_t splinterState, RangerState rangerState, Configuration cnf) {
		this.logger = LogFactory.createSimpleLogger("InputLinkManager", cnf.loglevel);

		this.agent = agent;
		this.agent.SetBlinkIfNoChange(false);

		this.waypoints = waypoints;

		initialize(splinterState, rangerState);
	}

	private void initialize(splinterstate_t splinterState, RangerState rangerState) {
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
