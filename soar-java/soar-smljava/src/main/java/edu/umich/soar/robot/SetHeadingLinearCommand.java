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
 * Set target heading and speed. Note, does not "complete" like set-heading (without
 * linear velocity) does.
 */
final public class SetHeadingLinearCommand extends DDCCommand implements Command {
	private static final Logger logger = Logger.getLogger(SetHeadingLinearCommand.class);
	private static final String YAW = "yaw";
	private static final String LINVEL = "linear-velocity";
	private static final double TOLERANCE = Math.toRadians(3);
	static final String NAME = "set-heading-linear";

	static Command newInstance(OffsetPose opose) {
		return new SetHeadingLinearCommand(opose);
	}
	
	private final OffsetPose opose;
	private CommandStatus status;
	private double yaw;
	private double linearVelocity;
	
	public SetHeadingLinearCommand(OffsetPose opose) {
		this.opose = opose;
	}

	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newHeadingLinearVelocityCommand(yaw, linearVelocity);
	}

	@Override
	public boolean execute(Agent agent, Identifier command) {
		if (this.agent != null || this.command != null) {
			throw new IllegalStateException();
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

		try {
			linearVelocity = Double.parseDouble(command.GetParameterValue(LINVEL));
		} catch (NullPointerException ex) {
			logger.warn(NAME + ": No " + LINVEL + " on command");
			CommandStatus.error.addStatus(agent, command);
			return false;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse " + LINVEL + ": " + command.GetParameterValue(LINVEL));
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		logger.debug(String.format(NAME + ": y%3.1f l%1.3f", yaw, linearVelocity));
		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.executing.addStatus(agent, command);
		
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
