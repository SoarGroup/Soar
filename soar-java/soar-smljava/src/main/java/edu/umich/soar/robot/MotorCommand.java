/**
 * 
 */
package edu.umich.soar.robot;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Command motors directly with throttles. 
 */
final public class MotorCommand extends DDCCommand implements Command {
	private static final Logger logger = Logger.getLogger(MotorCommand.class);
	static final String NAME = "motor";

	static Command newInstance() {
		return new MotorCommand();
	}
	
	private double left;
	private double right;
	
	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newMotorCommand(left, right);
	}

	@Override
	public boolean execute(Agent agent, Identifier command) {
		if (this.agent != null || this.command != null) {
			throw new IllegalStateException();
		}
		
		try {
			left = Double.parseDouble(command.GetParameterValue("left"));
		} catch (NullPointerException ex) {
			CommandStatus.error.addStatus(command, NAME + ":No left on command");
			return false;
		} catch (NumberFormatException e) {
			CommandStatus.error.addStatus(command, NAME + ": Unable to parse left: " + command.GetParameterValue("left"));
			return false;
		}

		try {
			right = Double.parseDouble(command.GetParameterValue("right"));
		} catch (NullPointerException ex) {
			CommandStatus.error.addStatus(command, NAME + ":No right on command");
			return false;
		} catch (NumberFormatException e) {
			CommandStatus.error.addStatus(command, NAME + ":Unable to parse right: " + command.GetParameterValue("right"));
			return false;
		}

		left = Math.max(left, -1.0);
		left = Math.min(left, 1.0);

		right = Math.max(right, -1.0);
		right = Math.min(right, 1.0);

		logger.debug(String.format(NAME + ": %10.3f %10.3f", left, right));
		CommandStatus.accepted.addStatus(command);
		CommandStatus.executing.addStatus(command);

		this.agent = agent;
		this.command = command;
		return true;
	}
}
