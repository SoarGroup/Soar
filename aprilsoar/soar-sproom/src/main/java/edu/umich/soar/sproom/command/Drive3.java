package edu.umich.soar.sproom.command;

import jmat.LinAlg;
import jmat.MathUtil;
import lcmtypes.pose_t;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.command.DifferentialDriveCommand.CommandType;

class Drive3 {
	private static final Log logger = LogFactory.getLog(Drive3.class);
	static final String HEADING_PID_NAME = "heading";

	private final Drive2 drive2;
	private final PIDController hController = new PIDController(HEADING_PID_NAME);
	private long utimePrev;
	private DifferentialDriveCommand.CommandType previousType;
	private double previousHeading;

	Drive3(Drive2 drive2) {
		this.drive2 = drive2;
		updateGains();
	}

	void updateGains() {
		double[] pid = CommandConfig.CONFIG.getGains(hController.getName());
		hController.setGains(pid);
	}
	
	void update(DifferentialDriveCommand ddc, pose_t pose) {
		long utimeElapsed = 0;
		if (pose != null) {
			utimeElapsed = pose.utime - utimePrev;
			utimePrev = pose.utime;
			
			if (ddc == null) {
				logger.warn("ddc null, sending estop");
				drive2.estop();
			} else {
				
				switch (ddc.getType()) {
				case ESTOP:
					drive2.estop();
					break;
					
				case MOTOR:
					drive2.setMotors(ddc.getLeft(), ddc.getRight());
					break;
					
				case VEL:
					drive2.setAngularVelocity(ddc.getAngularVelocity());
					drive2.setLinearVelocity(ddc.getLinearVelocity());
					break;
					
				case LINVEL:
					doLinearVelocity(ddc);
					break;
					
				case ANGVEL:
					if (previousType == CommandType.HEADING) {
						drive2.setLinearVelocity(0);
					}
					drive2.setAngularVelocity(ddc.getAngularVelocity());
					break;
					
				case HEADING_LINVEL:
					doLinearVelocity(ddc);
					// falls through
					
				case HEADING:
					if (previousType != CommandType.HEADING || Double.compare(previousHeading, ddc.getHeading()) != 0) {
						hController.clearIntegral();
						previousHeading = ddc.getHeading();
						// TODO: do we want to do this?
						//hardware.setLinearVelocity(0);
					}
					
					double target = MathUtil.mod2pi(ddc.getHeading());
					double actual = MathUtil.mod2pi(LinAlg.quatToRollPitchYaw(pose.orientation)[2]);
					double dt = utimeElapsed / 1000000.0;
					double out = hController.computeMod2Pi(dt, target, actual);
					drive2.setAngularVelocity(out);
					break;
				}
			
				this.previousType = ddc.getType();
			}
		}

		drive2.update(pose, utimeElapsed);
	}

	private void doLinearVelocity(DifferentialDriveCommand ddc) {
		if (previousType == CommandType.HEADING) {
			drive2.setAngularVelocity(0);
		}
		drive2.setLinearVelocity(ddc.getLinearVelocity());
	}
	
}
