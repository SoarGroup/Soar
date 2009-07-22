/**
 * 
 */
package edu.umich.soar.sps.control;

import jmat.LinAlg;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Set target heading to rotate to.
 */
final class SetHeadingCommand extends DDCCommand implements Command {
	private static final Logger logger = Logger.getLogger(SetHeadingCommand.class);
	private static final String YAW = "yaw";
	private static final double TOLERANCE = Math.toRadians(3);
	static final String NAME = "set-heading";

	private CommandStatus status;
	private double yaw;
	
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newHeadingCommand(yaw);
	}

	public boolean execute(InputLinkInterface inputLink, Agent agent,
			Identifier command, SplinterState splinter,
			OutputLinkManager outputLinkManager) {
		if (this.agent != null || this.command != null || this.status == null) {
			//throw new IllegalStateException();
		}
		
		try {
			yaw = Double.parseDouble(command.GetParameterValue(YAW));
		} catch (NullPointerException ex) {
			logger.warn(NAME + ": No " + YAW + " on command");
			CommandStatus.error.addStatus(agent, command);
			return false;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse " + YAW + ": " + command.GetParameterValue(YAW));
			CommandStatus.error.addStatus(agent, command);
			return false;
		}
		yaw = Math.toRadians(yaw);

		logger.debug(String.format(NAME + ": %10.3f", yaw));
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

	public boolean update(SplinterState splinter) {
		if (agent == null || command == null || status == null) {
			throw new IllegalStateException();
		}
		
		double splinterYaw = LinAlg.quatToRollPitchYaw(splinter.getSplinterPose().orientation)[2];
		splinterYaw = Math.abs(splinterYaw);
		
		if (Double.compare(splinterYaw, TOLERANCE) < 0) {
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
