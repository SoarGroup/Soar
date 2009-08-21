package edu.umich.soar.gridmap2d.soar;

import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import jmat.LinAlg;

import lcmtypes.pose_t;

import sml.Agent;
import sml.Identifier;
import sml.Kernel;
import sml.smlSystemEventId;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.map.RoomObject;
import edu.umich.soar.gridmap2d.players.CarryInterface;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.Robot;
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
	private final Map<String, SoarRobotObjectIL> players = new HashMap<String, SoarRobotObjectIL>();
	private final Map<Integer, SoarRobotObjectIL> objects = new HashMap<Integer, SoarRobotObjectIL>();
	private long runtime;
	private final CarryInterface ci;
	
	public SoarRobotInputLinkManager(Agent agent, Kernel kernel, OffsetPose opose, CarryInterface ci) {
		this.agent = agent;
		this.kernel = kernel;
		this.opose = opose;
		this.ci = ci;
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
		selfIL = new SoarRobotSelfIL(agent, self, opose, configurationIL, ci);
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
		objects.clear();

		for (SoarRobotObjectIL object : players.values()) {
			object.destroy();
		}
		players.clear();

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

	public void update(Robot player, RoomWorld world, RoomMap roomMap,
			boolean floatYawWmes) {
		
		configurationIL.update();
		// FIXME: should be configurable
		//timeIL.update();
		runtime += (long)(Gridmap2D.control.getTimeSlice() * 1000000000L);
		timeIL.updateExact(runtime);
		selfIL.update(player);
		
		if (areaIL == null || oldLocationId != player.getState().getLocationId()) {
			oldLocationId = player.getState().getLocationId();
		
			if (areaIL != null) {
				areaIL.destroy();
			}
			
			Identifier areaDescription = agent.GetInputLink().CreateIdWME("area-description");
			boolean door = roomMap.getCell(player.getLocation()).hasObjectWithProperty(Names.kPropertyGatewayRender);
			areaIL = new SoarRobotAreaDescriptionIL(areaDescription, player.getState().getLocationId(), opose, roomMap, door);
		}
		
		// objects
		Collection<RoomObject> roomObjects = roomMap.getRoomObjects();
		for (RoomObject rObj : roomObjects) {
			pose_t pose = rObj.getPose();
			if (pose == null) {
				continue; // not on map
			}
			if (rObj.getArea() == player.getState().getLocationId()) {
				final double MAX_ANGLE_OFF = Math.PI / 2;
				LinAlg.scaleEquals(pose.pos, SoarRobot.PIXELS_2_METERS);
				PointRelationship r = PointRelationship.calculate(opose.getPose(), pose.pos);
				if (Math.abs(r.getRelativeBearing()) <= MAX_ANGLE_OFF) {
					CellObject cObj = rObj.getCellObject();
					SoarRobotObjectIL oIL = objects.get(rObj.getId());
					if (oIL == null) {
						// create new object
						Identifier inputLink = agent.GetInputLink();
						Identifier parent = inputLink.CreateIdWME("object");
						oIL = new SoarRobotObjectIL(parent);
						oIL.initialize(pose, r);
						oIL.addProperty("type", cObj.getProperty("name"));
						oIL.addProperty("id", rObj.getId());
						oIL.addProperty("color", cObj.getProperty("color"));
						oIL.addProperty("weight", cObj.getProperty("weight", 0d, Double.class));
						objects.put(rObj.getId(), oIL);
					} else {
						oIL.update(pose, r);
					}
				}
			}
		}
		
		// players
		if (world.getPlayers().length > 1) {
			for (Player temp : world.getPlayers()) {
				Robot rTarget = (Robot)temp;
				if (rTarget.equals(player)) {
					continue;
				}
				pose_t rTargetPose = rTarget.getState().getPose();
				LinAlg.scaleEquals(rTargetPose.pos, SoarRobot.PIXELS_2_METERS);
				LinAlg.scaleEquals(rTargetPose.vel, SoarRobot.PIXELS_2_METERS);
				PointRelationship r = PointRelationship.calculate(opose.getPose(), rTargetPose.pos);
				String rName = rTarget.getName();
				SoarRobotObjectIL pIL = players.get(rName);
				if (pIL == null) {
					// create new player
					Identifier inputLink = agent.GetInputLink();
					Identifier parent = inputLink.CreateIdWME("object");
					pIL = new SoarRobotObjectIL(parent);
					pIL.initialize(rTargetPose, r);
					pIL.addProperty("type", "player");
					pIL.addProperty("name", rName);
					pIL.addProperty("color", rTarget.getColor());
					players.put(rName, pIL);
				
				} else {
					pIL.update(rTargetPose, r);
				}
			}
		}

		purge(Gridmap2D.simulation.getWorldCount());

		areaIL.update();
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
