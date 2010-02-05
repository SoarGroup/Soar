/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.command.CommandConfig;
import edu.umich.soar.sproom.command.Pose;
import edu.umich.soar.sproom.drive.DifferentialDriveCommand;
import edu.umich.soar.sproom.drive.DriveCommand;

import sml.Identifier;

/**
 * @author voigtjr
 *
 * Set target heading to rotate to.
 */
public class SetHeadingCommand extends OutputLinkCommand implements DriveCommand {
	private static final Log logger = LogFactory.getLog(SetHeadingCommand.class);
	private static final String YAW = "yaw";
	private static final double TOLERANCE = Math.toRadians(3);
	static final String NAME = "set-heading";

	private final Identifier wme;
	private DifferentialDriveCommand ddc;
	private CommandStatus status = CommandStatus.accepted;
	private double targetYaw;
	
	public SetHeadingCommand(Identifier wme) {
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
		try {
			targetYaw = Double.parseDouble(wme.GetParameterValue(YAW));
			targetYaw = CommandConfig.CONFIG.angleFromView(targetYaw);
		} catch (NullPointerException ex) {
			return new InvalidCommand(wme, "No " + YAW + " on command");
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Unable to parse " + YAW + ": " + wme.GetParameterValue(YAW));
		}

		ddc = DifferentialDriveCommand.newHeadingCommand(targetYaw);
		logger.debug(ddc);
		CommandStatus.accepted.addStatus(wme);
		return this;
	}

	@Override
	public void update(Adaptable app) {
		if (!status.isTerminated()) {
			Pose pose = (Pose)app.getAdapter(Pose.class);
			double currentYaw = pose.getYaw();
			double difference = targetYaw - currentYaw;
			difference = Math.abs(difference);
			
			if (Double.compare(difference, TOLERANCE) < 0) {
				status = CommandStatus.complete;
				status.addStatus(wme);
				return;
			}
			
			if (status != CommandStatus.executing) {
				status = CommandStatus.executing;
				status.addStatus(wme);
			}
		}
	}

	@Override
	public void interrupt() {
		if (!status.isTerminated()) {
			status = CommandStatus.interrupted;
			status.addStatus(wme);
		}
	}
}
