package edu.umich.soar.gridmap2d.world;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

import jmat.LinAlg;

import lcmtypes.pose_t;

import org.apache.log4j.Logger;

import com.commsen.stopwatch.Stopwatch;

import edu.umich.soar.gridmap2d.CognitiveArchitecture;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.config.PlayerConfig;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.GridMap;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.map.RoomObject;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.RobotCommander;
import edu.umich.soar.gridmap2d.players.Robot;
import edu.umich.soar.gridmap2d.players.RobotState;
import edu.umich.soar.robot.DifferentialDriveCommand;
import edu.umich.soar.robot.SendMessagesInterface;

public class RoomWorld implements World, SendMessagesInterface {
	private static Logger logger = Logger.getLogger(RoomWorld.class);

	private RoomMap map;
	private PlayersManager<Robot> players = new PlayersManager<Robot>();
	private boolean forceHuman = false;
	private List<String> stopMessages = new ArrayList<String>();
	private final double LIN_SPEED = 16;
	public static final int CELL_SIZE = 16;
	private double ANG_SPEED = Math.PI / 4.0;
	private CognitiveArchitecture cogArch;
	private String blockManipulationReason;
	private Queue<Message> messages = new LinkedList<Message>();
	
	public RoomWorld(CognitiveArchitecture cogArch) {
		this.cogArch = cogArch;
	}

	@Override
	public boolean hasPlayer(String name) {
		return players.get(name) != null;
	}
	
	@Override
	public boolean addPlayer(String id, PlayerConfig cfg, boolean debug) {
		int [] location = WorldUtil.getStartingLocation(map, cfg.pos);
		if (location == null) {
			Gridmap2D.control.errorPopUp("There are no suitable starting locations.");
			return false;
		}

		Robot player = new Robot(id);
		players.add(player, cfg.pos);
		
		if (cfg.productions != null) {
			RobotCommander cmdr = cogArch.createRoomCommander(player, this, cfg.productions, cfg.shutdown_commands, debug);
			if (cmdr == null) {
				players.remove(player);
				return false;
			}
			player.setCommander(cmdr);
		} else if (cfg.script != null) {
			Gridmap2D.control.infoPopUp("Scripted robots not implemented.");
		}

		players.setLocation(player, location);
		
		// put the player in it
		map.getCell(location).addPlayer(player);
		
		player.getState().setLocationId(map.getLocationId(location));
		double [] floatLocation = defaultFloatLocation(location);
		player.getState().setPos(floatLocation);

		logger.info(player.getName() + ": Spawning at (" + location[0] + "," + location[1] + "), (" + floatLocation[0] + "," + floatLocation[1] + ")");
		
		updatePlayers();
		return true;
	}

	@Override
	public GridMap getMap() {
		return map;
	}

	@Override
	public Player[] getPlayers() {
		return players.getAllAsPlayers();
	}

	@Override
	public void interrupted(String agentName) {
		players.interrupted(agentName);
		stopMessages.add("interrupted");
	}

	@Override
	public boolean isTerminal() {
		return stopMessages.size() > 0;
	}

	@Override
	public int numberOfPlayers() {
		return players.numberOfPlayers();
	}

	@Override
	public void removePlayer(String name) {
		Robot player = players.get(name);
		map.getCell(players.getLocation(player)).removePlayer(player);
		players.remove(player);
		player.shutdownCommander();
		updatePlayers();
	}

	@Override
	public void reset() {
		blockManipulationReason = null;
		messages.clear();
		map.reset();
		resetState();
	}

	@Override
	public void setForceHumanInput(boolean setting) {
		forceHuman = setting;
	}

	@Override
	public void setAndResetMap(String mapPath) {
		map = new RoomMap(mapPath);
		resetState();
	}

	private void resetState() {
		stopMessages.clear();
		resetPlayers();
	}

	/**
	 * @throws IllegalStateException If there are no available locations to spawn the players
	 */
	private void resetPlayers() {
		if (players.numberOfPlayers() == 0) {
			return;
		}
		
		for (Robot player : players.getAll()) {
			player.reset();
			
			// find a suitable starting location
			int [] location = WorldUtil.getStartingLocation(map, players.getInitialLocation(player));
			if (location == null) {
				throw new IllegalStateException("no empty locations available for spawn");
			}
			players.setLocation(player, location);

			// put the player in it
			map.getCell(location).addPlayer(player);

			player.getState().setLocationId(map.getLocationId(location));

			double [] floatLocation = defaultFloatLocation(location);
			player.getState().setPos(floatLocation);

			logger.info(player.getName() + ": Spawning at (" + location[0] + "," + location[1] + "), (" + floatLocation[0] + "," + floatLocation[1] + ")");
		}
		
		updatePlayers();
	}
	
	private double [] defaultFloatLocation(int [] location) {
		double [] floatLocation = new double [2];
		final int cellSize = CELL_SIZE;
		
		// default to center of square
		floatLocation[0] = (location[0] * cellSize) + (cellSize / 2); 
		floatLocation[1] = (location[1] * cellSize) + (cellSize / 2); 

		return floatLocation;
	}
	
	@Override
	public void update(int worldCount) {
		WorldUtil.checkNumPlayers(players.numberOfPlayers());

		long id1 = Stopwatch.start("update", "input");
		// Collect input
		for (Robot player : players.getAll()) {
			player.resetPointsChanged();

			CommandInfo command = forceHuman ? Gridmap2D.control.getHumanCommand(player) : player.getCommand();
			if (command == null) {
				Gridmap2D.control.stopSimulation();
				return;
			}
			players.setCommand(player, command);
			WorldUtil.checkStopSim(stopMessages, command, player);
		}
		Stopwatch.stop(id1);
		
		long id2 = Stopwatch.start("update", "move");
		moveRoomPlayers(Gridmap2D.control.getTimeSlice());
		
		for (Message message : messages) {
			message.recipient.getReceiveMessagesInterface().newMessage(message.from, message.tokens);
		}
		messages.clear();
		Stopwatch.stop(id2);
		
		long id3 = Stopwatch.start("update", "update");
		updatePlayers();
		Stopwatch.stop(id3);
	}

	private void moveRoomPlayers(double time) {
		for (Robot player : players.getAll()) {
			CommandInfo command = players.getCommand(player);	
			RobotState state = player.getState();
			
			DifferentialDriveCommand ddc = command.ddc;
			if (ddc != null) {
				switch(ddc.getType()) {
				case ANGVEL:
					state.setAngularVelocity(ddc.getAngularVelocity());
					state.resetDestYaw();
					break;
				case ESTOP:
					state.stop();
					break;
				case HEADING:
					player.getState().setDestYaw(ddc.getHeading(), ANG_SPEED);
					break;
				case HEADING_LINVEL:
					player.getState().setDestYaw(ddc.getHeading(), ANG_SPEED);
					state.setLinearVelocity(ddc.getLinearVelocity() * LIN_SPEED);
					break;
				case LINVEL:
					state.setLinearVelocity(ddc.getLinearVelocity() * LIN_SPEED);
					break;
				case MOTOR:
					// TODO: other than stop
					state.stop();
					break;
				case MOVE_TO:
					// TODO: implement
					assert false;
					break;
				case VEL:
					state.setAngularVelocity(ddc.getAngularVelocity());
					state.setLinearVelocity(ddc.getLinearVelocity() * LIN_SPEED);
					state.resetDestYaw();
					break;
				}
			}
						
			// reset collision sensor
			state.setCollisionX(false);
			state.setCollisionY(false);

			roomMovePlayer(player, time);
		}
	}

	private void updatePlayers() {
		for (Robot player : players.getAll()) {
			player.update(players.getLocation(player), map);
		}
	}

	private boolean checkBlocked(int [] location) {
		if (map.getCell(location).hasObjectWithProperty(Names.kPropertyBlock)) {
			return true;
		}
		return false;
	}
	
	private void roomMovePlayer(Robot player, double time) {
		int [] oldLocation = players.getLocation(player);
		int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);

		RobotState state = player.getState();
		state.update(time);
		pose_t pose = state.getPose();
		
		newLocation[0] = (int)pose.pos[0] / CELL_SIZE;
		newLocation[1] = (int)pose.pos[1] / CELL_SIZE;

		if (Arrays.equals(oldLocation, newLocation)) {
			map.getCell(oldLocation).setModified(true);
		} else {
			while (checkBlocked(newLocation)) {
				// 1) determine what edge we're intersecting
				if ((newLocation[0] != oldLocation[0]) && (newLocation[1] != oldLocation[1])) {
					// corner case
					int [] oldx = new int [] { oldLocation[0], newLocation[1] };
					
					// if oldx is blocked
					if (checkBlocked(oldx)) {
						state.setCollisionY(true);
						// calculate y first
						if (newLocation[1] > oldLocation[1]) {
							// south
							pose.pos[1] = oldLocation[1] * CELL_SIZE;
							pose.pos[1] += CELL_SIZE - 0.1;
							newLocation[1] = oldLocation[1];
						} 
						else if (newLocation[1] < oldLocation[1]) {
							// north
							pose.pos[1] = oldLocation[1] * CELL_SIZE;
							newLocation[1] = oldLocation[1];
						} else {
							assert false;
						}
					} 
					else {
						state.setCollisionX(true);
						// calculate x first
						if (newLocation[0] > oldLocation[0]) {
							// east
							pose.pos[0] = oldLocation[0] * CELL_SIZE;
							pose.pos[0] += CELL_SIZE - 0.1;
							newLocation[0] = oldLocation[0];
						} 
						else if (newLocation[0] < oldLocation[0]) {
							// west
							pose.pos[0] = oldLocation[0] * CELL_SIZE;
							newLocation[0] = oldLocation[0];
						} else {
							assert false;
						} 
					}
					continue;
				}
				
				if (newLocation[0] > oldLocation[0]) {
					state.setCollisionX(true);
					// east
					pose.pos[0] = oldLocation[0] * CELL_SIZE;
					pose.pos[0] += CELL_SIZE - 0.1;
					newLocation[0] = oldLocation[0];
				} 
				else if (newLocation[0] < oldLocation[0]) {
					state.setCollisionX(true);
					// west
					pose.pos[0] = oldLocation[0] * CELL_SIZE;
					newLocation[0] = oldLocation[0];
				} 
				else if (newLocation[1] > oldLocation[1]) {
					state.setCollisionY(true);
					// south
					pose.pos[1] = oldLocation[1] * CELL_SIZE;
					pose.pos[1] += CELL_SIZE - 0.1;
					newLocation[1] = oldLocation[1];
				} 
				else if (newLocation[1] < oldLocation[1]) {
					state.setCollisionY(true);
					// north
					pose.pos[1] = oldLocation[1] * CELL_SIZE;
					newLocation[1] = oldLocation[1];
				}
			}
			
			state.setPos(pose.pos);
			
			map.getCell(oldLocation).removePlayer(player);
			players.setLocation(player, newLocation);
			state.setLocationId(map.getLocationId(newLocation));
			map.getCell(newLocation).addPlayer(player);
		}
	}

	public boolean dropObject(Robot player, int id) {
		blockManipulationReason = null;
		RobotState state = player.getState();

		if (!state.hasObject()) {
			blockManipulationReason = "Not carrying an object";
			return false;
		}
		
		RoomObject object = state.getRoomObject();
		state.drop();
		object.setPose(state.getPose());
		map.getCell(player.getLocation()).addObject(object.getCellObject());
		return true;
	}

	/**
	 * @param player
	 * @param id
	 * @return
	 * @throws IllegalStateException If tried to remove the object from the cell it was supposed to be in but it wasn't there 
	 */
	public boolean getObject(Robot player, int id) {
		blockManipulationReason = null;
		
		if (player.getState().hasObject()) {
			blockManipulationReason = "Already carrying an object";
			return false;
		}
		
		// note: This is a stupid way to do this.
		for (RoomObject rObj : map.getRoomObjects()) {
			if (rObj.getPose() == null) {
				// already carried by someone
				continue;
			}
			CellObject cObj = rObj.getCellObject();
			
			if (rObj.getId() == id) {
				double distance = LinAlg.distance(player.getState().getPose().pos, rObj.getPose().pos);
				if (Double.compare(distance, CELL_SIZE) <= 0) {
					if (!cObj.getCell().removeObject(cObj)) {
						throw new IllegalStateException("Remove object failed for object that should be there.");
					}
					rObj.setPose(null);
					player.getState().pickUp(rObj);
					return true;
				}
				blockManipulationReason = "Object is too far";
				return false;
			}
		}
		blockManipulationReason = "No such object ID";
		return false;
	}

	public String reason() {
		return blockManipulationReason;
	}

	public List<double[]> getWaypointList(Robot player) {
		return player.getWaypointList();
	}

	private static class Message {
		String from;
		Robot recipient;
		List<String> tokens;
	}
	
	@Override
	public void sendMessage(String from, String to, List<String> tokens) {
		if (to != null) {
			Robot recipient = players.get(to);
			if (recipient == null) {
				Robot sender = players.get(from);
				if (sender == null) {
					StringBuilder sb = new StringBuilder();
					sb.append("Unknown sender ").append(from).append(" for message: ");
					for (String token : tokens) {
						sb.append(token);
						sb.append(" ");
					}
					logger.error(sb);
					return;
				}
				StringBuilder sb = new StringBuilder();
				sb.append("Unknown recipient ").append(to).append(" for message: ");
				for (String token : tokens) {
					sb.append(token);
					sb.append(" ");
				}
				logger.error(sb);
				return;
			}
			
			Message message = new Message();
			message.from = from;
			message.recipient = recipient;
			message.tokens = new ArrayList<String>(tokens.size());
			for (String token : tokens) {
				message.tokens.add(token);
			}
			messages.add(message);
		} else {
			for (Player p : getPlayers()) {
				Robot recipient = (Robot)p;
				Message message = new Message();
				message.from = from;
				message.recipient = recipient;
				message.tokens = new ArrayList<String>();
				message.tokens.addAll(tokens);
				messages.add(message);
			}
		}
	}
}
