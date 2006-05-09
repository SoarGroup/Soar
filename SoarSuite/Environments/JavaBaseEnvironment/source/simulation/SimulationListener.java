package simulation;

public interface SimulationListener {
	// TODO: split update event out since it will slow things down
	public static final int 
		kNoEvent = 0,
		kStartEvent = 1,
		kStopEvent = 2,
		kErrorMessageEvent = 3,
		kUpdateEvent = 4,
		kResetEvent = 5,
		kAgentCreatedEvent = 6,
		kAgentDestroyedEvent = 7,
		kNotificationEvent = 8;
	
	public void simulationEventHandler(int type);
}
