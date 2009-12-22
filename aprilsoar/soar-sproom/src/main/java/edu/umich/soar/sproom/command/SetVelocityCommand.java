/**
 * 
 */
package edu.umich.soar.sproom.command;

import lcmtypes.pose_t;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

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
public class SetVelocityCommand extends OutputLinkCommand implements DriveCommand {
	private static final Log logger = LogFactory.getLog(SetVelocityCommand.class);
	private static final String LINVEL = "linear-velocity";
	private static final String ANGVEL = "angular-velocity";
	static final String NAME = "set-velocity";

	private final Identifier wme;
	private DifferentialDriveCommand ddc;
	private CommandStatus status;

	SetVelocityCommand(Identifier wme) {
		super(Integer.valueOf(wme.GetTimeTag()));
		this.wme = wme;
	}

	@Override
	public String getName() {
		return NAME;
	}
	
	@Override
	public DifferentialDriveCommand getDDC() {
		return ddc;
	}

	@Override
	public OutputLinkCommand accept() {
		Double linearVelocity = null;
		Double angularVelocity = null;
		{
			String linvelString = wme.GetParameterValue(LINVEL);
			String angvelString = wme.GetParameterValue(ANGVEL);
			
			if (linvelString == null && angvelString == null) {
				return new InvalidCommand(wme, "Must have at least one of " + LINVEL + " or " + ANGVEL + " on the command.");
			}
	
			if (linvelString != null) {
				try {
					linearVelocity = Double.parseDouble(linvelString);
					linearVelocity = CommandConfig.CONFIG.speedFromView(linearVelocity);
				} catch (NumberFormatException e) {
					return new InvalidCommand(wme, "Unable to parse " + LINVEL + ": " + linvelString);
				}
			}
	
			if (angvelString != null) {
				try {
					angularVelocity = Double.parseDouble(angvelString);
					angularVelocity = CommandConfig.CONFIG.angleFromView(angularVelocity);
				} catch (NumberFormatException e) {
					return new InvalidCommand(wme, "Unable to parse " + ANGVEL + ": " + angvelString);
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
		CommandStatus.accepted.addStatus(wme);
		return this;
	}
	
	@Override
	public boolean update(pose_t pose) {
		if (status != CommandStatus.complete) {
			if (status != CommandStatus.executing) {
				status = CommandStatus.executing;
				status.addStatus(wme);
			}			
			return false; // not done
		}
		return true;
	}
	
	@Override
	public void interrupt() {
		if (status != CommandStatus.complete) {
			status = CommandStatus.complete;
			status.addStatus(wme);
		}
	}
}
