package edu.umich.soar.gridmap2d.soar;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import jmat.LinAlg;

import lcmtypes.pose_t;

import sml.Agent;
import sml.Identifier;
import sml.Kernel;
import sml.smlSystemEventId;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.RoomPlayer;
import edu.umich.soar.gridmap2d.world.RoomWorld;
import edu.umich.soar.robot.PointRelationship;
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
	private final Map<RoomPlayer, SoarRobotObjectIL> players = new HashMap<RoomPlayer, SoarRobotObjectIL>();
	private final Map<Integer, SoarRobotObjectIL> objects = new HashMap<Integer, SoarRobotObjectIL>();
	private long runtime;
	
	public SoarRobotInputLinkManager(Agent agent, Kernel kernel, OffsetPose opose) {
		this.agent = agent;
		this.kernel = kernel;
		this.opose = opose;
	}

	public void create() {
		runtime = 0;
		
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
		
		for (SoarRobotObjectIL object : objects.values()) {
			object.destroy();
		}

		for (SoarRobotObjectIL object : players.values()) {
			object.destroy();
		}

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
		// FIXME: should be configurable
		//timeIL.update();
		runtime += (long)(Gridmap2D.control.getTimeSlice() * 1000000000L);
		timeIL.updateExact(runtime);
		selfIL.update(player);
		
		if (oldLocationId != player.getState().getLocationId()) {
			oldLocationId = player.getState().getLocationId();
		
			if (areaIL != null) {
				areaIL.destroy();
			}
			
			Identifier areaDescription = agent.GetInputLink().CreateIdWME("area-description");
			areaIL = new SoarRobotAreaDescriptionIL(areaDescription, player.getState().getLocationId(), opose, roomMap);
		}
		
		// objects
		Set<CellObject> roomObjects = roomMap.getRoomObjects();
		for (CellObject obj : roomObjects) {
			RoomMap.RoomObjectInfo info = roomMap.getRoomObjectInfo(obj);
			if (info.area == player.getState().getLocationId()) {
				double maxAngleOff = 180 / 2;
				pose_t temp = info.pose.copy();
				LinAlg.scaleEquals(temp.pos, SoarRobot.PIXELS_2_METERS);
				PointRelationship r = PointRelationship.calculate(opose.getPose(), temp.pos);
				if (Math.abs(r.getRelativeBearing()) <= maxAngleOff) {
					int id = info.object.getIntProperty("object-id", -1);
					String type = info.object.getProperty("id");
					addOrUpdateObject(id, type, temp, world, r);
				}
			}
		}
		
		// players
		if (world.getPlayers().length > 1) {
			for (Player temp : world.getPlayers()) {
				RoomPlayer rTarget = (RoomPlayer)temp;
				if (rTarget.equals(player)) {
					continue;
				}
				pose_t rTargetPose = rTarget.getState().getPose();
				PointRelationship r = PointRelationship.calculate(opose.getPose(), rTargetPose.pos);
				addOrUpdatePlayer(player, rTarget, world, r);
			}
		}

		purge(Gridmap2D.simulation.getWorldCount());

		areaIL.update();
	}

	private void addOrUpdatePlayer(RoomPlayer self, RoomPlayer target, RoomWorld world, PointRelationship r) {
		pose_t targetPose = target.getState().getPose();
		
		SoarRobotObjectIL pIL = players.get(target);
		if (pIL == null) {
			// create new player
			Identifier inputLink = agent.GetInputLink();
			Identifier parent = inputLink.CreateIdWME("object");
			pIL = new SoarRobotObjectIL(parent);
			pIL.initialize(target.getName(), targetPose, r);
			players.put(target, pIL);
		
		} else {
			pIL.update(targetPose, r);
		}
	}
	
	private void addOrUpdateObject(int objectId, String type, pose_t objectPose, RoomWorld world, PointRelationship r) {
		SoarRobotObjectIL oIL = objects.get(objectId);
		if (oIL == null) {
			// create new object
			Identifier inputLink = agent.GetInputLink();
			Identifier parent = inputLink.CreateIdWME("object");
			oIL = new SoarRobotObjectIL(parent);
			oIL.initialize(objectId, type, objectPose, r);
			objects.put(objectId, oIL);
		} else {
			oIL.update(objectPose, r);
		}
	}
	
	private void purge(int cycle) {
		{
			Iterator<SoarRobotObjectIL> oiter = objects.values().iterator();
			while (oiter.hasNext()) {
				SoarRobotObjectIL oIL = oiter.next();
				if (oIL.getCycleTouched() < cycle) {
					if (oIL.getCycleTouched() > cycle - 3) {
						oIL.makeInvisible();
					} else {
						oIL.destroy();
						oiter.remove();
					}
				}
			}
		}
		{
			Iterator<SoarRobotObjectIL> oiter = players.values().iterator();
			while (oiter.hasNext()) {
				SoarRobotObjectIL oIL = oiter.next();
				if (oIL.getCycleTouched() < cycle) {
					if (oIL.getCycleTouched() > cycle - 3) {
						oIL.makeInvisible();
					} else {
						oIL.destroy();
						oiter.remove();
					}
				}
			}
		}
	}
}
