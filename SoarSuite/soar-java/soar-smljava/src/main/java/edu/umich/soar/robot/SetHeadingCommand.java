/**
 * 
 */
package edu.umich.soar.robot;

import jmat.LinAlg;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Set target heading to rotate to.
 */
final public class SetHeadingCommand extends DDCCommand implements Command {
	private static final Logger logger = Logger.getLogger(SetHeadingCommand.class);
	private static final String YAW = "yaw";
	private static final double TOLERANCE = Math.toRadians(3);
	public static final String NAME = "set-heading";

	public static Command newInstance(OffsetPose opose) {
		return new SetHeadingCommand(opose);
	}
	
	public SetHeadingCommand(OffsetPose opose) {
		this.opose = opose;
	}

	private final OffsetPose opose;
	private CommandStatus status;
	private double yaw;
	
	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newHeadingCommand(yaw);
	}

	@Override
	public boolean execute(Agent agent, Identifier command) {
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

	@Override
	public void interrupt() {
		if (agent == null || command == null || status == null) {
			throw new IllegalStateException();
		}
		
		CommandStatus.interrupted.addStatus(agent, command);
		agent = null;
		command = null;
		status = null;
	}

	@Override
	public boolean update() {
		if (agent == null || command == null || status == null) {
			throw new IllegalStateException();
		}
		
		double splinterYaw = LinAlg.quatToRollPitchYaw(opose.getPose().orientation)[2];
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
