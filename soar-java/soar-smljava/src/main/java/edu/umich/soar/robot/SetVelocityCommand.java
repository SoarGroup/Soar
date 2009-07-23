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
 * Set linear and angular velocities.
 * 
 * Returns executing. Not interruptable. Creates DDC.
 */
final public class SetVelocityCommand extends DDCCommand implements Command {
	private static final Logger logger = Logger.getLogger(SetVelocityCommand.class);
	private static final String LINVEL = "linear-velocity";
	private static final String ANGVEL = "angular-velocity";
	public static final String NAME = "set-velocity";

	public static Command newInstance() {
		return new SetVelocityCommand();
	}
	
	private double linearVelocity;
	private double angularVelocity;
	
	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newVelocityCommand(angularVelocity, linearVelocity);
	}

	@Override
	public boolean execute(Agent agent, Identifier command) {
		if (this.agent != null || this.command != null) {
			throw new IllegalStateException();
		}
		
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

		try {
			angularVelocity = Math.toRadians(Double.parseDouble(command.GetParameterValue(ANGVEL)));
		} catch (NullPointerException ex) {
			logger.warn(NAME + ": No " + ANGVEL + " on command");
			CommandStatus.error.addStatus(agent, command);
			return false;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Unable to parse " + ANGVEL + ": " + command.GetParameterValue(ANGVEL));
			CommandStatus.error.addStatus(agent, command);
			return false;
		}

		logger.debug(String.format(NAME + ": l%1.3f a%3.1f", linearVelocity, angularVelocity));
		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.executing.addStatus(agent, command);
		
		this.agent = agent;
		this.command = command;
		return true;
	}
}
