package edu.umich.soar.room.soar;

import java.util.List;

import jmat.LinAlg;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sps.control.robot.ObjectManipulationInterface;
import edu.umich.soar.sps.control.robot.OutputLinkManager;
import edu.umich.soar.sps.control.robot.ConfigureInterface;
import edu.umich.soar.sps.control.robot.OffsetPose;
import edu.umich.soar.sps.control.robot.DifferentialDriveCommand;
import edu.umich.soar.sps.control.robot.ReceiveMessagesInterface;
import edu.umich.soar.room.core.Simulation;
import edu.umich.soar.room.map.Robot;
import edu.umich.soar.room.map.RobotCommand;
import edu.umich.soar.room.map.RobotCommander;
import edu.umich.soar.room.map.RoomMap;
import edu.umich.soar.room.map.RoomWorld;
import lcmtypes.pose_t;

import sml.Agent;
import sml.Kernel;

public class SoarRobot implements RobotCommander, ConfigureInterface, OffsetPose, ObjectManipulationInterface, SoarAgent  {
	private static Log logger = LogFactory.getLog(SoarRobot.class);

	private Robot player;
	private Agent agent;
	private String [] shutdownCommands;
	RoomWorld world;
	
	final SoarRobotInputLinkManager input;
	final OutputLinkManager output;
	
	DifferentialDriveCommand ddc;
	pose_t offset = new pose_t();
	boolean floatYawWmes = true;
	
	public static final double PIXELS_2_METERS = 1.0 / 16.0;
	public static final double METERS_2_PIXELS = 16.0;
	
	private final Simulation sim;
	
	private RobotCommand command;
	
	public SoarRobot(Simulation sim, Robot player, Agent agent, Kernel kernel,
			RoomWorld world, String[] shutdownCommands) {
		this.player = player;
		this.sim = sim;
		this.agent = agent;
		this.world = world;
		this.shutdownCommands = shutdownCommands;
		
		agent.SetBlinkIfNoChange(false);
		
		input = new SoarRobotInputLinkManager(sim, agent, kernel, this, player.getState());
		input.create();
		input.update(player, world, (RoomMap)sim.getMap(), this.isFloatYawWmes());	
		output = new OutputLinkManager(agent);
		output.create(input.getWaypointInterface(), world, input.getReceiveMessagesInterface(), this, this, this);
	}

	@Override
	public void reset() {
		if (agent == null) {
			return;
		}
		command = null;
		input.destroy();
		output.destroy();

		agent.InitSoar();

		input.create();
		output.create(input.getWaypointInterface(), world, input.getReceiveMessagesInterface(), this, this, this);
	}

	@Override
	public RobotCommand nextCommand() {
		if (command == null) {
			return RobotCommand.NULL;
		}
		
		// TODO: this is useful for non-sim environments, it is more fail-safe.
		// Having it here, however, makes the sync -> async changeover lose commands.
//		RobotCommand temp = command;
//		command = null;
//		return temp;
		return command;
	}
	
	private void error(String message) {
		sim.error("Soar Robot", message);
	}
	
	@Override
	public void shutdown() {
		assert agent != null;
		if (shutdownCommands != null) { 
			// execute the pre-shutdown commands
			for (String command : shutdownCommands) {
				String result = player.getName() + ": result: " + agent.ExecuteCommandLine(command, true);
				logger.info(player.getName() + ": shutdown command: " + command);
				if (agent.HadError()) {
					error(result);
				} else {
					logger.info(player.getName() + ": result: " + result);
				}
			}
		}
	}

	@Override
	public boolean isFloatYawWmes() {
		return floatYawWmes;
	}

	@Override
	public void setFloatYawWmes(boolean setting) {
		floatYawWmes = setting;
	}

	@Override
	public pose_t getOffset() {
		pose_t copy = offset.copy();
		LinAlg.scaleEquals(copy.pos, PIXELS_2_METERS);
		LinAlg.scaleEquals(copy.vel, PIXELS_2_METERS);
		return copy;
	}

	@Override
	public pose_t getPose() {
		pose_t copy = player.getState().getPose().copy();
		LinAlg.scaleEquals(copy.pos, PIXELS_2_METERS);
		LinAlg.scaleEquals(copy.vel, PIXELS_2_METERS);
		return copy;
	}

	@Override
	public void setOffset(pose_t offset) {
		pose_t copy = offset.copy();
		LinAlg.scaleEquals(copy.pos, METERS_2_PIXELS);
		LinAlg.scaleEquals(copy.vel, METERS_2_PIXELS);
		this.offset = copy;
	}

	@Override
	public boolean drop(int id) {
		return world.dropObject(player, id);
	}

	@Override
	public boolean get(int id) {
		return world.getObject(player, id);
	}

	@Override
	public boolean diffuse(int id) {
		return world.diffuseObject(player, id);
	}

	@Override
	public boolean diffuseByWire(int id, String color) {
		return world.diffuseObjectByWire(player, id, color);
	}
	
	@Override
	public String reason() {
		return world.reason();
	}

	@Override
	public List<double[]> getWaypointList() {
		List<double[]> waypoints = input.getWaypointInterface().getWaypointList();
		for (double[] wp : waypoints) {
			LinAlg.scaleEquals(wp, METERS_2_PIXELS);
		}
		return waypoints;
	}

	@Override
	public ReceiveMessagesInterface getReceiveMessagesInterface() {
		return input.getReceiveMessagesInterface();
	}

	@Override
	public void processSoarOuput() {
		ddc = output.update();
		command = new RobotCommand.Builder(ddc).build();
	}

	@Override
	public void updateSoarInput() {
		input.update(player, world, (RoomMap)sim.getMap(), this.isFloatYawWmes());	
	}

}
