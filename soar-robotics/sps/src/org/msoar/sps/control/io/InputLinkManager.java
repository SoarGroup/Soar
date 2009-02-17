package org.msoar.sps.control.io;

import java.io.DataInputStream;
import java.io.IOException;

import org.apache.log4j.Logger;
import org.msoar.sps.Names;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;
import lcmtypes.pose_t;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 * Soar input link management. Also handles some updating of waypoint state.
 */
public class InputLinkManager implements LCMSubscriber {
	private static Logger logger = Logger.getLogger(InputLinkManager.class);

	private Agent agent;
	private TimeIL timeIL;
	private SelfIL selfIL;
	private RangerIL rangerIL;
	private pose_t pose;
	private laser_t laser;

	public InputLinkManager(Agent agent, int rangesCount) {
		this.agent = agent;
		this.agent.SetBlinkIfNoChange(false);

		LCM lcm = LCM.getSingleton();
		lcm.subscribe(Names.POSE_CHANNEL, this);
		lcm.subscribe(Names.LASER_CHANNEL, this);
		initialize(rangesCount);
	}

	private void initialize(int rangesCount) {
		logger.debug("Initializing input-link");

		Identifier inputLink = agent.GetInputLink();

		// Please see default-robot.vsa for input link definition and comments!
		// TODO: make new default-robot.vsa
		
		Identifier time = agent.CreateIdWME(inputLink, "time");
		timeIL = new TimeIL(agent, time);

		Identifier self = agent.CreateIdWME(inputLink, "self");
		selfIL = new SelfIL(agent, self);

		Identifier ranges = agent.CreateIdWME(inputLink, "ranges");
		rangerIL = new RangerIL(agent, ranges, rangesCount);

		agent.Commit();
	}

	public void update() {
		timeIL.update();
		selfIL.update(pose);
		rangerIL.update(laser);
	}

	@Override
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(Names.POSE_CHANNEL)) {
			try {
				pose = new pose_t(ins);
			} catch (IOException e) {
				logger.error("Error decoding pose_t message: " + e.getMessage());
			}
		} else if (channel.equals(Names.LASER_CHANNEL)) {
			try {
				laser = new laser_t(ins);
			} catch (IOException e) {
				logger.error("Error decoding laser_t message: " + e.getMessage());
			}
		}
	}

	public double getYawRadians() {
		return selfIL.getYawRadians();
	}

	public WaypointsIL getWaypointsIL() {
		return selfIL.getWaypointsIL();
	}

	public long getPoseUtime() {
		if (pose == null) {
			return 0;
		}
		
		return pose.utime;
	}
}
