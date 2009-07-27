package edu.umich.soar.gridmap2d.soar;

import sml.Agent;
import sml.Identifier;
import sml.Kernel;
import sml.smlSystemEventId;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.players.RoomPlayer;
import edu.umich.soar.gridmap2d.world.RoomWorld;
import edu.umich.soar.robot.ReceiveMessagesInterface;
import edu.umich.soar.robot.OffsetPose;
import edu.umich.soar.robot.WaypointInterface;
import edu.umich.soar.robot.TimeIL;
import edu.umich.soar.robot.ConfigurationIL;

public class SoarRobotInputLinkManager {
	
	private final Agent agent;
	private final Kernel kernel;
	private final OffsetPose opose;
	private SoarRobotSelfIL selfIL;
	private TimeIL timeIL;
	private ConfigurationIL configurationIL;
	private SoarRobotAreaDescriptionIL areaIL;
	private int oldLocationId = -1;

	public SoarRobotInputLinkManager(Agent agent, Kernel kernel, OffsetPose opose) {
		this.agent = agent;
		this.kernel = kernel;
		this.opose = opose;
	}

	public void create() {
		Identifier inputLink = agent.GetInputLink();
		
		Identifier configuration = agent.CreateIdWME(inputLink, "configuration");
		configurationIL = new ConfigurationIL(configuration, opose);

		Identifier time = agent.CreateIdWME(inputLink, "time");
		timeIL = new TimeIL(time);

		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, timeIL, null);
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, timeIL, null);

		Identifier self = agent.CreateIdWME(inputLink, "self");
		selfIL = new SoarRobotSelfIL(agent, self, opose, configurationIL);
	}
	
	public void destroy() {
		configurationIL.destroy();
		configurationIL = null;

		timeIL.destroy();
		timeIL = null;
		
		selfIL.destroy();
		selfIL = null;

		if (areaIL != null) {
			areaIL.destroy();
			areaIL = null;
		}
	}

	public WaypointInterface getWaypointInterface() {
		return selfIL.getWaypointsIL();
	}

	public ReceiveMessagesInterface getReceiveMessagesInterface() {
		return selfIL.getMessagesIL();
	}

	public void update(RoomPlayer player, RoomWorld world, RoomMap roomMap,
			boolean floatYawWmes) {
		
		configurationIL.update();
		timeIL.update();
		selfIL.update(player);
		
		
		if (player.getMoved()) {
			if (oldLocationId != player.getState().getLocationId()) {
				oldLocationId = player.getState().getLocationId();
			
				if (areaIL != null) {
					areaIL.destroy();
				}
				
				Identifier areaDescription = agent.GetInputLink().CreateIdWME("area-description");
				areaIL = new SoarRobotAreaDescriptionIL(areaDescription, player, world);
			}
		}
		
		areaIL.update(player, world);
	}
}
