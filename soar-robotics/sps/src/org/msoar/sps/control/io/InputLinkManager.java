package org.msoar.sps.control.io;

import java.util.List;

import org.apache.log4j.Logger;

import lcmtypes.laser_t;
import lcmtypes.pose_t;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 * Soar input link management. Also handles some updating of waypoint state.
 */
public class InputLinkManager {
	private static Logger logger = Logger.getLogger(InputLinkManager.class);

	private Agent agent;
	private TimeIL timeIL;
	private SelfIL selfIL;
	private RangerIL rangerIL;

	public InputLinkManager(Agent agent, int rangesCount) {
		this.agent = agent;
		this.agent.SetBlinkIfNoChange(false);

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

	public void update(pose_t pose, laser_t laser, List<String> tokens) {
		timeIL.update();
		selfIL.update(pose, tokens);
		rangerIL.update(laser);
	}

	public double getYawRadians() {
		return selfIL.getYawRadians();
	}

	public WaypointsIL getWaypointsIL() {
		return selfIL.getWaypointsIL();
	}

	public ReceivedMessagesIL getMessagesIL() {
		return selfIL.getMessagesIL();
	}
}
