/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Agent;
import sml.Identifier;

/**
 * @author voigtjr
 *
 * Emergency stop.
 */
final class EStopCommand extends DDCCommand implements Command {
	private static final Logger logger = Logger.getLogger(EStopCommand.class);
	static final String NAME = "estop";

	public DifferentialDriveCommand getDDC() {
		return DifferentialDriveCommand.newEStopCommand();
	}

	public boolean execute(InputLinkInterface inputLink, Agent agent,
			Identifier command, SplinterState splinter,
			OutputLinkManager outputLinkManager) {
		logger.debug(NAME + ":");
		CommandStatus.accepted.addStatus(agent, command);
		CommandStatus.complete.addStatus(agent, command);
		
		this.agent = agent;
		this.command = command;
		return true;
	}
}
