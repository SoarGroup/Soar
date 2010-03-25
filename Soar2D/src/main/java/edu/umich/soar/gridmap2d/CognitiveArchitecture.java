package edu.umich.soar.gridmap2d;

import edu.umich.soar.gridmap2d.players.Eater;
import edu.umich.soar.gridmap2d.players.EaterCommander;
import edu.umich.soar.gridmap2d.players.Tank;
import edu.umich.soar.gridmap2d.players.TankCommander;
import edu.umich.soar.gridmap2d.players.Taxi;
import edu.umich.soar.gridmap2d.players.TaxiCommander;


public interface CognitiveArchitecture {

	void seed(int seed);

	EaterCommander createEaterCommander(Eater eater, String productions, int vision, String[] shutdownCommands, boolean debug);
	TankCommander createTankCommander(Tank tank, String productions, String[] shutdown_commands, boolean debug);
	TaxiCommander createTaxiCommander(Taxi taxi, String productions, String[] shutdown_commands, boolean debug);

	void doBeforeClients();

	void doAfterClients();

	boolean debug();
	
	void destroyPlayer(String name);

	void reload(String name);

	void shutdown();

	boolean isClientConnected(String debuggerClient);

	boolean haveAgents();

	void runStep();

	void runForever();

}
