/**
 * 
 */
package edu.umich.soar.sps.control;

import org.apache.log4j.Logger;

import edu.umich.soar.waypoints.OffsetPose;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Set linear and angular velocities.
 * 
 * Returns executing. Not interruptable. Creates DDC.
 */
class SetVelocityCommand extends DDCCommand implements Command {
	private static final Logger logger = Logger.getLogger(SetVelocityCommand.class);
	private static final String LINVEL = "linear-velocity";
	private static final String ANGVEL = "angular-velocity";
	static final String NAME = "set-velocity";

	private double linearVelocity;
	private double angularVelocity;
	
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newVelocityCommand(angularVelocity, linearVelocity);
	}

	public boolean execute(InputLinkInterface inputLink, Agent agent,
			Identifier command, OffsetPose opose,
			OutputLinkManager outputLinkManager) {
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
