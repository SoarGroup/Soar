/**
 * 
 */
package org.msoar.sps.control.o;

import org.apache.log4j.Logger;

import lcmtypes.pose_t;
import sml.Identifier;

class MoveCommand implements Command {
	private static Logger logger = Logger.getLogger(MoveCommand.class);
	
	private enum Direction {
		forward,
		backward,
		stop;
	}
	
	private Direction direction;
	private double throttle;
	
	public CommandStatus execute(Identifier command, pose_t pose, OutputLinkManager outputLinkManager) {
		String directionString = command.GetParameterValue("direction");
		if (directionString == null) {
			logger.warn("No direction on move command");
			return CommandStatus.error;
		}
		
		direction = Direction.valueOf(directionString);
		if (direction == null) {
			logger.warn("Unknown direction on move command: " + directionString);
			return CommandStatus.error;
		}

		try {
			throttle = Double.parseDouble(command.GetParameterValue("throttle"));
		} catch (NullPointerException ex) {
			logger.warn("No throttle on move command");
			return CommandStatus.error;
		} catch (NumberFormatException e) {
			logger.warn("Unable to parse throttle: " + command.GetParameterValue("throttle"));
			return CommandStatus.error;
		}

		throttle = Math.max(throttle, 0);
		throttle = Math.min(throttle, 1.0);

		logger.debug(String.format("move: %10s %10.3f", direction, throttle));

		return direction == Direction.stop ? CommandStatus.complete : CommandStatus.executing;
	}

	public boolean isInterruptable() {
		return false;
	}

	public boolean modifiesInput() {
		return true;
	}

	public void updateInput(SplinterInput input) {
		if (direction == Direction.backward) {
			input.move(throttle * -1);
		} else if (direction == Direction.forward) {
			input.move(throttle);
		} else {
			assert direction == Direction.stop;
			input.stop();
		}
	}
}