/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.command.CommandConfig;
import edu.umich.soar.sproom.drive.DifferentialDriveCommand;
import edu.umich.soar.sproom.drive.DriveCommand;

import sml.Identifier;

/**
 * @author voigtjr
 *
 * Set linear and angular velocities.
 * 
 * Returns executing. Not interruptible. Creates DDC.
 */
public class SetLinearVelocityCommand extends OutputLinkCommand implements DriveCommand {
	private static final Log logger = LogFactory.getLog(SetLinearVelocityCommand.class);
	private static final String LINVEL = "linear-velocity";
	static final String NAME = "set-linear-velocity";

	private final Identifier wme;
	private DifferentialDriveCommand ddc;
	
	public SetLinearVelocityCommand(Identifier wme) {
		super(wme);
		this.wme = wme;
	}

	@Override
	public DifferentialDriveCommand getDDC() {
		return ddc;
	}

	@Override
	protected boolean accept() {
		double linearVelocity;
		try {
			linearVelocity = Double.parseDouble(wme.GetParameterValue(LINVEL));
			linearVelocity = CommandConfig.CONFIG.speedFromView(linearVelocity);
		} catch (NullPointerException ex) {
			addStatusError("No " + LINVEL + " on command");
			return false;
		} catch (NumberFormatException e) {
			addStatusError("Unable to parse " + LINVEL + ": " + wme.GetParameterValue(LINVEL));
			return false;
		}

		ddc = DifferentialDriveCommand.newLinearVelocityCommand(linearVelocity);
		logger.debug(ddc);
		addStatus(CommandStatus.ACCEPTED);
		return true;
	}
	
	@Override
	public void update(Adaptable app) {
		addStatus(CommandStatus.EXECUTING);
	}
	
	@Override
	public void interrupt() {
		addStatus(CommandStatus.COMPLETE);
	}
}
