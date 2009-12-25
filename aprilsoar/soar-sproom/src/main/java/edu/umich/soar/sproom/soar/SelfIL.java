package edu.umich.soar.sproom.soar;

import lcmtypes.pose_t;
import sml.Agent;
import sml.Identifier;
import edu.umich.soar.FloatWme;
import edu.umich.soar.StringWme;
import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.Pose;

public class SelfIL implements InputLinkElement {
	private static final String NAME = "name";
	private static final String AREA = "area";

	private StringWme area;
	private FloatWme[] xyz = new FloatWme[3];
	private FloatWme[] xyzVelocity = new FloatWme[3];
	private YawWme yaw;
	private FloatWme yawVelocity;
	
	public SelfIL(Identifier root, Adaptable app) {
		Agent agent = (Agent)app.getAdapter(Agent.class);
		
		StringWme.newInstance(root, NAME, agent.GetAgentName());
		area = StringWme.newInstance(root, AREA);
		
		Identifier pose = agent.CreateIdWME(root, SharedNames.POSE);
		xyz[0] = FloatWme.newInstance(pose, SharedNames.X);
		xyz[1] = FloatWme.newInstance(pose, SharedNames.Y);
		xyz[2] = FloatWme.newInstance(pose, SharedNames.Z);
		yaw = YawWme.newInstance(pose, SharedNames.YAW);
		xyzVelocity[0] = FloatWme.newInstance(pose, SharedNames.X_VELOCITY);
		xyzVelocity[1] = FloatWme.newInstance(pose, SharedNames.Y_VELOCITY);
		xyzVelocity[2] = FloatWme.newInstance(pose, SharedNames.Z_VELOCITY);
		yawVelocity = FloatWme.newInstance(pose, SharedNames.YAW_VELOCITY);
		
		update(app);
	}

	@Override
	public void update(Adaptable app) {
		// TODO: area

		Pose poseClass = (Pose)app.getAdapter(Pose.class);
		pose_t pose = poseClass.getPose();
		
		for (int i = 0; i < xyz.length; ++i) {
			xyz[i].update(pose.pos[i]);
			xyzVelocity[i].update(pose.vel[i]);
		}
		yaw.update(poseClass.getYaw());
		yawVelocity.update(pose.rotation_rate[2]);
	}
}
