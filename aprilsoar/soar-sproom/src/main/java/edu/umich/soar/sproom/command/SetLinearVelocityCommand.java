/**
 * 
 */
package edu.umich.soar.sproom.command;

import lcmtypes.pose_t;

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
public class SetLinearVelocityCommand extends OutputLinkCommand implements DriveCommand {
	private static final Log logger = LogFactory.getLog(SetLinearVelocityCommand.class);
	private static final String LINVEL = "linear-velocity";
	static final String NAME = "set-linear-velocity";

	private final Identifier wme;
	private DifferentialDriveCommand ddc;
	private CommandStatus status;
	
	SetLinearVelocityCommand(Identifier wme) {
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
		double linearVelocity;
		try {
			linearVelocity = Double.parseDouble(wme.GetParameterValue(LINVEL));
			linearVelocity = CommandConfig.CONFIG.speedFromView(linearVelocity);
		} catch (NullPointerException ex) {
			return new InvalidCommand(wme, "No " + LINVEL + " on command");
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Unable to parse " + LINVEL + ": " + wme.GetParameterValue(LINVEL));
		}

		ddc = DifferentialDriveCommand.newLinearVelocityCommand(linearVelocity);
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
