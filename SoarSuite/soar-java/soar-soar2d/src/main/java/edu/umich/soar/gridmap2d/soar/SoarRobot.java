package edu.umich.soar.gridmap2d.soar;

import java.io.File;
import java.util.Arrays;
import java.util.List;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.RoomCommander;
import edu.umich.soar.gridmap2d.players.RoomPlayer;
import edu.umich.soar.gridmap2d.world.RoomWorld;
import edu.umich.soar.robot.OutputLinkManager;
import edu.umich.soar.robot.ConfigureInterface;
import edu.umich.soar.robot.OffsetPose;
import edu.umich.soar.robot.DifferentialDriveCommand;
import edu.umich.soar.robot.SendMessagesInterface;
import lcmtypes.pose_t;

import sml.Agent;
import sml.Kernel;

public class SoarRobot implements RoomCommander, ConfigureInterface, OffsetPose, SendMessagesInterface  {
	private static Logger logger = Logger.getLogger(SoarRobot.class);

	private RoomPlayer player;
	private Agent agent;
	private String [] shutdownCommands;
	RoomWorld world;
	
	private InputLinkMetadata metadata;
	private File commonMetadataFile;
	private File mapMetadataFile;

	final SoarRobotInputLinkManager input;
	final OutputLinkManager output;
	
	DifferentialDriveCommand ddc;
	double[] offset = {0, 0};
	boolean floatYawWmes = true;
	
	public SoarRobot(RoomPlayer player, Agent agent, Kernel kernel,
			RoomWorld world, String[] shutdownCommands, File commonMetadataFile,
			File mapMetadataFile) throws Exception {
		this.player = player;
		this.agent = agent;
		this.world = world;
		this.shutdownCommands = shutdownCommands;
		
		this.commonMetadataFile = commonMetadataFile;
		this.mapMetadataFile = mapMetadataFile;
		
		agent.SetBlinkIfNoChange(false);
		
		input = new SoarRobotInputLinkManager(agent, kernel, this);
		input.create();
		output = new OutputLinkManager(agent);
		output.create(input.getWaypointInterface(), this, input.getReceiveMessagesInterface(), this, this);
		
		metadata = InputLinkMetadata.load(agent, commonMetadataFile, mapMetadataFile);
		
		if (!agent.Commit()) {
			throw new Exception(Names.Errors.commitFail + player.getName());
		}
	}

	@Override
	public void reset() throws Exception {
		if (agent == null) {
			return;
		}
		
		metadata.destroy();
		metadata = null;
		metadata = InputLinkMetadata.load(agent, commonMetadataFile, mapMetadataFile);

		input.destroy();
		output.destroy();

		agent.InitSoar();

		input.create();
		output.create(input.getWaypointInterface(), this, input.getReceiveMessagesInterface(), this, this);

		agent.Commit();
	}

	@Override
	public void update(RoomMap roomMap) throws Exception {

		ddc = output.update();
		input.update(player, world, roomMap, this.isFloatYawWmes());	
		agent.Commit();
	}

	@Override
	public CommandInfo nextCommand() throws Exception {
		return new CommandInfo(ddc);
	}

	private void error(String message) {
		logger.error(message);
		Gridmap2D.control.errorPopUp(message);
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
	public void receiveMessage(Player player, List<String> message) {
		input.getReceiveMessagesInterface().newMessage(player.getName(), this.player.getName(), message);
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
	public double[] getOffset() {
		return Arrays.copyOf(offset, offset.length);
	}

	@Override
	public pose_t getPose() {
		return player.getState().getPose();
	}

	@Override
	public void setOffset(double[] offset) {
		this.offset = Arrays.copyOf(offset, offset.length);
	}

	@Override
	public void sendMessage(String from, String to, List<String> tokens) {
		logger.warn("sendMessage: Not implemented yet.");
	}
}
