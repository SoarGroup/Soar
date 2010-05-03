package edu.umich.soar.sproom.command;

import java.awt.BorderLayout;
import java.awt.Color;
import java.io.IOException;
import java.util.Arrays;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import javax.swing.JFrame;

import april.jmat.LinAlg;
import april.jmat.MathUtil;
import april.jmat.Matrix;

import lcm.lcm.LCM;
import lcm.lcm.LCMDataInputStream;
import lcm.lcm.LCMSubscriber;
import april.lcmtypes.pose_t;
import april.lcmtypes.tag_pose_t;
import april.vis.VisCanvas;
import april.vis.VisChain;
import april.vis.VisRobot;
import april.vis.VisWorld;

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
			tag_pose_t tp = tagpose;
			tagpose = null;
			return tp;
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
		final Pose pose = new Pose();

		JFrame jf = new JFrame(Pose.class.toString());
		VisWorld vw = new VisWorld();
	    VisCanvas vc = new VisCanvas(vw);
    	jf.setLayout(new BorderLayout());
    	jf.add(vc, BorderLayout.CENTER);
    	jf.setSize(1350,800);
    	jf.setVisible(true);
    	
    	final VisWorld.Buffer vbp = vw.getBuffer("pose");
    	final VisWorld.Buffer vbtp = vw.getBuffer("tagpose");

		ScheduledExecutorService schexec = Executors.newSingleThreadScheduledExecutor();
		schexec.scheduleAtFixedRate(new Runnable() {
			@Override
			public void run() {
				pose_t p = pose.getPose();
				double[][] m = LinAlg.quatPosToMatrix(p.orientation, p.pos);
				vbp.addBuffered(new VisChain(m, new VisRobot(Color.blue, Color.black)));
				vbp.switchBuffer();
				
				tag_pose_t tp = pose.getTagPose();
				if (tp != null && tp.ntags > 0) {
					Matrix M = new Matrix(tp.poses[0]);
					double[] rpy = LinAlg.matrixToRollPitchYaw(M);
					System.out.format("%1.6f %1.6f %1.6f\n", rpy[0], rpy[1], rpy[2]);
					System.out.print(M);
					M.set(3,3,1.0);
					vbtp.addBuffered(new VisChain(M, new VisRobot(Color.red, Color.black)));
					vbtp.switchBuffer();
				}
			}
		}, 0, 100, TimeUnit.MILLISECONDS);
	}

}
