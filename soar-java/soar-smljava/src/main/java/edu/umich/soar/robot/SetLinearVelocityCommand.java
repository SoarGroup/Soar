/**
 * 
 */
package edu.umich.soar.robot;

import org.apache.log4j.Logger;

import sml.Identifier;

/**
 * @author voigtjr
 *
 * Set linear and angular velocities.
 * 
 * Returns executing. Not interruptable. Creates DDC.
 */
final public class SetLinearVelocityCommand extends DDCCommand implements Command {
	private static final Logger logger = Logger.getLogger(SetLinearVelocityCommand.class);
	private static final String LINVEL = "linear-velocity";
	static final String NAME = "set-linear-velocity";

	static Command newInstance() {
		return new SetLinearVelocityCommand();
	}
	
	private double linearVelocity;
	
	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newLinearVelocityCommand(linearVelocity);
	}

	@Override
	public boolean execute(Identifier command) {
		if (this.command != null) {
			throw new IllegalStateException();
		}
		
		try {
			linearVelocity = Double.parseDouble(command.GetParameterValue(LINVEL));
		} catch (NullPointerException ex) {
			CommandStatus.error.addStatus(command, NAME + ": No " + LINVEL + " on command");
			return false;
		} catch (NumberFormatException e) {
			CommandStatus.error.addStatus(command, NAME + ": Unable to parse " + LINVEL + ": " + command.GetParameterValue(LINVEL));
			return false;
		}

		logger.debug(String.format(NAME + ": l%1.3f", linearVelocity));
		CommandStatus.accepted.addStatus(command);
		CommandStatus.executing.addStatus(command);
		
		this.command = command;
		return true;
	}
}
