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
 * Set target heading and speed. Note, does not "complete" like set-heading (without
 * linear velocity) does.
 */
public class SetHeadingLinearCommand extends OutputLinkCommand implements DriveCommand {
	private static final Log logger = LogFactory.getLog(SetHeadingLinearCommand.class);
	private static final String YAW = "yaw";
	private static final String LINVEL = "linear-velocity";
	private static final double TOLERANCE = Math.toRadians(3);
	static final String NAME = "set-heading-linear";

	private final Identifier wme;
	private DifferentialDriveCommand ddc;
	private CommandStatus status = CommandStatus.accepted;
	private double targetYaw;
	
	SetHeadingLinearCommand(Identifier wme) {
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
		targetYaw = Math.toRadians(targetYaw);

		double linearVelocity;
		try {
			linearVelocity = Double.parseDouble(wme.GetParameterValue(LINVEL));
			linearVelocity = CommandConfig.CONFIG.speedFromView(linearVelocity);
		} catch (NullPointerException ex) {
			return new InvalidCommand(wme, "No " + LINVEL + " on command");
		} catch (NumberFormatException e) {
			return new InvalidCommand(wme, "Unable to parse " + LINVEL + ": " + wme.GetParameterValue(LINVEL));
		}

		ddc = DifferentialDriveCommand.newHeadingLinearVelocityCommand(targetYaw, linearVelocity);
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
