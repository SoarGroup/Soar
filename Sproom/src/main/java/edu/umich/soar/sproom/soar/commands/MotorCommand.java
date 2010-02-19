/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.command.Command;
import edu.umich.soar.sproom.drive.DifferentialDriveCommand;
import edu.umich.soar.sproom.drive.DriveCommand;

import sml.Identifier;

/**
 * @author voigtjr
 *
 * Command motors directly with throttles. 
 */
public class MotorCommand extends OutputLinkCommand implements DriveCommand {
	private static final Log logger = LogFactory.getLog(MotorCommand.class);
	private static final String LEFT = "left";
	private static final String RIGHT = "right";
	static final String NAME = "motor";

	private final Identifier wme;
	private DifferentialDriveCommand ddc;

	public MotorCommand(Identifier wme) {
		super(wme);
		this.wme = wme;
	}
	
	@Override
	public DifferentialDriveCommand getDDC() {
		return ddc;
	}

	@Override
	protected boolean accept() {
		double left;
		try {
			left = Double.parseDouble(wme.GetParameterValue(LEFT));
		} catch (NullPointerException ex) {
			addStatusError("No " + LEFT + " on command");
			return false;
		} catch (NumberFormatException e) {
			addStatusError("Unable to parse " + LEFT + ": " + wme.GetParameterValue(LEFT));
			return false;
		}

		double right;
		try {
			right = Double.parseDouble(wme.GetParameterValue(RIGHT));
		} catch (NullPointerException ex) {
			addStatusError("No " + RIGHT + " on command");
			return false;
		} catch (NumberFormatException e) {
			addStatusError("Unable to parse " + LEFT + ": " + wme.GetParameterValue(LEFT));
			return false;
		}

		left = Command.clamp(left, -1.0, 1.0);
		left = Command.clamp(right, -1.0, 1.0);
		ddc = DifferentialDriveCommand.newMotorCommand(left, right);
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
