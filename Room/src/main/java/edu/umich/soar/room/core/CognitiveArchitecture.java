package edu.umich.soar.room.core;

import edu.umich.soar.room.map.Robot;
import edu.umich.soar.room.map.RobotCommander;
import edu.umich.soar.room.map.RoomWorld;

public interface CognitiveArchitecture {
	void seed(int seed);

	RobotCommander createRoomCommander(Robot player, RoomWorld world, String productions, String[] shutdown_commands);

	void setDebug(boolean setting);
	boolean debug();
	
	void destroyPlayer(String name);

	void reload(String name);

	void shutdown();

	boolean isClientConnected(String debuggerClient);
	boolean isAsync();
	void setAsync(boolean setting);

	int getUpdateCount();
}
