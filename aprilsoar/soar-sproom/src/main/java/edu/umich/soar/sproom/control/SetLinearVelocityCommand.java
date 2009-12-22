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
public class SetLinearVelocityCommand extends DDCCommand implements Command {
	private static final Log logger = LogFactory.getLog(SetLinearVelocityCommand.class);
	private static final String LINVEL = "linear-velocity";
	static final String NAME = "set-linear-velocity";

	static Command newInstance() {
		return new SetLinearVelocityCommand();
	}
	
	private double linearVelocity;

	private SetLinearVelocityCommand() {
	}

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
