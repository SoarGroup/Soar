/**
 * 
 */
package edu.umich.soar.sproom.control;

import sml.Identifier;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * @author voigtjr
 *
 * Emergency stop.
 */
public class EStopCommand extends DDCCommand implements Command {
	private static final Log logger = LogFactory.getLog(EStopCommand.class);
	static final String NAME = "estop";

	static Command newInstance() {
		return new EStopCommand();
	}
	
	private EStopCommand() {
	}

	@Override
	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newEStopCommand();
	}

	@Override
	public boolean execute(Identifier command) {
		logger.debug(NAME + ":");
		CommandStatus.accepted.addStatus(command);
		CommandStatus.complete.addStatus(command);
		
		this.command = command;
		return true;
	}
}
