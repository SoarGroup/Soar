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
 * Set target heading and linear velocity. 
 * 
 * @author voigtjr@gmail.com
 */
public class SetHeadingLinearCommand extends OutputLinkCommand implements DriveCommand {
	private static final Log logger = LogFactory.getLog(SetHeadingLinearCommand.class);
	private static final String YAW = "yaw";
	private static final String LINVEL = "linear-velocity";
	private static final double TOLERANCE = Math.toRadians(3);
	static final String NAME = "set-heading-linear";

	private final Identifier wme;
	private DifferentialDriveCommand ddc;
	private double targetYaw;
	
	public SetHeadingLinearCommand(Identifier wme) {
		super(wme);
		this.wme = wme;
	}

	@Override
	public DifferentialDriveCommand getDDC() {
		return ddc;
	}

	@Override
	protected boolean accept() {
		try {
			targetYaw = Double.parseDouble(wme.GetParameterValue(YAW));
			targetYaw = CommandConfig.CONFIG.angleFromView(targetYaw);
		} catch (NullPointerException ex) {
			addStatusError("No " + YAW + " on command");
			return false;
		} catch (NumberFormatException e) {
			addStatusError("Unable to parse " + YAW + ": " + wme.GetParameterValue(YAW));
			return false;
		}
		targetYaw = Math.toRadians(targetYaw);

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

		ddc = DifferentialDriveCommand.newHeadingLinearVelocityCommand(targetYaw, linearVelocity);
		logger.debug(ddc);
		addStatus(CommandStatus.ACCEPTED);
		return true;
	}

	@Override
	public void update(Adaptable app) {
		Pose pose = (Pose)app.getAdapter(Pose.class);
		double currentYaw = Pose.getYaw(pose.getPose());
		double difference = targetYaw - currentYaw;
		difference = Math.abs(difference);
		
		if (Double.compare(difference, TOLERANCE) < 0) {
			addStatus(CommandStatus.COMPLETE);
			return;
		}
		
		addStatus(CommandStatus.EXECUTING);
	}

	@Override
	public void interrupt() {
		addStatus(CommandStatus.INTERRUPTED);
	}
}
