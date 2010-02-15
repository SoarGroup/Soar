/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.command.Pose;
import edu.umich.soar.sproom.drive.DifferentialDriveCommand;
import edu.umich.soar.sproom.drive.DriveCommand;

import jmat.LinAlg;

import sml.Identifier;

/**
 * @author voigtjr
 *
 * Gracefully stop movement.
 * 
 * Returns accepted. Not interruptible. Creates DDC.
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
	protected OutputLinkCommand accept() {
		addStatus(CommandStatus.ACCEPTED);
		return this;
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
