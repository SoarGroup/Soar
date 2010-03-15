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
 * Set linear and angular velocities.
 * 
 * @author voigtjr@gmail.com
 */
public class SetVelocityCommand extends OutputLinkCommand implements DriveCommand {
	private static final Log logger = LogFactory.getLog(SetVelocityCommand.class);
	private static final String LINVEL = "linear-velocity";
	private static final String ANGVEL = "angular-velocity";
	static final String NAME = "set-velocity";

	private final Identifier wme;
	private DifferentialDriveCommand ddc;

	public SetVelocityCommand(Identifier wme) {
		super(wme);
		this.wme = wme;
	}

	@Override
	public DifferentialDriveCommand getDDC() {
		return ddc;
	}

	@Override
	protected boolean accept() {
		Double linearVelocity = null;
		Double angularVelocity = null;
		{
			String linvelString = wme.GetParameterValue(LINVEL);
			String angvelString = wme.GetParameterValue(ANGVEL);
			
			if (linvelString == null && angvelString == null) {
				addStatusError("Must have at least one of " + LINVEL + " or " + ANGVEL + " on the command.");
				return false;
			}
	
			if (linvelString != null) {
				try {
					linearVelocity = Double.parseDouble(linvelString);
					linearVelocity = CommandConfig.CONFIG.speedFromView(linearVelocity);
				} catch (NumberFormatException e) {
					addStatusError("Unable to parse " + LINVEL + ": " + linvelString);
					return false;
				}
			}
	
			if (angvelString != null) {
				try {
					angularVelocity = Double.parseDouble(angvelString);
					angularVelocity = CommandConfig.CONFIG.angleFromView(angularVelocity);
				} catch (NumberFormatException e) {
					addStatusError("Unable to parse " + ANGVEL + ": " + angvelString);
					return false;
				}
			}
		}
		
		if (linearVelocity == null) {
			ddc = DifferentialDriveCommand.newAngularVelocityCommand(angularVelocity);
		} else if (angularVelocity == null) {
			ddc = DifferentialDriveCommand.newLinearVelocityCommand(linearVelocity);
		} else {
			ddc = DifferentialDriveCommand.newVelocityCommand(angularVelocity, linearVelocity);
		}
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
