/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.command.Pose;
import edu.umich.soar.sproom.drive.DifferentialDriveCommand;
import edu.umich.soar.sproom.drive.DriveCommand;

import april.jmat.LinAlg;

import sml.Identifier;

/**
 * Gracefully stop movement.
 *
 * @author voigtjr@gmail.com
 */
public class StopCommand extends OutputLinkCommand implements DriveCommand {
	private static final double TOLERANCE = 0.01; // meters per second
	static final String NAME = "stop";

	private final DifferentialDriveCommand ddc = DifferentialDriveCommand.newVelocityCommand(0, 0);
	
	public StopCommand(Identifier wme) {
		super(wme);
	}

	@Override
	public DifferentialDriveCommand getDDC() {
		return ddc;
	}

	@Override
	protected boolean accept() {
		addStatus(CommandStatus.ACCEPTED);
		return true;
	}

	@Override
	public void update(Adaptable app) {
		Pose pose = (Pose)app.getAdapter(Pose.class);
		
		if (Double.compare(LinAlg.magnitude(pose.getPose().vel), TOLERANCE) < 0) {
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
