/**
 * 
 */
package edu.umich.soar.sproom.control;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import jmat.LinAlg;

import sml.Identifier;

/**
 * @author voigtjr
 *
 * Set target heading and speed. Note, does not "complete" like set-heading (without
 * linear velocity) does.
 */
public class SetHeadingLinearCommand extends DDCCommand implements Command {
	private static final Log logger = LogFactory.getLog(SetHeadingLinearCommand.class);
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
	
	private SetHeadingLinearCommand(OffsetPose opose) {
		this.opose = opose;
	}

	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newHeadingLinearVelocityCommand(yaw, linearVelocity);
	}

	@Override
	public boolean execute(Identifier command) {
		if (this.command != null) {
			throw new IllegalStateException();
		}
		
		try {
			yaw = Double.parseDouble(command.GetParameterValue(YAW));
		} catch (NullPointerException ex) {
			CommandStatus.error.addStatus(command, NAME + ": No " + YAW + " on command");
			return false;
		} catch (NumberFormatException e) {
			CommandStatus.error.addStatus(command, NAME + ": Unable to parse " + YAW + ": " + command.GetParameterValue(YAW));
			return false;
		}
		yaw = Math.toRadians(yaw);

		try {
			linearVelocity = Double.parseDouble(command.GetParameterValue(LINVEL));
		} catch (NullPointerException ex) {
			CommandStatus.error.addStatus(command, NAME + ": No " + LINVEL + " on command");
			return false;
		} catch (NumberFormatException e) {
			CommandStatus.error.addStatus(command, NAME + ": Unable to parse " + LINVEL + ": " + command.GetParameterValue(LINVEL));
			return false;
		}

		logger.debug(String.format(NAME + ": y%3.1f l%1.3f", yaw, linearVelocity));
		CommandStatus.accepted.addStatus(command);
		CommandStatus.executing.addStatus(command);
		
		this.command = command;
		return true;
	}

	@Override
	public void interrupt() {
		if (command == null || status == null) {
			throw new IllegalStateException();
		}
		
		CommandStatus.interrupted.addStatus(command);
		command = null;
		status = null;
	}

	@Override
	public boolean update() {
		if (command == null || status == null) {
			throw new IllegalStateException();
		}
		
		double currentYaw = LinAlg.quatToRollPitchYaw(opose.getPose().orientation)[2];
		double difference = yaw - currentYaw;
		difference = Math.abs(difference);
		
		if (Double.compare(difference, TOLERANCE) < 0) {
			CommandStatus.complete.addStatus(command);
			command = null;
			status = null;
			return true; // complete
		} else if (status == CommandStatus.accepted) {
			CommandStatus.executing.addStatus(command);
			status = CommandStatus.executing;
		}
		return false; // executing
	}
}
