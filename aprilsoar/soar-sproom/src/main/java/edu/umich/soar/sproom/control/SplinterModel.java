package edu.umich.soar.sproom.control;

import jmat.LinAlg;
import jmat.MathUtil;
import lcmtypes.pose_t;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.control.DifferentialDriveCommand.CommandType;
import edu.umich.soar.sproom.control.PIDController.Gains;

class SplinterModel implements OffsetPose {
	private static final Log logger = LogFactory.getLog(SplinterModel.class);

	static SplinterModel newInstance() {
		return new SplinterModel();
	}
	
	private final LCMProxy lcmProxy;
	private final SplinterHardware hardware;
	private final pose_t pose = new pose_t();
	private DifferentialDriveCommand ddc;
	private DifferentialDriveCommand.CommandType previousType;
	private PIDController headingController = new PIDController();	// experimentally derived in lab
	private double previousHeading;
	private final pose_t offset = new pose_t();
	
	private SplinterModel() {
		this.lcmProxy = LCMProxy.getInstance();
		this.hardware = SplinterHardware.newInstance(this.lcmProxy);
		
		headingController.setGains(new Gains(1, 0, 0.125));
		
		if (pose.utime != 0) {
			throw new AssertionError();
		}
	}
	
	void setAGains(Gains g) {
		hardware.setAGains(g);
	}
	
	void setLGains(Gains g) {
		hardware.setLGains(g);
	}
	
	void setHGains(Gains g) {
		logger.info(String.format("Heading gains: p%f i%f d%f", g.p, g.i, g.d));
		headingController.setGains(g);
	}
	
	Gains getAGains() {
		return hardware.getAGains();
	}
	
	Gains getLGains() {
		return hardware.getLGains();
	}
	
	Gains getHGains() {
		return headingController.getGains();
	}
	
	void update(DifferentialDriveCommand ddc) {
		this.ddc = ddc;
		
		// elapsed time
		double dt = updatePose();
		if (Double.compare(dt, 0) == 0) {
			// invalid state
			return;
		}
		
		hardware.setPose(pose);
		updateDC(dt);
		
		if (ddc != null) {
			this.previousType = ddc.getType();
		}
	}
	
	private void updateDC(double dt) {
		hardware.setUTime(pose.utime);

		if (ddc == null) {
			logger.warn("ddc null, sending estop");
			hardware.estop();
			return;
		}
		
		switch (ddc.getType()) {
		case ESTOP:
			hardware.estop();
			return;
			
		case MOTOR:
			hardware.setMotors(ddc.getLeft(), ddc.getRight());
			return;
			
		case VEL:
			hardware.setAngularVelocity(ddc.getAngularVelocity());
			hardware.setLinearVelocity(ddc.getLinearVelocity());
			return;
			
		case LINVEL:
			doLinearVelocity();
			return;
			
		case ANGVEL:
			if (previousType == CommandType.HEADING) {
				hardware.setLinearVelocity(0);
			}
			hardware.setAngularVelocity(ddc.getAngularVelocity());
			return;
			
		case HEADING_LINVEL:
			doLinearVelocity();
			// falls through
			
		case HEADING:
			if (previousType != CommandType.HEADING || previousHeading != ddc.getHeading()) {
				headingController.clearIntegral();
				previousHeading = ddc.getHeading();
				// TODO: do we want to do this?
				//hardware.setLinearVelocity(0);
			}
			
			double target = MathUtil.mod2pi(ddc.getHeading());
			double actual = MathUtil.mod2pi(LinAlg.quatToRollPitchYaw(pose.orientation)[2]);
			double out = headingController.computeMod2Pi(dt, target, actual);
			hardware.setAngularVelocity(out);
			return;
		}
		
		throw new AssertionError("Not implemented");
	}
	
	private void doLinearVelocity() {
		if (previousType == CommandType.HEADING) {
			hardware.setAngularVelocity(0);
		}
		hardware.setLinearVelocity(ddc.getLinearVelocity());
	}
	
	private void bootstrapPose(pose_t newPose) {
		updatePose(newPose);
		assert Double.compare(newPose.pos[2], 0) == 0;
		assert Double.compare(newPose.orientation[1], 0) == 0;
		assert Double.compare(newPose.orientation[2], 0) == 0;
		assert Double.compare(newPose.vel[0], 0) == 0;
		assert Double.compare(newPose.vel[1], 0) == 0;
		assert Double.compare(newPose.vel[2], 0) == 0;
		assert Double.compare(newPose.rotation_rate[0], 0) == 0;
		assert Double.compare(newPose.rotation_rate[1], 0) == 0;
		assert Double.compare(newPose.rotation_rate[2], 0) == 0;
		assert Double.compare(newPose.accel[0], 0) == 0;
		assert Double.compare(newPose.accel[1], 0) == 0;
		assert Double.compare(newPose.accel[2], 0) == 0;
	}

	private double updatePose() {
		// get the new pose
		pose_t lcmPose = lcmProxy.getPose();
		if (lcmPose == null) {
			logger.trace("No pose yet");
			return 0;
		}
		
		if (lcmPose.utime <= pose.utime) {
			logger.trace("Received old pose");
			return 0;
		}
		
		if (pose.utime == 0) {
			logger.trace("Received first pose");
			bootstrapPose(lcmPose);
			return 0;
		}
		
		double dt = (lcmPose.utime - pose.utime) * (1.0 / 1000000.0); // (new usec - old usec) * 1 / usec in sec

		pose.vel[0] = (lcmPose.pos[0] - pose.pos[0]) * (1.0 / dt);	// TODO: may need modification for smoothing
		pose.vel[1] = (lcmPose.pos[1] - pose.pos[1]) * (1.0 / dt);
		
		double newTheta = LinAlg.quatToRollPitchYaw(lcmPose.orientation)[2];
		newTheta = MathUtil.mod2pi(newTheta);
		double oldTheta = LinAlg.quatToRollPitchYaw(pose.orientation)[2];
		oldTheta = MathUtil.mod2pi(oldTheta);
		pose.rotation_rate[2] = MathUtil.mod2pi(newTheta - oldTheta) * (1.0 / dt);
		
		if (logger.isTraceEnabled()) {
			double xvel = LinAlg.rotate2(pose.vel, -newTheta)[0];
			logger.trace(String.format("dt%1.5f vx%1.3f vy%1.3f r%1.3f xv%1.3f", dt, pose.vel[0], pose.vel[1], pose.rotation_rate[2], xvel));
		}
		updatePose(lcmPose);
		
		return dt;
	}

	private void updatePose(pose_t newPose) {
		pose.utime = newPose.utime;
		pose.pos = LinAlg.add(newPose.pos, offset.pos);
		pose.orientation[0] = newPose.orientation[0];
		pose.orientation[3] = newPose.orientation[3];
	}
	
	@Override
	public pose_t getPose() {
		return pose.copy();
	}

	@Override
	public void setOffset(pose_t offset) {
		System.arraycopy(offset.pos, 0, this.offset.pos, 0, offset.pos.length);
	}

	@Override
	public pose_t getOffset() {
		return offset.copy();
	}
}
