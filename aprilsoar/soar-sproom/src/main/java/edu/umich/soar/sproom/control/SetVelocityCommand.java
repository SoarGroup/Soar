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
 * Set linear and angular velocities.
 * 
 * Returns executing. Not interruptible. Creates DDC.
 */
final public class SetVelocityCommand extends DDCCommand implements Command {
	private static final Log logger = LogFactory.getLog(SetVelocityCommand.class);
	private static final String LINVEL = "linear-velocity";
	private static final String ANGVEL = "angular-velocity";
	static final String NAME = "set-velocity";

	static Command newInstance() {
		return new SetVelocityCommand();
	}
	
	private double linearVelocity;
	private double angularVelocity;
	
	private SetVelocityCommand() {
	}
	
	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newVelocityCommand(angularVelocity, linearVelocity);
	}

	@Override
	public boolean execute(Identifier command) {
		if (this.command != null) {
			throw new IllegalStateException();
		}

		String linvelString = command.GetParameterValue(LINVEL);
		String angvelString = command.GetParameterValue(ANGVEL);
		
		if (linvelString == null && angvelString == null) {
			CommandStatus.error.addStatus(command, NAME + ": Must have at least one of " + LINVEL + " or " + ANGVEL + " on the command.");
			return false;
		}

		if (linvelString != null) {
			try {
				linearVelocity = Double.parseDouble(linvelString);
			} catch (NumberFormatException e) {
				CommandStatus.error.addStatus(command, NAME + ": Unable to parse " + LINVEL + ": " + command.GetParameterValue(LINVEL));
				return false;
			}
		}
		
		if (angvelString != null) {
			try {
				angularVelocity = Math.toRadians(Double.parseDouble(angvelString));
			} catch (NumberFormatException e) {
				CommandStatus.error.addStatus(command, NAME + ": Unable to parse " + ANGVEL + ": " + command.GetParameterValue(ANGVEL));
				return false;
			}
		}
		
		logger.debug(String.format(NAME + ": l%1.3f a%3.1f", linearVelocity, angularVelocity));
		CommandStatus.accepted.addStatus(command);
		CommandStatus.executing.addStatus(command);
		
		this.command = command;
		return true;
	}
}
