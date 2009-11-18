package edu.umich.soar.room.map;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

import jmat.LinAlg;

import lcmtypes.pose_t;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sps.control.robot.DifferentialDriveCommand;
import edu.umich.soar.sps.control.robot.SendMessagesInterface;
import edu.umich.soar.room.config.PlayerConfig;
import edu.umich.soar.room.core.Names;
import edu.umich.soar.room.core.Simulation;

public class RoomWorld implements SendMessagesInterface {
	private static Log logger = LogFactory.getLog(RoomWorld.class);

	private RoomMap map;
	private PlayersManager players = new PlayersManager();
	private List<String> stopMessages = new ArrayList<String>();
	private final double LIN_SPEED = 64;
	public static final int CELL_SIZE = 16;
	private double ANG_SPEED = Math.PI / 1.0;
	private String blockManipulationReason;
	private Queue<CommMessage> messages = new ConcurrentLinkedQueue<CommMessage>();
	private final Simulation sim;
	
	public RoomWorld(Simulation sim) {
		this.sim = sim;
	}

	public boolean hasPlayer(String name) {
		return players.get(name) != null;
	}
	
	public Robot addPlayer(PlayerConfig cfg) {
		int [] location = WorldUtil.getStartingLocation(map, cfg.pos);
		if (location == null) {
			sim.error("Room Environment", "There are no suitable starting locations.");
			return null;
		}

		Robot player = new Robot(cfg.name, cfg.color);
		players.add(player, cfg.pos);
		
		// put the player in it
		map.getCell(location).addPlayer(player);
		
		player.getState().setLocationId(map.getLocationId(location));
		double [] floatLocation = defaultFloatLocation(location);
		player.getState().setPos(floatLocation);

		if (cfg.productions != null) {
			RobotCommander cmdr = sim.getCogArch().createRoomCommander(player, this, cfg.productions, cfg.shutdown_commands);
			if (cmdr == null) {
				players.remove(player);
				map.getCell(location).removePlayer(player);
				return null;
			}
			player.setCommander(cmdr);
		} else if (cfg.script != null) {
			sim.info("Room Environment", "Scripted robots not implemented.");
		}

		logger.info(player.getName() + ": Spawning at (" + location[0] + "," + location[1] + "), (" + floatLocation[0] + "," + floatLocation[1] + ")");
		return player;
	}

	public RoomMap getMap() {
		return map;
	}

	public List<Robot> getPlayers() {
		return players.getAll();
	}

	public boolean isTerminal() {
		return stopMessages.size() > 0;
	}

	public int numberOfPlayers() {
		return players.numberOfPlayers();
	}

	public void removePlayer(String name) {
		Robot player = players.get(name);
		map.getCell(player.getState().getLocation()).removePlayer(player);
		players.remove(player);
		player.shutdown();
	}

	public void reset() {
		blockManipulationReason = null;
		messages.clear();
		map.reset();
		resetState();
	}

	public void setAndResetMap(String mapPath) {
		map = new RoomMap(mapPath, sim.getConfig());
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

			// put the player in it
			map.getCell(location).addPlayer(player);

			player.getState().setLocationId(map.getLocationId(location));

			double [] floatLocation = defaultFloatLocation(location);
			player.getState().setPos(floatLocation);

			logger.info(player.getName() + ": Spawning at (" + location[0] + "," + location[1] + "), (" + floatLocation[0] + "," + floatLocation[1] + ")");
		}
	}
	
	private double [] defaultFloatLocation(int [] location) {
		double [] floatLocation = new double [2];
		final int cellSize = CELL_SIZE;
		
		// default to center of square
		floatLocation[0] = (location[0] * cellSize) + (cellSize / 2); 
		floatLocation[1] = (location[1] * cellSize) + (cellSize / 2); 

		return floatLocation;
	}
	
	public void update(int worldCount) {
		WorldUtil.checkNumPlayers(sim, players.numberOfPlayers());

		// Collect input
		for (Robot player : players.getAll()) {
			RobotCommand command = player.getCommand();
			if (command == null) {
				sim.stop();
				return;
			}
			players.setCommand(player, command);
			WorldUtil.checkStopSim(sim, stopMessages, command.isStopSim(), player);
		}
		
		moveRoomPlayers();
		
		for (CommMessage message : messages) {
			for (CommListener listener : commListeners) {
				listener.write(message);
			}
			if (message.isBroadcast()) {
				for (Robot recipient : players.getAll()) {
					recipient.getReceiveMessagesInterface().newMessage(message.getFrom(), message.getTokens());
				}
			} else {
				Robot recipient = players.get(message.getDestination());
				if (recipient != null) {
					recipient.getReceiveMessagesInterface().newMessage(message.getFrom(), message.getTokens());
				}
			}
		}
		messages.clear();
	}

	private void moveRoomPlayers() {
		for (Robot player : players.getAll()) {
			RobotCommand command = players.getCommand(player);	
			RobotState state = player.getState();
			
			DifferentialDriveCommand ddc = state.isMalfunctioning() ? DifferentialDriveCommand.newEStopCommand() : command.getDdc();
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

			roomMovePlayer(player);
		}
	}

	private boolean checkBlocked(int [] location) {
		if (map.getCell(location).hasObjectWithProperty(Names.kPropertyBlock)) {
			return true;
		}
		return false;
	}
	
	private void roomMovePlayer(Robot player) {
		int [] oldLocation = player.getState().getLocation();
		int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);

		RobotState state = player.getState();
		state.update();
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
			state.setLocationId(map.getLocationId(newLocation));
			map.getCell(newLocation).addPlayer(player);
		}
	}

	public boolean dropObject(Robot player, int id) {
		blockManipulationReason = null;
		RobotState state = player.getState();

		if (state.isMalfunctioning()) {
			blockManipulationReason = "Malfunction";
			return false;
		}
		
		if (!state.hasObject()) {
			blockManipulationReason = "Not carrying an object";
			return false;
		}
		
		RoomObject object = state.getRoomObject();
		state.drop();
		object.setPose(state.getPose());
		map.getCell(player.getState().getLocation()).addObject(object.getCellObject());
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
		
		if (player.getState().isMalfunctioning()) {
			blockManipulationReason = "Malfunction";
			return false;
		}
		
		if (player.getState().hasObject()) {
			blockManipulationReason = "Already carrying an object";
			return false;
		}
		
		// note: This is a stupid way to do this.
		for (RoomObject rObj : map.getRoomObjects()) {
			if (rObj.getPose() == null) {
				// not on map
				continue;
			}
			CellObject cObj = rObj.getCellObject();
			
			if (rObj.getId() == id) {
				if (!isWithinTouchDistance(player, rObj)) {
					blockManipulationReason = "Object is too far";
					return false;
				}
				
				if (!cObj.getProperty("diffused", Boolean.TRUE, Boolean.class)) {
					blockManipulationReason = "Object was not diffused and exploded.";
					player.getState().setMalfunction(true);
					
					if (!cObj.getCell().removeObject(cObj)) {
						throw new IllegalStateException("Remove object failed for exploded object that should be there.");
					}
					rObj.destroy();
					return false;
				}
				
				// success
				if (!cObj.getCell().removeObject(cObj)) {
					throw new IllegalStateException("Remove object failed for got object that should be there.");
				}
				rObj.setPose(null);
				player.getState().pickUp(rObj);
				return true;
			}
		}
		blockManipulationReason = "No such object ID";
		return false;
	}

	private boolean isWithinTouchDistance(Robot player, RoomObject rObj) {
		double distance = LinAlg.distance(player.getState().getPose().pos, rObj.getPose().pos);
		return Double.compare(distance, CELL_SIZE) <= 0;
	}
	
	/**
	 * @param player
	 * @param id
	 * @return
	 */
	public boolean diffuseObject(Robot player, int id) {
		blockManipulationReason = null;
		
		if (player.getState().isMalfunctioning()) {
			blockManipulationReason = "Malfunction";
			return false;
		}
		
		// note: This is a stupid way to do this.
		for (RoomObject rObj : map.getRoomObjects()) {
			if (rObj.getPose() == null) {
				// not on map
				continue;
			}
			CellObject cObj = rObj.getCellObject();
			
			if (rObj.getId() == id) {
				if (!isWithinTouchDistance(player, rObj)) {
					blockManipulationReason = "Object is too far";
					return false;
				}
				
				Boolean diffusible = cObj.getProperty("diffusible", false, Boolean.class);
				if (diffusible) {
					if (cObj.getProperty("diffuse-wire") == null) {
						// success
						cObj.setProperty("diffused", Boolean.TRUE.toString());
						return true;
					} else {
						// Boom!
						blockManipulationReason = "Diffusal failed (needed wire command) and object exploded.";
						
						player.getState().setMalfunction(true);
						
						if (!cObj.getCell().removeObject(cObj)) {
							throw new IllegalStateException("Remove object failed for exploded object that should be there.");
						}
						rObj.destroy();
						return false;
					}
				}

				blockManipulationReason = "Object was not diffusable and has been destroyed";

				if (!cObj.getCell().removeObject(cObj)) {
					throw new IllegalStateException("Diffuse object failed for object that should be there.");				
				}
				rObj.destroy();
				return false;
			}
		}
		blockManipulationReason = "No such object ID";
		return false;
	}

	/**
	 * @param player
	 * @param id
	 * @return
	 */
	public boolean diffuseObjectByWire(Robot player, int id, String color) {
		blockManipulationReason = null;
		
		if (player.getState().isMalfunctioning()) {
			blockManipulationReason = "Malfunction";
			return false;
		}
		
		// note: This is a stupid way to do this.
		for (RoomObject rObj : map.getRoomObjects()) {
			if (rObj.getPose() == null) {
				// not on map
				continue;
			}
			CellObject cObj = rObj.getCellObject();
			
			if (rObj.getId() == id) {
				if (!isWithinTouchDistance(player, rObj)) {
					blockManipulationReason = "Object is too far";
					return false;
				}
				
				Boolean diffusible = cObj.getProperty("diffusible", false, Boolean.class);
				if (diffusible) {
					Boolean diffused = cObj.getProperty("diffused", true, Boolean.class);
					if (diffused || color.equals(cObj.getProperty("diffuse-wire"))) {
						// success
						cObj.setProperty("diffused", Boolean.TRUE.toString());
						return true;
					} else {
						// Boom!
						blockManipulationReason = "Diffusal failed (wrong wire) and object exploded.";
						
						player.getState().setMalfunction(true);
						
						if (!cObj.getCell().removeObject(cObj)) {
							throw new IllegalStateException("Remove object failed for exploded object that should be there.");
						}
						rObj.destroy();
						return false;
					}
				}

				blockManipulationReason = "Object was not diffusable and has been destroyed";

				if (!cObj.getCell().removeObject(cObj)) {
					throw new IllegalStateException("Diffuse object failed for object that should be there.");				
				}
				rObj.destroy();
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

	@Override
	public void sendMessage(String from, String to, List<String> tokens) {
		if (from == null || tokens == null) {
			throw new NullPointerException();
		}
		if (to != null) {
			CommMessage message = new CommMessage(from, to, tokens);
			messages.add(message);
		} else {
			// broadcast
			CommMessage message = new CommMessage(from, tokens);
			messages.add(message);
		}
	}

	private final List<CommListener> commListeners = new ArrayList<CommListener>();;
	public void addCommListener(CommListener commListener) {
		this.commListeners.add(commListener);
		// TODO: removeCommListener
	}
	
}
