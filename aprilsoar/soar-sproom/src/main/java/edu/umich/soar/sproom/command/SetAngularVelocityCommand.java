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
public class SetAngularVelocityCommand extends OutputLinkCommand implements DriveCommand {
	private static final Log logger = LogFactory.getLog(SetAngularVelocityCommand.class);
	private static final String ANGVEL = "angular-velocity";
	static final String NAME = "set-angular-velocity";

	private final Identifier wme;
	private DifferentialDriveCommand ddc;
	private CommandStatus status;

	SetAngularVelocityCommand(Identifier wme) {
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
		double angularVelocity;
		try {
			angularVelocity = Double.parseDouble(wme.GetParameterValue(ANGVEL));
			angularVelocity = CommandConfig.CONFIG.angleFromView(angularVelocity);
		} catch (NullPointerException ex) {
			return new InvalidCommand(wme, "No " + ANGVEL + " on command");
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Unable to parse " + ANGVEL + ": " + wme.GetParameterValue(ANGVEL));
		}

		ddc = DifferentialDriveCommand.newAngularVelocityCommand(angularVelocity);
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
