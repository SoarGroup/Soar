package edu.umich.soar.sproom.command;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

import jmat.LinAlg;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import april.config.Config;

import edu.umich.soar.sproom.HzChecker;
import edu.umich.soar.sproom.drive.DifferentialDriveCommand;
import edu.umich.soar.sproom.drive.Drive3;
import edu.umich.soar.sproom.gp.GPComponentListener;
import edu.umich.soar.sproom.gp.GamepadJInput;
import edu.umich.soar.sproom.soar.SoarInterface;

/**
 * Top level container for components that ultimately produce drive commands
 * for splinter.
 *
 * @author voigtjr@gmail.com
 */
public class Command {
	private static final Log logger = LogFactory.getLog(Command.class);

	public static double clamp(double value, double min, double max) {
		value = Math.max(value, min);
		value = Math.min(value, max);
		return value;
	}

	private final HzChecker hzChecker = HzChecker.newInstance(Command.class.toString());
	private final Pose pose = new Pose();
	private final Drive3 drive3;
	private final Waypoints waypoints = new Waypoints();
	private final Comm comm = new Comm();
	private final Lidar lidar;
	private final MapMetadata metadata;
	private final VirtualObjects vobjs;
	private final SoarInterface soar;
	private final HttpController httpController = new HttpController();
	private final ScheduledExecutorService shexec = Executors.newSingleThreadScheduledExecutor();

    private enum GamepadInputScheme {
        JOY_MOTOR,              // left: off, right x: turn component, right y: forward component
        TANK,                   // left y: left motor, right y: right motor
        JOY_VELOCITIES, 		// left: off, right x: linvel, right y: angvel
        GAS_AND_WHEEL,  		// left y: linvel, right: heading, right center: angvel -> 0
    }
    private GamepadInputScheme gpInputScheme = GamepadInputScheme.TANK;

    private GamepadJInput gpji = new GamepadJInput();
    private final AtomicBoolean override = new AtomicBoolean(false);
    private final AtomicBoolean slow = new AtomicBoolean(false);
    
    private float ly = 0;
    private float rx = 0;
    private float ry = 0;

    public Command(Config config) {
		logger.debug("Command started");
		
		CommandConfig.CONFIG.initialize(config);
		
		drive3 = Drive3.newInstance(pose);
		lidar = new Lidar(CommandConfig.CONFIG.getLidarCacheTime());
		metadata = new MapMetadata(config);
		vobjs = new VirtualObjects(config);
		soar = new SoarInterface(pose, waypoints, comm, lidar, metadata, vobjs);
		httpController.addDriveListener(drive3);
		soar.addDriveListener(drive3);
		httpController.addSoarControlListener(soar);
		
		if (gpji.isValid()) {
			initializeGamepad();
		}

		shexec.scheduleAtFixedRate(new Runnable() {
			@Override
			public void run() {
				if (logger.isDebugEnabled()) {
					hzChecker.tick();
				}
				
				if (override.get()) {
					drive3.handleDriveEvent(getGamepadDDC());
				}
			}
		}, 0, 30, TimeUnit.MILLISECONDS);
	}

    private void initializeGamepad() {
		gpji.addComponentListener(GamepadJInput.Id.OVERRIDE, new GPComponentListener() {
			@Override
			public void stateChanged(GamepadJInput.Id id, float value) {
				if (logger.isTraceEnabled()) {
					logger.trace(id + ": " + value);
				}
				if (Float.compare(value, 0) != 0) {
					if (override.compareAndSet(true, false)) {
						soar.addDriveListener(drive3);
						logger.info("Override disabled.");
						
					} else if (override.compareAndSet(false, true)) {
						soar.removeDriveListener(drive3);
						logger.info("Override enabled.");
					}
				}
			}
		});
		
		gpji.addComponentListener(GamepadJInput.Id.SOAR, new GPComponentListener() {
			@Override
			public void stateChanged(GamepadJInput.Id id, float value) {
				if (logger.isTraceEnabled()) {
					logger.trace(id + ": " + value);
				}
				if (Float.compare(value, 0) != 0) {
					soar.toggleRunState();
				}
			}
		});
		
		gpji.addComponentListener(GamepadJInput.Id.SLOW, new GPComponentListener() {
			@Override
			public void stateChanged(GamepadJInput.Id id, float value) {
				if (logger.isTraceEnabled()) {
					logger.trace(id + ": " + value);
				}
				if (Float.compare(value, 0) != 0) {
					if (slow.get()) {
						slow.set(false);
					} else {
						slow.set(true);
					}
				}
			}
		});
		
		gpji.addComponentListener(GamepadJInput.Id.GPMODE, new GPComponentListener() {
			@Override
			public void stateChanged(GamepadJInput.Id id, float value) {
				if (logger.isTraceEnabled()) {
					logger.trace(id + ": " + value);
				}
				if (Float.compare(value, 0) != 0) {
					int index = gpInputScheme.ordinal() + 1;
					index %= GamepadInputScheme.values().length;
					gpInputScheme = GamepadInputScheme.values()[index];
					logger.info("Input changed to " + gpInputScheme);
				}
			}
		});
		
		gpji.addComponentListener(GamepadJInput.Id.LY, new GPComponentListener() {
			@Override
			public void stateChanged(GamepadJInput.Id id, float value) {
				if (logger.isTraceEnabled()) {
					logger.trace(id + ": " + value);
				}
				ly = value;
			}
		});
		gpji.addComponentListener(GamepadJInput.Id.RX, new GPComponentListener() {
			@Override
			public void stateChanged(GamepadJInput.Id id, float value) {
				if (logger.isTraceEnabled()) {
					logger.trace(id + ": " + value);
				}
				rx = value;
			}
		});
		gpji.addComponentListener(GamepadJInput.Id.RY, new GPComponentListener() {
			@Override
			public void stateChanged(GamepadJInput.Id id, float value) {
				if (logger.isTraceEnabled()) {
					logger.trace(id + ": " + value);
				}
				ry = value;
			}
		});
    }
    
	private DifferentialDriveCommand getGamepadDDC() {
        double left = 0;
        double right = 0;
        
        DifferentialDriveCommand ddc = DifferentialDriveCommand.newEStopCommand();
        CommandConfig c = CommandConfig.CONFIG;
        
        switch (gpInputScheme) {
        case JOY_MOTOR:
                // this should not be linear, it is difficult to precisely control
                double fwd = -1 * ry; // +1 = forward, -1 = back
                double lr = -1 * rx; // +1 = left, -1 = right

                left = fwd - lr;
                right = fwd + lr;

                double max = Math.max(Math.abs(left), Math.abs(right));
                if (max > 1) {
                        left /= max;
                        right /= max;
                }

                if (slow.get()) {
                        left *= 0.5;
                        right *= 0.5;
                }
                ddc = DifferentialDriveCommand.newMotorCommand(left, right);
                break;
                
        case TANK:
                left = ly * -1;
                right = ry * -1;

                if (slow.get()) {
                        left *= 0.5;
                        right *= 0.5;
                }
                ddc = DifferentialDriveCommand.newMotorCommand(left, right);
                break;
                
        case JOY_VELOCITIES:
                double angvel = c.getLimitAngVelMax() * rx * -1;
                double linvel = c.getLimitLinVelMax() * ry * -1;
                ddc = DifferentialDriveCommand.newVelocityCommand(angvel, linvel);
                break;
                
        case GAS_AND_WHEEL:
                linvel = c.getLimitLinVelMax() * ly * -1;
                
                double magnitude = LinAlg.magnitude(new double[] { rx, ry * -1 } );
                if (magnitude < c.getGamepadZeroThreshold()) {
                        // angvel to 0
                        ddc = DifferentialDriveCommand.newVelocityCommand(0, linvel);
                } else {
                        double heading = Math.atan2(ry * -1, rx);
                        ddc = DifferentialDriveCommand.newHeadingLinearVelocityCommand(heading, linvel);
                }
                break;
        }
        
        return ddc;		
	}
}
