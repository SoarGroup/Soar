package edu.umich.soar.sproom.command;

import java.io.IOException;
import java.util.Arrays;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import april.jmat.LinAlg;
import april.jmat.MathUtil;

import lcm.lcm.LCM;
import lcm.lcm.LCMDataInputStream;
import lcm.lcm.LCMSubscriber;
import april.lcmtypes.pose_t;
import april.lcmtypes.tag_pose_t;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.HzChecker;
import edu.umich.soar.sproom.SharedNames;

/**
 * Listens to pose data from LCM and elaborates it for use by the Command component.
 * 
 * @author voigtjr@gmail.com
 */
public class Pose
{
	private static final Log logger = LogFactory.getLog(Pose.class);

	private final LCM lcm = LCM.getSingleton();
	private pose_t pose;
	private pose_t elaboratedPose = new pose_t();
	private tag_pose_t tagpose;
	private ScheduledExecutorService schexec = Executors.newSingleThreadScheduledExecutor();
	private final HzChecker checker = HzChecker.newInstance(Pose.class.toString());
	
	Pose()
	{
		lcm.subscribe(SharedNames.POSE_CHANNEL, subscriber);
		lcm.subscribe(SharedNames.TAG_POSE_CHANNEL, subscriber);

		schexec.scheduleAtFixedRate(update, 0, 50, TimeUnit.MILLISECONDS);
	}
	
	private Runnable update = new Runnable() 
	{
		public void run() 
		{
			checker.tick();
			
			pose_t tp = pose; // grab a reference once
			if (tp != null && tp.utime > elaboratedPose.utime)
			{
				// elaborate to get velocities from odometry
				double dt = (tp.utime - elaboratedPose.utime) * (1.0 / 1000000.0); // (new usec - old usec) * 1 / usec in sec

				tp.vel[0] = (tp.pos[0] - elaboratedPose.pos[0]) * (1.0 / dt); // TODO: may need modification for smoothing
				tp.vel[1] = (tp.pos[1] - elaboratedPose.pos[1]) * (1.0 / dt);

				double newTheta = LinAlg.quatToRollPitchYaw(tp.orientation)[2];
				newTheta = MathUtil.mod2pi(newTheta);
				double oldTheta = LinAlg.quatToRollPitchYaw(elaboratedPose.orientation)[2];
				oldTheta = MathUtil.mod2pi(oldTheta);
				tp.rotation_rate[2] = MathUtil.mod2pi(newTheta - oldTheta) * (1.0 / dt);

				if (logger.isTraceEnabled())
				{
					double xvel = LinAlg.rotate2(tp.vel, -newTheta)[0];
					logger.trace(String.format("dt%1.5f vx%1.3f vy%1.3f r%1.3f xv%1.3f", 
							dt, tp.vel[0], tp.vel[1], tp.rotation_rate[2], xvel));
				}
				
				elaboratedPose = tp;
			}

		}
	};

	private LCMSubscriber subscriber = new LCMSubscriber()
	{
		public void messageReceived(LCM lcm, String channel, LCMDataInputStream ins)
		{
			if (channel.equals(SharedNames.POSE_CHANNEL))
			{
				try
				{
					pose = new pose_t(ins);
				} catch (IOException e)
				{
					logger.error("Error decoding pose_t message: " + e.getMessage());
				}
			} else if (channel.equals(SharedNames.TAG_POSE_CHANNEL))
			{
				try
				{
					tagpose = new tag_pose_t(ins);
				} catch (IOException e)
				{
					logger.error("Error decoding tag_pose_t message: " + e.getMessage());
				}
			} else
			{
				logger.warn("Received unknown message on channel: " + channel);
			}
		}
	};

	public pose_t getPose()
	{
		return elaboratedPose.copy();
	}
	
	private tag_pose_t getTagPose()
	{
		if (tagpose != null) {
			return tagpose.copy();
		}
		return null;
	}
	
	public static RelativePointData getRelativePointData(pose_t pose, double[] pos)
	{
		double distance = LinAlg.distance(pose.pos, pos);

		double[] delta = LinAlg.subtract(pos, pose.pos);
		double yaw = Math.atan2(delta[1], delta[0]);

		double relativeYaw = yaw - LinAlg.quatToRollPitchYaw(pose.orientation)[2];
		relativeYaw = MathUtil.mod2pi(relativeYaw);

		return new RelativePointData(distance, yaw, relativeYaw);
	}

	public static double getYaw(pose_t pose)
	{
		return LinAlg.quatToRollPitchYaw(pose.orientation)[2];
	}
	
	public static void main(String[] args) {
		Pose pose = new Pose();
		try {
			while (true) 
			{
				pose_t p = pose.getPose();
				double pYaw = p != null ? Pose.getYaw(p) : 0;
				tag_pose_t tp = pose.getTagPose();
				double tpYaw = tp != null && tp.ntags > 0 ? LinAlg.matrixToXyzrpy(tp.poses[0])[5] : 0;
				String message = String.format("p: %3.2f, tp: %3.2f", Math.toDegrees(pYaw), Math.toDegrees(tpYaw));
				System.out.println(message);
				Thread.sleep(500);
			}
		} catch (InterruptedException e) {
			
		}
	}

}
