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
 * Set target heading to rotate to.
 */
public class SetHeadingCommand extends DDCCommand implements Command {
	private static final Log logger = LogFactory.getLog(SetHeadingCommand.class);
	private static final String YAW = "yaw";
	private static final double TOLERANCE = Math.toRadians(3);
	static final String NAME = "set-heading";

	static Command newInstance(OffsetPose opose) {
		return new SetHeadingCommand(opose);
	}
	
	private final OffsetPose opose;
	private CommandStatus status;
	private double yaw;
	
	private SetHeadingCommand(OffsetPose opose) {
		this.opose = opose;
	}

	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newHeadingCommand(yaw);
	}

	@Override
	public boolean execute(Identifier command) {
		if (this.command != null || this.status == null) {
			//throw new IllegalStateException();
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

		logger.debug(String.format(NAME + ": %10.3f", yaw));
		CommandStatus.accepted.addStatus(command);
		status = CommandStatus.accepted;
		
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
