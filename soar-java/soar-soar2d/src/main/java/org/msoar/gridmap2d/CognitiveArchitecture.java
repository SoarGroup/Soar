package org.msoar.gridmap2d;

import java.io.File;

import org.msoar.gridmap2d.players.Eater;
import org.msoar.gridmap2d.players.EaterCommander;
import org.msoar.gridmap2d.players.RoomCommander;
import org.msoar.gridmap2d.players.RoomPlayer;
import org.msoar.gridmap2d.players.Tank;
import org.msoar.gridmap2d.players.TankCommander;
import org.msoar.gridmap2d.players.Taxi;
import org.msoar.gridmap2d.players.TaxiCommander;
import org.msoar.gridmap2d.world.RoomWorld;


public interface CognitiveArchitecture {

	void seed(int seed);

	EaterCommander createEaterCommander(Eater eater, String productions, int vision, String[] shutdownCommands, File metadataFile, boolean debug) throws Exception;
	TankCommander createTankCommander(Tank tank, String productions, String[] shutdown_commands, File metadataFile, boolean debug) throws Exception;
	TaxiCommander createTaxiCommander(Taxi taxi, String productions, String[] shutdown_commands, File metadataFile, boolean debug) throws Exception;
	RoomCommander createRoomCommander(RoomPlayer player, RoomWorld world, String productions, String[] shutdown_commands, File metadataFile, boolean debug) throws Exception;

	void doBeforeClients() throws Exception;

	void doAfterClients() throws Exception;

	boolean debug();
	
	void destroyPlayer(String name);

	void reload(String name);

	void shutdown();

	String getAgentPath();

	boolean isClientConnected(String debuggerClient);

	boolean haveAgents();

	void runStep();

	void runForever();

}
