package splintersoar.orc;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.Timer;
import java.util.TimerTask;
import java.util.logging.Level;
import java.util.logging.Logger;

import jmat.LinAlg;
import jmat.MathUtil;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.pose_t;

import orc.Motor;
import orc.Orc;
import orc.OrcStatus;
import splintersoar.Configuration;
import splintersoar.LCMInfo;
import splintersoar.LogFactory;
import lcmtypes.splinterstate_t;
import lcmtypes.xy_t;
import splintersoar.pf.ParticleFilter;

import lcmtypes.differential_drive_command_t;

/**
 * @author voigtjr
 * Represents communication to the orc board. Listens for laser localization coordinates and drive commands.
 * Produces splinter pose objects indicating where the robot is probably at.
 */
public class OrcInterface implements LCMSubscriber {
	private Logger logger;

	private Configuration cnf;

	private Timer timer = new Timer();

	private Orc orc;
	private Motor[] motor = new Motor[2];

	private splinterstate_t previousState = new splinterstate_t();

	private double[] command = { 0, 0 };

	private LCM lcmL1;
	private LCM lcmGG;

	private xy_t laserxy;

	private double[] initialxy = null;

	differential_drive_command_t driveCommand;

	public OrcInterface(Configuration cnf) {
		this.cnf = cnf;

		previousState.pose = new pose_t();
		previousState.pose.orientation = new double[4];
		previousState.pose.pos = new double[3];

		logger = LogFactory.createSimpleLogger("OrcInterface", Level.INFO);

		try {
			logger.info(String.format("Using %s for %s provider URL.", LCMInfo.L1_NETWORK, LCMInfo.DRIVE_COMMANDS_CHANNEL));
			lcmL1 = new LCM(LCMInfo.L1_NETWORK);

		} catch (IOException e) {
			logger.severe("Error creating lcmin.");
			e.printStackTrace();
			System.exit(1);
		}
		lcmL1.subscribe(LCMInfo.DRIVE_COMMANDS_CHANNEL, this);

		try {
			logger.info(String.format("Using %s for %s provider URL.", LCMInfo.GG_NETWORK, LCMInfo.COORDS_CHANNEL));
			lcmGG = new LCM(LCMInfo.GG_NETWORK);

		} catch (IOException e) {
			logger.severe("Error creating lcmin.");
			e.printStackTrace();
			System.exit(1);
		}
		
		lcmGG.subscribe(LCMInfo.COORDS_CHANNEL, this);

		orc = Orc.makeOrc();
		motor[0] = new Motor(orc, cnf.orc.ports[0], cnf.orc.invert[0]);
		motor[1] = new Motor(orc, cnf.orc.ports[1], cnf.orc.invert[1]);

		logger.info("Orc up");
		timer.schedule(new UpdateTask(), 0, 1000 / cnf.orc.updateHz);
	}

	public void shutdown() {
		timer.cancel();
		logger.info("Orc down");
	}

	class UpdateTask extends TimerTask {
		int runs = 0;
		ParticleFilter pf;
		double[] yawCalcXY;
		
		UpdateTask() {
			if (cnf.orc.usePF) {
				 pf = new ParticleFilter(cnf);
			}
		}

		@Override
		public void run() {
			boolean moving;
			splinterstate_t currentState = new splinterstate_t();
			{
				// Get OrcStatus
				OrcStatus currentStatus = orc.getStatus();

				// calculate location if moving
				moving = (currentStatus.qeiVelocity[0] != 0) || (currentStatus.qeiVelocity[1] != 0);

				// assemble output
				currentState.utime = currentStatus.utime;
				currentState.leftodom = currentStatus.qeiPosition[cnf.orc.ports[0]] * (cnf.orc.invert[0] ? -1 : 1);
				currentState.rightodom = currentStatus.qeiPosition[cnf.orc.ports[1]] * (cnf.orc.invert[1] ? -1 : 1);
			}

			// update pose
			if (moving) {
				// odometry
				double[] deltaxyt = calculateDeltaXYT(currentState);

				// laser
				double[] adjustedlaserxy = getAdjustedLaserXY();

				boolean doOdometry = true;
				
				// calculate pose
				if (cnf.orc.usePF) {
					// propagate odometry
					pf.propagate(deltaxyt);
					
					// adjustedlaserxy could be null
					if (adjustedlaserxy != null && shouldUpdatePF()) {
						currentState.pose = pf.update(adjustedlaserxy);

						double yaw = LinAlg.quatToRollPitchYaw(currentState.pose.orientation)[2];
						if (logger.isLoggable(Level.FINER)) {
							logger.finer(String.format("pf: %5.2f %5.2f %5.1f", currentState.pose.pos[0], currentState.pose.pos[1], Math.toDegrees(yaw)));
						}
						
						doOdometry = false;
					} else {
						currentState.pose = previousState.pose.copy();
					}
						
				} else {
					currentState.pose = previousState.pose.copy();

					if (adjustedlaserxy != null) {
						if (yawCalcXY == null) {
							yawCalcXY = Arrays.copyOf(adjustedlaserxy, adjustedlaserxy.length);
							
						} else if (LinAlg.distance(yawCalcXY, adjustedlaserxy) > cnf.orc.laserThreshold) {
							currentState.pose.pos[0] = adjustedlaserxy[0];
							currentState.pose.pos[1] = adjustedlaserxy[1];

							double[] rpy = { 0, 0, 0 };
							rpy[2] = Math.atan2(adjustedlaserxy[1] - yawCalcXY[1], adjustedlaserxy[0] - yawCalcXY[0]);
							rpy[2] = MathUtil.mod2pi(rpy[2]);
							currentState.pose.orientation = LinAlg.rollPitchYawToQuat(rpy);

							yawCalcXY = Arrays.copyOf(adjustedlaserxy, adjustedlaserxy.length);

							if (logger.isLoggable(Level.FINER)) {
								logger.finer(String.format("laser: %5.2f %5.2f %5.1f", currentState.pose.pos[0], currentState.pose.pos[1], Math
										.toDegrees(rpy[2])));
							}

							doOdometry = false;
						}
					}
				}

				if (doOdometry) {
					currentState.pose.pos[0] += deltaxyt[0];
					currentState.pose.pos[1] += deltaxyt[1];

					double[] rpy = LinAlg.quatToRollPitchYaw(previousState.pose.orientation);
					rpy[2] += deltaxyt[2];
					rpy[2] = MathUtil.mod2pi(rpy[2]);
					currentState.pose.orientation = LinAlg.rollPitchYawToQuat(rpy);

					if (logger.isLoggable(Level.FINER)) {
						logger.finer(String.format(" odom: %5.2f %5.2f %5.1f", currentState.pose.pos[0], currentState.pose.pos[1], Math.toDegrees(rpy[2])));
					}
				}

			} else {
				currentState.pose = previousState.pose.copy();
			}
			currentState.pose.utime = currentState.utime;

			// publish pose
			lcmGG.publish(LCMInfo.SPLINTER_STATE_CHANNEL, currentState);

			// command motors
			commandMotors(currentState.utime);

			previousState = currentState;
		}

		long lastPFUpdate = 0;
		private boolean shouldUpdatePF() {
			long current = System.nanoTime();
			if ((current - lastPFUpdate) > cnf.orc.pfUpdatePeriodNanos) {
				lastPFUpdate = current;
				return true;
			}
			return false;
		}

		double[] calculateDeltaXYT(splinterstate_t currentState) {
			double dleft = (currentState.leftodom - previousState.leftodom) * cnf.orc.tickMeters;
			double dright = (currentState.rightodom - previousState.rightodom) * cnf.orc.tickMeters;
			double phi = (dright - dleft) / cnf.orc.baselineMeters;
			double dcenter = (dleft + dright) / 2;

			phi = MathUtil.mod2pi(phi);

			double theta = LinAlg.quatToRollPitchYaw(previousState.pose.orientation)[2];
			theta = MathUtil.mod2pi(theta);

			return new double[] { dcenter * Math.cos(theta), dcenter * Math.sin(theta), phi };
		}

		double[] getAdjustedLaserXY() {
			if (laserxy == null)
				return null;

			xy_t laserxycopy = laserxy.copy();

			laserxy = null;

			if (initialxy == null) {
				initialxy = Arrays.copyOf(laserxycopy.xy, 2);
				if (logger.isLoggable(Level.FINE)) {
					logger.fine(String.format("initialxy: %5.2f, %5.2f%n", initialxy[0], initialxy[1]));
				}
			}

			return LinAlg.subtract(laserxycopy.xy, initialxy);
		}
	}

	@Override
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(LCMInfo.COORDS_CHANNEL)) {
			if (laserxy != null) {
				return;
			}

			try {
				laserxy = new xy_t(ins);
			} catch (IOException ex) {
				logger.warning("Error decoding laserxy message: " + ex);
			}
		} else if (channel.equals(LCMInfo.DRIVE_COMMANDS_CHANNEL)) {
			try {
				driveCommand = new differential_drive_command_t(ins);
			} catch (IOException ex) {
				logger.warning("Error decoding differential_drive_command_t message: " + ex);
			}
		}
	}

	long lastDriveCommandSeen = 0;
	private void commandMotors(long utime) {
		long current = System.nanoTime();
		if (driveCommand == null) {
			if (current - lastDriveCommandSeen > 5000000000L ) {
				motor[0].setPWM(command[0]);
				motor[1].setPWM(command[1]);
			}
			return;
		}
		lastDriveCommandSeen = current;
		
		differential_drive_command_t newDriveCommand = driveCommand.copy();

		if (logger.isLoggable(Level.FINEST))
			logger.finest(String.format("Got input %f %f", newDriveCommand.left, newDriveCommand.right));

		if (newDriveCommand.left_enabled) {
			newDriveCommand.left = Math.min(newDriveCommand.left, 1.0);
			newDriveCommand.left = Math.max(newDriveCommand.left, -1.0);

			double delta = newDriveCommand.left - command[0];
			
			if (logger.isLoggable(Level.FINEST))
				logger.finest(String.format("Delta %f, Max: %f", delta, cnf.orc.maxThrottleChangePerUpdate));

			if (delta > 0) {
				delta = Math.min(delta, cnf.orc.maxThrottleChangePerUpdate);
			} else if (delta < 0) {
				delta = Math.max(delta, -1 * cnf.orc.maxThrottleChangePerUpdate);
			}

			command[0] += delta;
		}

		if (newDriveCommand.right_enabled) {
			newDriveCommand.right = Math.min(newDriveCommand.right, 1.0);
			newDriveCommand.right = Math.max(newDriveCommand.right, -1.0);

			double delta = newDriveCommand.right - command[1];

			if (logger.isLoggable(Level.FINEST))
				logger.finest(String.format("Delta %f, Max: %f", delta, cnf.orc.maxThrottleChangePerUpdate));

			if (delta > 0) {
				delta = Math.min(delta, cnf.orc.maxThrottleChangePerUpdate);
			} else if (delta < 0) {
				delta = Math.max(delta, -1 * cnf.orc.maxThrottleChangePerUpdate);
			}
			command[1] += delta;
		}

		if (logger.isLoggable(Level.FINEST))
			logger.finest(String.format("Sending input %f %f", command[0], command[1]));

		motor[0].setPWM(command[0]);
		motor[1].setPWM(command[1]);
	}
}
