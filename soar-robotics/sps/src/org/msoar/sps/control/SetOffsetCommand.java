/**
 * 
 */
package org.msoar.sps.control;

import org.apache.log4j.Logger;

import sml.Identifier;

final class SetOffsetCommand implements Command {
	private static final Logger logger = Logger.getLogger(SetOffsetCommand.class);
	static final String NAME = "set-offset";
	
	public CommandStatus execute(InputLinkInterface inputLink, Identifier command, SplinterState splinter, OutputLinkManager outputLinkManager) {
		if (splinter == null) {
			throw new AssertionError();
		}

		double[] offset = null;
		try {
			offset = new double[] {
					Double.parseDouble(command.GetParameterValue("x")),
					Double.parseDouble(command.GetParameterValue("y")),
			};
		} catch (NullPointerException ignored) {
			logger.warn(NAME + ": Missing coordinates");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn(NAME + ": Error parsing coordinates");
			return CommandStatus.error;
		}
		
		logger.debug(String.format("%s: x%10.3f y%10.3f", NAME, offset[0], offset[1]));
		splinter.setOffset(offset);
		return CommandStatus.complete;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean createsDDC() {
		return false;
	}

	public DifferentialDriveCommand getDDC() {
		throw new AssertionError();
	}
}