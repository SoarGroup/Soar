package org.msoar.sps.control;

import java.io.DataInputStream;
import java.io.IOException;

import org.apache.log4j.Logger;
import org.msoar.sps.Names;

import jmat.LinAlg;
import jmat.MathUtil;
import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.pose_t;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;

/**
 * @author voigtjr
 * Soar input link management. Also handles some updating of waypoint state.
 */
class InputLinkManager implements LCMSubscriber {
	private static Logger logger = Logger.getLogger(InputLinkManager.class);

	private class TimeIL {
		private IntElement secondswme;
		private IntElement microsecondswme;

		private final static long nanosecondsPerSecond = 1000000000;

		private TimeIL(Identifier time) {
			secondswme = agent.CreateIntWME(time, "seconds", 0);
			microsecondswme = agent.CreateIntWME(time, "microseconds", 0);

			update();
		}

		private void update() {
			long current = System.nanoTime();
			int seconds = (int) (current / nanosecondsPerSecond);
			int microseconds = (int) (current % nanosecondsPerSecond);
			microseconds /= 1000;

			agent.Update(secondswme, seconds);
			agent.Update(microsecondswme, microseconds);
		}
	}

	private class SelfIL {
		private FloatElement xwme;
		private FloatElement ywme;
		private FloatElement zwme;
		private FloatElement yawwme;

		private long utimeLast = 0;

		private SelfIL(Identifier self) {
			agent.CreateStringWME(self, "name", agent.GetAgentName());

			Identifier posewme = agent.CreateIdWME(self, "pose");
			xwme = agent.CreateFloatWME(posewme, "x", 0);
			ywme = agent.CreateFloatWME(posewme, "y", 0);
			zwme = agent.CreateFloatWME(posewme, "z", 0);
			yawwme = agent.CreateFloatWME(posewme, "yaw", 0);
		}

		private void update() {
			if (pose == null) {
				return; // no info
			}
			
			if (utimeLast == pose.utime) {
				return; // same info
			}

			// TODO: possibly need synchronization here
			pose_t poseCopy = pose.copy();
			utimeLast = poseCopy.utime;
			
			agent.Update(xwme, poseCopy.pos[0]);
			agent.Update(ywme, poseCopy.pos[1]);
			agent.Update(zwme, poseCopy.pos[2]);
			yaw = toDisplayYaw(poseCopy.orientation);
			agent.Update(yawwme, yaw);
		}
		
		private double toDisplayYaw(double[] orientation) {
			double newYaw = LinAlg.quatToRollPitchYaw(orientation)[2];
			newYaw = MathUtil.mod2pi(newYaw);
			newYaw = Math.toDegrees(newYaw);
			return newYaw;
		}
	}

	private Agent agent;

	private TimeIL timeIL;
	private SelfIL selfIL;
	private pose_t pose;
	private double yaw = 0;

	InputLinkManager(Agent agent) {
		this.agent = agent;
		this.agent.SetBlinkIfNoChange(false);

		LCM lcm = LCM.getSingleton();
		lcm.subscribe(Names.POSE_CHANNEL, this);
		initialize();
	}

	private void initialize() {
		logger.debug("Initializing input-link");

		Identifier inputLink = agent.GetInputLink();

		// Please see default-robot.vsa for input link definition and comments!
		// TODO: make new default-robot.vsa
		
		Identifier time = agent.CreateIdWME(inputLink, "time");
		timeIL = new TimeIL(time);

		Identifier self = agent.CreateIdWME(inputLink, "self");
		selfIL = new SelfIL(self);

		agent.Commit();
	}

	void update() {
		timeIL.update();
		selfIL.update();
	}

	@Override
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(Names.POSE_CHANNEL)) {
			try {
				pose = new pose_t(ins);
			} catch (IOException e) {
				logger.error("Error decoding pose_t message: " + e.getMessage());
			}
		}
	}

	public double getYaw() {
		return yaw;
	}
}
