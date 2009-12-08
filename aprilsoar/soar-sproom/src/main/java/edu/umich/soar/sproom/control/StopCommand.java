/**
 * 
 */
package edu.umich.soar.sproom.control;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import jmat.LinAlg;
import lcmtypes.pose_t;

import sml.Identifier;

/**
 * @author voigtjr
 *
 * Gracefully stop movement.
 * 
 * Returns accepted. Not interruptible. Creates DDC.
 */
final public class StopCommand extends DDCCommand implements Command {
	private static final Log logger = LogFactory.getLog(StopCommand.class);
	private static final double TOLERANCE = 0.01; // meters per second
	static final String NAME = "stop";

	static Command newInstance(OffsetPose opose) {
		return new StopCommand(opose);
	}
	
	private StopCommand(OffsetPose opose) {
		this.opose = opose;
	}

	private final OffsetPose opose;
	private CommandStatus status;
	
	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newVelocityCommand(0, 0);
	}

	@Override
	public boolean execute(Identifier command) {
		if (this.command != null || this.status != null) {
			throw new IllegalStateException();
		}

		logger.debug(NAME + ":");
		CommandStatus.accepted.addStatus(command);
		status = CommandStatus.accepted;

		this.command = command;
		return true;
	}

	@Override
	public void interrupt() {
		if (command == null || status == null) {
			throw new IllegalStateException();
		}
		
		CommandStatus.interrupted.addStatus(command);
		command = null;
		status = null;
	}

	@Override
	public boolean update() {
		if (command == null || status == null) {
			throw new IllegalStateException();
		}
		
		pose_t pose = opose.getPose();
		
		if (Double.compare(LinAlg.magnitude(pose.vel), TOLERANCE) < 0) {
			CommandStatus.complete.addStatus(command);
			command = null;
			status = null;
			return true; // complete
		} else if (status == CommandStatus.accepted) {
			CommandStatus.executing.addStatus(command);
			status = CommandStatus.executing;
		}
		return false; // executing
	}
}
