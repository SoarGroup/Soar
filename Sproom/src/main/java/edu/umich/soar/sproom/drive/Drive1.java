package edu.umich.soar.sproom.drive;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import lcm.lcm.LCM;
import april.lcmtypes.differential_drive_command_t;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.Command;

/**
 * Lowest level drive command abstraction, takes throttles and transmits them on a drive channel.
 *
 * @author voigtjr@gmail.com
 */
class Drive1 {
	private static final Log logger = LogFactory.getLog(Drive1.class);
	
	private final differential_drive_command_t dc = new differential_drive_command_t();
	private final LCM lcm = LCM.getSingleton();

	Drive1() {
		dc.left_enabled = true;
		dc.right_enabled = true;
	}
	
	void estop() {
		setMotors(0, 0);
		transmit(dc);
	}
	
	void setMotors(double left, double right) {
		// set motors
		dc.left = left;
		dc.right = right;
	}
	
	double getLeft() {
		return dc.left;
	}
	
	double getRight() {
		return dc.right;
	}
	
	void update() {
		// transmit dc
		transmit(dc);
	}
	
	private void transmit(differential_drive_command_t dc) {
		dc.utime = System.nanoTime();
		dc.left = Command.clamp(dc.left, -1, 1);
		dc.right = Command.clamp(dc.right, -1, 1);
		
		if (logger.isTraceEnabled()) {
			logger.trace(String.format("transmit: %2.3f, %2.3f", dc.left, dc.right));
		}
		lcm.publish(SharedNames.DRIVE_CHANNEL, dc);
	}
}

