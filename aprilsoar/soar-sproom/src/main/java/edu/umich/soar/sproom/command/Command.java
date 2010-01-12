package edu.umich.soar.sproom.command;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import jmat.LinAlg;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import april.config.Config;

import edu.umich.soar.sproom.HzChecker;
import edu.umich.soar.sproom.drive.DifferentialDriveCommand;
import edu.umich.soar.sproom.drive.Drive3;
import edu.umich.soar.sproom.soar.SoarInterface;

public class Command {
	private static final Log logger = LogFactory.getLog(Command.class);

	public static double clamp(double value, double min, double max) {
		value = Math.max(value, min);
		value = Math.min(value, max);
		return value;
	}

	private final HzChecker hzChecker = HzChecker.newInstance(Command.class.toString());
	private final Pose pose = new Pose();
	private final Drive3 drive3 = Drive3.newInstance(pose);
	private final Waypoints waypoints = new Waypoints();
	private final Comm comm = new Comm();
	private final Lidar lidar = new Lidar();
	private final MapMetadata metadata;
	private final VirtualObjects vobjs;
	private final SoarInterface soar;
	private final HttpController httpController = new HttpController();
	private Gamepad gp;
	private final ScheduledExecutorService shexec = Executors
			.newSingleThreadScheduledExecutor();
	private boolean override = false;

    private enum GamepadInputScheme {
        JOY_MOTOR,              // left: off, right x: turn component, right y: forward component
        TANK,                   // left y: left motor, right y: right motor
        JOY_VELOCITIES, 		// left: off, right x: linvel, right y: angvel
        GAS_AND_WHEEL,  		// left y: linvel, right: heading, right center: angvel -> 0
    }
    private GamepadInputScheme gpInputScheme = GamepadInputScheme.JOY_MOTOR;

	public Command(Config config) {
		logger.debug("Command started");

		metadata = new MapMetadata(config);
		vobjs = new VirtualObjects(config);
		soar = new SoarInterface(pose, waypoints, comm, lidar, metadata, vobjs);
		httpController.addDriveListener(drive3);
		soar.addDriveListener(drive3);
		httpController.addSoarControlListener(soar);

		try {
			gp = new Gamepad();
			Buttons.setGamepad(gp);
			shexec.scheduleAtFixedRate(new Runnable() {
				@Override
				public void run() {
					if (logger.isDebugEnabled()) {
						hzChecker.tick();
					}

					Buttons.OVERRIDE.update();
					if (override) {
						if (Buttons.OVERRIDE.checkAndDisable()) {
							soar.addDriveListener(drive3);
							override = false;
							logger.info("Override disabled.");
						} else {
							drive3.handleDriveEvent(getGamepadDDC());
						}
					} else {
						if (Buttons.OVERRIDE.checkAndDisable()) {
							soar.removeDriveListener(drive3);
							override = true;
							drive3.handleDriveEvent(getGamepadDDC());
							logger.info("Override enabled.");
						}
					}
					
					Buttons.SOAR.update();
					if (Buttons.SOAR.checkAndDisable()) {
						soar.toggleRunState();
					}
				}
			}, 0, 30, TimeUnit.MILLISECONDS);
		} catch (IllegalStateException e) {
			logger.warn("No joystick.");
		}
	}
	
	private DifferentialDriveCommand getGamepadDDC() {
		if (gp == null) {
			return null;
		}
		
        //double left_x;
        double right_x = gp.getAxis(2);
        double left_y = gp.getAxis(1) * -1;
        double right_y = gp.getAxis(3) * -1;
        
        double left = 0;
        double right = 0;
        
        DifferentialDriveCommand ddc = DifferentialDriveCommand.newEStopCommand();
        CommandConfig c = CommandConfig.CONFIG;
        
        switch (gpInputScheme) {
        case JOY_MOTOR:
                // this should not be linear, it is difficult to precisely control
                double fwd = right_y; // +1 = forward, -1 = back
                double lr = -1 * right_x; // +1 = left, -1 = right

                left = fwd - lr;
                right = fwd + lr;

                double max = Math.max(Math.abs(left), Math.abs(right));
                if (max > 1) {
                        left /= max;
                        right /= max;
                }

                if (Buttons.SLOW.isEnabled()) {
                        left *= 0.5;
                        right *= 0.5;
                }
                ddc = DifferentialDriveCommand.newMotorCommand(left, right);
                break;
                
        case TANK:
                left = left_y;
                right = right_y;

                if (Buttons.SLOW.isEnabled()) {
                        left *= 0.5;
                        right *= 0.5;
                }
                ddc = DifferentialDriveCommand.newMotorCommand(left, right);
                break;
                
        case JOY_VELOCITIES:
                double angvel = c.getLimitAngVelMax() * right_x * -1;
                double linvel = c.getLimitLinVelMax() * right_y;
                ddc = DifferentialDriveCommand.newVelocityCommand(angvel, linvel);
                break;
                
        case GAS_AND_WHEEL:
                linvel = c.getLimitLinVelMax() * left_y;
                
                double magnitude = LinAlg.magnitude(new double[] { right_x, right_y } );
                if (magnitude < c.getGamepadZeroThreshold()) {
                        // angvel to 0
                        ddc = DifferentialDriveCommand.newVelocityCommand(0, linvel);
                } else {
                        double heading = Math.atan2(right_y, right_x);
                        ddc = DifferentialDriveCommand.newHeadingLinearVelocityCommand(heading, linvel);
                }
                break;
        }
        
        return ddc;		
	}
}
