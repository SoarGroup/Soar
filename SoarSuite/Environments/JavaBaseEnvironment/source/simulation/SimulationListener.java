package simulation;

public interface SimulationListener {
	// TODO: split update event out since it will slow things down
	public static final int kNoEvent 				= 0;
	public static final int kStartEvent 			= 1;
	public static final int kStopEvent 				= 2;
	public static final int kErrorMessageEvent 		= 3;
	public static final int kUpdateEvent 			= 4;
	public static final int kResetEvent 			= 5;
	public static final int kAgentCreatedEvent 		= 6;
	public static final int kAgentDestroyedEvent 	= 7;
	public static final int kNotificationEvent 		= 8;
	public static final int kHumanInputEvent 		= 9;
	
	public void simulationEventHandler(int type);
}
