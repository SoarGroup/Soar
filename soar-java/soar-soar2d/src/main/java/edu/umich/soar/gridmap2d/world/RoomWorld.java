package edu.umich.soar.gridmap2d.world;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.CognitiveArchitecture;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.config.PlayerConfig;
import edu.umich.soar.gridmap2d.map.GridMap;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.RoomCommander;
import edu.umich.soar.gridmap2d.players.RoomPlayer;
import edu.umich.soar.gridmap2d.players.RoomPlayerState;
import edu.umich.soar.robot.DifferentialDriveCommand;

public class RoomWorld implements World {
	private static Logger logger = Logger.getLogger(RoomWorld.class);

	private RoomMap roomMap;
	private PlayersManager<RoomPlayer> players = new PlayersManager<RoomPlayer>();
	private boolean forceHuman = false;
	private List<String> stopMessages = new ArrayList<String>();
	// TODO fix
	//private double speed = 16;
	public static final int cell_size = 16;
	private double rotate_speed = Math.PI / 4.0;
	private CognitiveArchitecture cogArch;

	public RoomWorld(CognitiveArchitecture cogArch) {
		this.cogArch = cogArch;
	}

	@Override
	public void addPlayer(String playerId, PlayerConfig playerConfig, boolean debug) throws Exception {
		
		RoomPlayer player = new RoomPlayer(playerId);

		players.add(player, roomMap, playerConfig.pos);
		
		if (playerConfig.productions != null) {
			RoomCommander eaterCommander = cogArch.createRoomCommander(player, this, playerConfig.productions, playerConfig.shutdown_commands, roomMap.getMetadataFile(), debug);
			player.setCommander(eaterCommander);
		} else if (playerConfig.script != null) {
			// TODO: implement
		}

		int [] location = WorldUtil.getStartingLocation(player, roomMap, players.getInitialLocation(player));
		players.setLocation(player, location);
		
		// put the player in it
		roomMap.getCell(location).setPlayer(player);
		
		player.getState().setLocationId(roomMap.getLocationId(location));
		double [] floatLocation = defaultFloatLocation(location);
		players.setFloatLocation(player, floatLocation);

		logger.info(player.getName() + ": Spawning at (" + location[0] + "," + location[1] + "), (" + floatLocation[0] + "," + floatLocation[1] + ")");
		
		updatePlayers();
	}

	@Override
	public GridMap getMap() {
		return roomMap;
	}

	@Override
	public Player[] getPlayers() {
		return players.getAllAsPlayers();
	}

	@Override
	public void interrupted(String agentName) throws Exception {
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
	public void removePlayer(String name) throws Exception {
		RoomPlayer player = players.get(name);
		roomMap.getCell(players.getLocation(player)).setPlayer(null);
		players.remove(player);
		player.shutdownCommander();
		updatePlayers();
	}

	@Override
	public void reset() throws Exception {
		roomMap.reset();
		resetState();
	}

	@Override
	public void setForceHumanInput(boolean setting) {
		forceHuman = setting;
	}

	@Override
	public void setMap(String mapPath) throws Exception {
		roomMap = new RoomMap(mapPath);
		resetState();
	}

	private void resetState() throws Exception {
		stopMessages.clear();
		resetPlayers();
	}

	private void resetPlayers() throws Exception {
		if (players.numberOfPlayers() == 0) {
			return;
		}
		
		for (RoomPlayer player : players.getAll()) {
			player.reset();
			
			// find a suitable starting location
			int [] startingLocation = WorldUtil.getStartingLocation(player, roomMap, players.getInitialLocation(player));
			players.setLocation(player, startingLocation);

			// put the player in it
			roomMap.getCell(startingLocation).setPlayer(player);

			player.getState().setLocationId(roomMap.getLocationId(startingLocation));

			double [] floatLocation = defaultFloatLocation(startingLocation);
			players.setFloatLocation(player, floatLocation);

			logger.info(player.getName() + ": Spawning at (" + startingLocation[0] + "," + startingLocation[1] + "), (" + floatLocation[0] + "," + floatLocation[1] + ")");
		}
		
		updatePlayers();
	}
	
	private double [] defaultFloatLocation(int [] location) {
		double [] floatLocation = new double [2];
		final int cellSize = cell_size;
		
		// default to center of square
		floatLocation[0] = (location[0] * cellSize) + (cellSize / 2); 
		floatLocation[1] = (location[1] * cellSize) + (cellSize / 2); 

		return floatLocation;
	}
	
	@Override
	public void update(int worldCount) throws Exception {
		WorldUtil.checkNumPlayers(players.numberOfPlayers());

		// Collect input
		for (RoomPlayer player : players.getAll()) {
			player.resetPointsChanged();

			CommandInfo command = forceHuman ? Gridmap2D.control.getHumanCommand(player) : player.getCommand();
			if (command == null) {
				Gridmap2D.control.stopSimulation();
				return;
			}
			players.setCommand(player, command);
			WorldUtil.checkStopSim(stopMessages, command, player);
		}
		
		moveRoomPlayers(Gridmap2D.control.getTimeSlice());
		
		updatePlayers();
	}

	private void moveRoomPlayers(double time) throws Exception {
		for (RoomPlayer player : players.getAll()) {
			CommandInfo command = players.getCommand(player);	
			RoomPlayerState state = player.getState();
			
			DifferentialDriveCommand ddc = command.ddc;
			switch(ddc.getType()) {
			case ANGVEL:
				state.setAngularVelocity(ddc.getAngularVelocity());
				state.resetDestinationHeading();
				break;
			case ESTOP:
				state.stop();
				break;
			case HEADING:
				calculateAndSetAngularVelocity(ddc.getHeading(), player, time);
				break;
			case HEADING_LINVEL:
				calculateAndSetAngularVelocity(ddc.getHeading(), player, time);
				state.setLinearVelocity(ddc.getLinearVelocity());
				break;
			case LINVEL:
				state.setLinearVelocity(ddc.getLinearVelocity());
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
				state.setLinearVelocity(ddc.getLinearVelocity());
				break;
			}
			
			// update heading if we rotate
			if (state.getAngularVelocity() != 0) {
				double heading = state.getHeading();
				heading += state.getAngularVelocity();
				if (state.hasDestinationHeading()) {
					if (state.getAngularVelocity() > 0) {
						if (Double.compare(heading, state.getDestinationHeading()) >= 0) {
							heading = state.getDestinationHeading();
							logger.debug("Destination heading reached: " + heading);
							player.getState().setAngularVelocity(0);
							state.resetDestinationHeading();
						} 
						else {
							logger.debug("Destination heading pending");
						}
					} 
					else {
						if (Double.compare(heading, state.getDestinationHeading()) <= 0) {
							heading = state.getDestinationHeading();
							logger.debug("Destination heading reached: " + heading);
							player.getState().setAngularVelocity(0);
							state.resetDestinationHeading();
						} 
						else {
							logger.debug("Destination heading pending");
						}
					}
				}
				logger.debug("rotated to " + heading);
				state.setHeading(heading); // does mod2pi
			}
			
			// reset collision sensor
			state.setCollisionX(false);
			state.setCollisionY(false);

			// TODO: if we have velocity, process move
			roomMovePlayer(player, time);
		}
	}

	private void updatePlayers() throws Exception {
		for (RoomPlayer player : players.getAll()) {
			player.update(players.getLocation(player), players.getFloatLocation(player), roomMap);
		}
	}

	private boolean checkBlocked(int [] location) {
		if (roomMap.getCell(location).hasAnyWithProperty(Names.kPropertyBlock)) {
			return true;
		}
		return false;
	}
	
	private void roomMovePlayer(RoomPlayer player, double time) {
		int [] oldLocation = players.getLocation(player);
		int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);

		double [] oldFloatLocation = players.getFloatLocation(player);
		double [] newFloatLocation = Arrays.copyOf(oldFloatLocation, oldFloatLocation.length);

		RoomPlayerState state = player.getState();

		// TODO: fix
		//newFloatLocation[0] += state.getSpeed() * Math.cos(state.getHeading()) * time;
		//newFloatLocation[1] += state.getSpeed() * Math.sin(state.getHeading()) * time;
		
		newLocation[0] = (int)newFloatLocation[0] / cell_size;
		newLocation[1] = (int)newFloatLocation[1] / cell_size;
		
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
						newFloatLocation[1] = oldLocation[1] * cell_size;
						newFloatLocation[1] += cell_size - 0.1;
						newLocation[1] = oldLocation[1];
					} 
					else if (newLocation[1] < oldLocation[1]) {
						// north
						newFloatLocation[1] = oldLocation[1] * cell_size;
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
						newFloatLocation[0] = oldLocation[0] * cell_size;
						newFloatLocation[0] += cell_size - 0.1;
						newLocation[0] = oldLocation[0];
					} 
					else if (newLocation[0] < oldLocation[0]) {
						// west
						newFloatLocation[0] = oldLocation[0] * cell_size;
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
				newFloatLocation[0] = oldLocation[0] * cell_size;
				newFloatLocation[0] += cell_size - 0.1;
				newLocation[0] = oldLocation[0];
			} 
			else if (newLocation[0] < oldLocation[0]) {
				state.setCollisionX(true);
				// west
				newFloatLocation[0] = oldLocation[0] * cell_size;
				newLocation[0] = oldLocation[0];
			} 
			else if (newLocation[1] > oldLocation[1]) {
				state.setCollisionY(true);
				// south
				newFloatLocation[1] = oldLocation[1] * cell_size;
				newFloatLocation[1] += cell_size - 0.1;
				newLocation[1] = oldLocation[1];
			} 
			else if (newLocation[1] < oldLocation[1]) {
				state.setCollisionY(true);
				// north
				newFloatLocation[1] = oldLocation[1] * cell_size;
				newLocation[1] = oldLocation[1];
			}
		}
		
		// TODO: fix
		//state.setVelocity(new double [] { (newFloatLocation[0] - oldFloatLocation[0])/time, (newFloatLocation[1] - oldFloatLocation[1])/time });
		roomMap.getCell(oldLocation).setPlayer(null);
		players.setLocation(player, newLocation);
		players.setFloatLocation(player, newFloatLocation);
		state.setLocationId(roomMap.getLocationId(newLocation));
		roomMap.getCell(newLocation).setPlayer(player);
	}
	
	void calculateAndSetAngularVelocity(double targetHeading, RoomPlayer player, double time) {
		double toGo = targetHeading - player.getState().getHeading();
		logger.debug("rotation togo: " + toGo);
		if (toGo < 0) {
			player.getState().setAngularVelocity(rotate_speed * -1 * time);
			player.getState().setDestinationHeading(targetHeading);
		} 
		else if (toGo > 0) {
			player.getState().setAngularVelocity(rotate_speed * time);
			player.getState().setDestinationHeading(targetHeading);
		} 
		else {
			player.getState().setAngularVelocity(0);
			player.getState().resetDestinationHeading();
		}
	}
}
