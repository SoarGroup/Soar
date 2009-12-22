/**
 * 
 */
package edu.umich.soar.sproom.control;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import sml.Identifier;

/**
 * @author voigtjr
 *
 * Command motors directly with throttles. 
 */
public class MotorCommand extends DDCCommand implements Command {
	private static final Log logger = LogFactory.getLog(MotorCommand.class);
	static final String NAME = "motor";

	static Command newInstance() {
		return new MotorCommand();
	}
	
	private double left;
	private double right;
	
	private MotorCommand() {
	}
	
	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newMotorCommand(left, right);
	}

	@Override
	public boolean execute(Identifier command) {
		if (this.command != null) {
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

		this.command = command;
		return true;
	}
}
