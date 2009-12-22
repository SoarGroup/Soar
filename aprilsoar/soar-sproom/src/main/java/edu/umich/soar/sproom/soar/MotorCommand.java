/**
 * 
 */
package edu.umich.soar.sproom.soar;

import lcmtypes.pose_t;

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
	private CommandStatus status = CommandStatus.accepted;

	MotorCommand(Identifier wme) {
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
		double left;
		try {
			left = Double.parseDouble(wme.GetParameterValue(LEFT));
		} catch (NullPointerException ex) {
			return new InvalidCommand(wme, "No " + LEFT + " on command");
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Unable to parse " + LEFT + ": " + wme.GetParameterValue(LEFT));
		}

		double right;
		try {
			right = Double.parseDouble(wme.GetParameterValue(RIGHT));
		} catch (NullPointerException ex) {
			return new InvalidCommand(wme, "No " + RIGHT + " on command");
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Unable to parse " + LEFT + ": " + wme.GetParameterValue(LEFT));
		}

		left = Command.clamp(left, -1.0, 1.0);
		left = Command.clamp(right, -1.0, 1.0);
		ddc = DifferentialDriveCommand.newMotorCommand(left, right);
		logger.debug(ddc);
		CommandStatus.accepted.addStatus(wme);
		return this;
	}

	@Override
	public void update(pose_t pose, Adaptable app) {
		if (!status.isTerminated()) {
			if (status != CommandStatus.executing) {
				status = CommandStatus.executing;
				status.addStatus(wme);
			}			
		}
	}
	
	@Override
	public void interrupt() {
		if (!status.isTerminated()) {
			status = CommandStatus.complete;
			status.addStatus(wme);
		}
	}
}
