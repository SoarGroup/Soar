/**
 * 
 */
package org.msoar.sps.control.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class RotateCommand implements Command {
	private static Logger logger = Logger.getLogger(RotateCommand.class);

	private enum Direction {
		left,
		right,
		stop;
	}
	
	private Direction direction;
	private double throttle;

	public CommandStatus execute(Identifier command, pose_t pose, OutputLinkManager outputLinkManager) {
		command.GetTimeTag();
		String directionString = command.GetParameterValue("direction");
		if (directionString == null) {
			logger.warn("No direction on rotate command");
			return CommandStatus.error;
		}

		direction = Direction.valueOf(directionString);
		if (direction == null) {
			logger.warn("Unknown direction on rotate command: " + directionString);
			return CommandStatus.error;
		}

		try {
			throttle = Double.parseDouble(command.GetParameterValue("throttle"));
		} catch (NullPointerException ex) {
			logger.warn("No throttle on rotate command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse throttle: " + command.GetParameterValue("throttle"));
			return CommandStatus.error;
		}

		throttle = Math.max(throttle, 0);
		throttle = Math.min(throttle, 1.0);

		logger.debug(String.format("rotate: %10s %10.3f", direction, throttle));
		
		return direction == Direction.stop ? CommandStatus.complete : CommandStatus.executing;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean modifiesInput() {
		return true;
	}

	public void updateInput(SplinterInput input) {
		if (direction == Direction.left) {
			input.rotate(SplinterInput.Direction.left, throttle);
		} else if (direction == Direction.right) {
			input.rotate(SplinterInput.Direction.right, throttle);
		} else {
			assert direction == Direction.stop;
			input.stop();
		}
	}
}