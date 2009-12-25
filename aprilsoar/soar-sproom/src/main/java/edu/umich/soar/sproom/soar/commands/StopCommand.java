/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

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
	private static final Log logger = LogFactory.getLog(StopCommand.class);
	private static final double TOLERANCE = 0.01; // meters per second
	static final String NAME = "stop";

	private final Identifier wme;
	private final DifferentialDriveCommand ddc = DifferentialDriveCommand.newVelocityCommand(0, 0);
	private CommandStatus status = CommandStatus.accepted;
	
	StopCommand(Identifier wme) {
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
		logger.debug(NAME + ":");
		CommandStatus.accepted.addStatus(wme);
		return this;
	}

	@Override
	public void update(Adaptable app) {
		if (status != CommandStatus.complete) {
			Pose pose = (Pose)app.getAdapter(Pose.class);
			
			if (Double.compare(LinAlg.magnitude(pose.getPose().vel), TOLERANCE) < 0) {
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
