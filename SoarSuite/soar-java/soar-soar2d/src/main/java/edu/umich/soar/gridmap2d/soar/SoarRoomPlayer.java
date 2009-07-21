package edu.umich.soar.gridmap2d.soar;

import java.io.File;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.RoomCommander;
import edu.umich.soar.gridmap2d.players.RoomPlayer;
import edu.umich.soar.gridmap2d.world.RoomWorld;

import sml.Agent;
import sml.Identifier;

public class SoarRoomPlayer implements RoomCommander {
	private static Logger logger = Logger.getLogger(SoarRoomPlayer.class);

	private RoomPlayer player;
	private Agent agent;
	private String [] shutdownCommands;
	private InputLinkMetadata metadata;
	private File commonMetadataFile;
	private File mapMetadataFile;
	private SoarRoomIL roomil;
	
	Identifier moveCommandId = null;
	boolean moveCommandExecutingAdded = false;
	Identifier rotateCommandId = null;
	boolean rotateCommandExecutingAdded = false;
	Identifier getCommandId = null;
	Identifier dropCommandId = null;
	RoomWorld world;

	public SoarRoomPlayer(RoomPlayer player, Agent agent,
			RoomWorld world, String[] shutdownCommands, File commonMetadataFile,
			File mapMetadataFile) throws Exception {
		this.player = player;
		this.agent = agent;
		this.world = world;
		this.commonMetadataFile = commonMetadataFile;
		this.mapMetadataFile = mapMetadataFile;
		this.shutdownCommands = shutdownCommands;
		
		agent.SetBlinkIfNoChange(false);
		
		roomil = new SoarRoomIL(agent);
		roomil.create();
		
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
		
		moveCommandId = null;
		moveCommandExecutingAdded = false;
		rotateCommandId = null;
		rotateCommandExecutingAdded = false;
		getCommandId = null;
		dropCommandId = null;
		
		this.roomil.destroy();
		
		metadata.destroy();
		metadata = null;
		metadata = InputLinkMetadata.load(agent, commonMetadataFile, mapMetadataFile);
		
		agent.InitSoar();
		
		this.roomil.create();
	}

	@Override
	public void update(RoomMap roomMap) throws Exception {
		roomil.update(player, world, roomMap);

		// Add status executing to any commands that need it
		if (moveCommandId != null) {
			if (!moveCommandExecutingAdded) {
				agent.CreateStringWME(moveCommandId, "status", "executing");
				moveCommandExecutingAdded = true;
			}
		}
		if (rotateCommandId != null) {
			if (!rotateCommandExecutingAdded) {
				agent.CreateStringWME(rotateCommandId, "status", "executing");
				rotateCommandExecutingAdded = true;
			}
		}
		assert getCommandId == null;
		assert dropCommandId == null;
		
		agent.Commit();
	}

	@Override
	public CommandInfo nextCommand() throws Exception {
		// go through the commands
		CommandInfo commandInfo = new CommandInfo();
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();
			
			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				if (moveCommandId != null) {
					if (moveCommandId.GetTimeTag() != commandId.GetTimeTag()) {
						moveCommandId.AddStatusComplete();
						moveCommandId = null;
					}
				}
				if (commandInfo.move) {
					logger.warn(player.getName() + " multiple move commands issued");
					commandId.AddStatusError();
					continue;
				}
				
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				
				if (direction == null) {
					logger.warn(player.getName() + " move command missing direction parameter");
					commandId.AddStatusError();
					continue;
				}
				
				if (direction.equalsIgnoreCase(Names.kForwardID)) {
					commandInfo.forward = true;
					
				} else if (direction.equalsIgnoreCase(Names.kBackwardID)) {
					commandInfo.backward = true;
				
				} else if (direction.equalsIgnoreCase(Names.kStopID)) {
					commandInfo.move = true;
					commandInfo.forward = true;
					commandInfo.backward = true;
					commandId.AddStatusComplete();
					continue;
					
				} else {
					logger.warn(player.getName() + "unrecognized move direction: " + direction);
					commandId.AddStatusError();
					continue;
				}

				commandInfo.move = true;
				moveCommandId = commandId;
				moveCommandExecutingAdded = false;

			} else if (commandName.equalsIgnoreCase(Names.kRotateID)) {
				if (rotateCommandId != null) {
					if (rotateCommandId.GetTimeTag() != commandId.GetTimeTag()) {
						rotateCommandId.AddStatusComplete();
						rotateCommandId = null;
					}
				}
				if (commandInfo.rotate) {
					logger.warn(player.getName() + " multiple rotate commands issued");
					commandId.AddStatusError();
					continue;
				}
				
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				
				if (direction == null) {
					logger.warn(player.getName() + " rotate command missing direction parameter");
					commandId.AddStatusError();
					continue;
				}
				
				if (direction.equalsIgnoreCase(Names.kLeftID)) {
					commandInfo.rotateDirection = Names.kRotateLeft;
					
				} else if (direction.equalsIgnoreCase(Names.kRightID)) {
					commandInfo.rotateDirection = Names.kRotateRight;
				
				} else if (direction.equalsIgnoreCase(Names.kStopID)) {
					commandInfo.rotate = true;
					commandInfo.rotateDirection = Names.kRotateStop;
					commandId.AddStatusComplete();
					continue;
				
				} else {
					logger.warn(player.getName() + "unrecognized rotate direction: " + direction);
					commandId.AddStatusError();
					continue;
					
				}

				commandInfo.rotate = true;
				rotateCommandId = commandId;
				rotateCommandExecutingAdded = false;

			} else if (commandName.equalsIgnoreCase(Names.kStopSimID)) {
				if (commandInfo.stopSim) {
					logger.warn(player.getName() + " multiple stop-sim commands issued");
					commandId.AddStatusError();
					continue;
				}
				commandInfo.stopSim = true;
				commandId.AddStatusComplete();
				
			} else if (commandName.equalsIgnoreCase(Names.kRotateAbsoluteID)) {
				if (rotateCommandId != null) {
					if (rotateCommandId.GetTimeTag() != commandId.GetTimeTag()) {
						rotateCommandId.AddStatusComplete();
						rotateCommandId = null;
					}
				}
				if (commandInfo.rotateAbsolute) {
					logger.warn(player.getName() + " multiple rotate-absolute commands issued");
					commandId.AddStatusError();
					continue;
				}
				
				String yawString = commandId.GetParameterValue("yaw");
				if (yawString == null) {
					logger.warn(player.getName() + " rotate-absolute command missing yaw parameter");
					commandId.AddStatusError();
					continue;
				}
				
				try {
					commandInfo.rotateAbsoluteHeading = Double.parseDouble(yawString);
				} catch (NumberFormatException e) {
					logger.warn(player.getName() + " rotate-absolute yaw parameter improperly formatted");
					commandId.AddStatusError();
					continue;
				}
				
				commandInfo.rotateAbsolute = true;
				rotateCommandId = commandId;
				rotateCommandExecutingAdded = false;
				
			} else if (commandName.equalsIgnoreCase(Names.kRotateRelativeID)) {
				if (rotateCommandId != null) {
					if (rotateCommandId.GetTimeTag() != commandId.GetTimeTag()) {
						rotateCommandId.AddStatusComplete();
						rotateCommandId = null;
					}
				}
				if (commandInfo.rotateRelative) {
					logger.warn(player.getName() + " multiple rotate-relative commands issued");
					commandId.AddStatusError();
					continue;
				}
				
				String amountString = commandId.GetParameterValue("yaw");
				if (amountString == null) {
					logger.warn(player.getName() + " rotate-relative command missing yaw parameter");
					commandId.AddStatusError();
					continue;
				}
				
				try {
					commandInfo.rotateRelativeYaw = Double.parseDouble(amountString);
				} catch (NumberFormatException e) {
					logger.warn(player.getName() + " rotate-relative yaw parameter improperly formatted");
					commandId.AddStatusError();
					continue;
				}

				commandInfo.rotateRelative = true;
				rotateCommandId = commandId;
				rotateCommandExecutingAdded = false;
				
			} else if (commandName.equalsIgnoreCase(Names.kGetID)) {
				if (commandInfo.get) {
					logger.warn(player.getName() + " multiple get commands issued");
					commandId.AddStatusError();
					continue;
				}

				if (commandInfo.drop) {
					logger.warn(player.getName() + " get: both get and drop simultaneously issued");
					commandId.AddStatusError();
					continue;
				}
				
				String idString = commandId.GetParameterValue("id");
				if (idString == null) {
					logger.warn(player.getName() + " get command missing id parameter");
					commandId.AddStatusError();
					continue;
				}
				try {
					commandInfo.getId = Integer.parseInt(idString);
				} catch (NumberFormatException e) {
					logger.warn(player.getName() + " get command id parameter improperly formatted");
					commandId.AddStatusError();
					continue;
				}
				
				SoarRoomObjectIL oIL = roomil.getOIL(commandInfo.getId);
				if (oIL == null) {
					logger.warn(player.getName() + " get command invalid id " + commandInfo.getId);
					commandId.AddStatusError();
					continue;
				}
				if (oIL.range.GetValue() > RoomWorld.cell_size) {
					logger.warn(player.getName() + " get command object out of range");
					commandId.AddStatusError();
					continue;
				}
				
				commandInfo.get = true;
				getCommandId = commandId;
				commandInfo.getLocation = new int [] { oIL.col.GetValue(), oIL.row.GetValue() };
				
			} else if (commandName.equalsIgnoreCase(Names.kDropID)) {
				if (commandInfo.drop) {
					logger.warn(player.getName() + " multiple drop commands issued");
					commandId.AddStatusError();
					continue;
				}

				if (commandInfo.get) {
					logger.warn(player.getName() + " drop: both drop and get simultaneously issued");
					commandId.AddStatusError();
					continue;
				}
				
				String idString = commandId.GetParameterValue("id");
				if (idString == null) {
					logger.warn(player.getName() + " drop command missing id parameter");
					commandId.AddStatusError();
					continue;
				}
				try {
					commandInfo.dropId = Integer.parseInt(idString);
				} catch (NumberFormatException e) {
					logger.warn(player.getName() + " drop command id parameter improperly formatted");
					commandId.AddStatusError();
					continue;
				}
				
				commandInfo.drop = true;
				dropCommandId = commandId;
				
			} else if (commandName.equalsIgnoreCase("communicate")) {
				CommandInfo.Communication comm = commandInfo.new Communication();
				String toString = commandId.GetParameterValue("to");
				if (toString == null) {
					logger.warn(player.getName() + " communicate command missing to parameter");
					commandId.AddStatusError();
					continue;
				}
				comm.to = toString;
				
				String messageString = commandId.GetParameterValue("message");
				if (messageString == null) {
					logger.warn(player.getName() + " communicate command missing message parameter");
					commandId.AddStatusError();
					continue;
				}
				comm.message = messageString;
				
				commandInfo.messages.add(comm);
				commandId.AddStatusComplete();
				
			} else {
				logger.warn("Unknown command: " + commandName);
				commandId.AddStatusError();
				continue;
			}
		}
		agent.ClearOutputLinkChanges();
		if (!agent.Commit()) {
			error(Names.Errors.commitFail + player.getName());
			Gridmap2D.control.stopSimulation();
		}
		return commandInfo;
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
	public void receiveMessage(Player player, String message) {
		roomil.addMessage(player, message);
		
	}

	@Override
	public void rotateComplete() {
		if (rotateCommandId != null) {
			rotateCommandId.AddStatusComplete();
			rotateCommandId = null;
		}
	}

	@Override
	public void updateDropStatus(boolean success) {
		if (success) {
			dropCommandId.AddStatusComplete();
			dropCommandId = null;
			return;
		}
		dropCommandId.AddStatusError();
		dropCommandId = null;
	}

	@Override
	public void updateGetStatus(boolean success) {
		if (success) {
			getCommandId.AddStatusComplete();
			getCommandId = null;
			return;
		}
		getCommandId.AddStatusError();
		getCommandId = null;
	}

	@Override
	public void carry(CellObject object) {
		roomil.carry(object);
	}

	@Override
	public void drop() {
		roomil.drop();
	}

}
