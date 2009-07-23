/**
 * 
 */
package edu.umich.soar.sps.control;

import jmat.LinAlg;
import lcmtypes.pose_t;

import org.apache.log4j.Logger;

import edu.umich.soar.robot.OffsetPose;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Gracefully stop movement.
 * 
 * Returns accepted. Not interruptible. Creates DDC.
 */
final class StopCommand extends DDCCommand implements Command {
	private static final Logger logger = Logger.getLogger(StopCommand.class);
	private static final double TOLERANCE = 0.01; // meters per second
	static final String NAME = "stop";

	private CommandStatus status;
	
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newVelocityCommand(0, 0);
	}

	public boolean execute(InputLinkInterface inputLink, Agent agent,
			Identifier command, OffsetPose opose,
			OutputLinkManager outputLinkManager) {
		if (this.agent != null || this.command != null || this.status != null) {
			throw new IllegalStateException();
		}

		logger.debug(NAME + ":");
		CommandStatus.accepted.addStatus(agent, command);
		status = CommandStatus.accepted;

		this.agent = agent;
		this.command = command;
		return true;
	}

	public void interrupt() {
		if (agent == null || command == null || status == null) {
			throw new IllegalStateException();
		}
		
		CommandStatus.interrupted.addStatus(agent, command);
		agent = null;
		command = null;
		status = null;
	}

	public boolean update(OffsetPose opose) {
		if (agent != null || command != null || status != null) {
			throw new IllegalStateException();
		}
		
		pose_t pose = opose.getPose();
		
		if (Double.compare(LinAlg.magnitude(pose.vel), TOLERANCE) < 0) {
			CommandStatus.complete.addStatus(agent, command);
			agent = null;
			command = null;
			status = null;
			return true; // complete
		} else if (status == CommandStatus.accepted) {
			CommandStatus.executing.addStatus(agent, command);
			status = CommandStatus.executing;
		}
		return false; // executing
	}
}
