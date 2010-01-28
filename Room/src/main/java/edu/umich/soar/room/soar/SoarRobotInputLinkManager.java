package edu.umich.soar.room.soar;

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
import edu.umich.soar.sps.control.robot.PointRelationship;
import edu.umich.soar.sps.control.robot.ReceiveMessagesInterface;
import edu.umich.soar.sps.control.robot.OffsetPose;
import edu.umich.soar.sps.control.robot.WaypointInterface;
import edu.umich.soar.sps.control.robot.TimeIL;
import edu.umich.soar.sps.control.robot.ConfigurationIL;
import edu.umich.soar.room.core.Names;
import edu.umich.soar.room.core.Simulation;
import edu.umich.soar.room.map.CarryInterface;
import edu.umich.soar.room.map.CellObject;
import edu.umich.soar.room.map.Robot;
import edu.umich.soar.room.map.RoomMap;
import edu.umich.soar.room.map.RoomObject;
import edu.umich.soar.room.map.RoomWorld;

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
	private final Simulation sim;
	
	public SoarRobotInputLinkManager(Simulation sim, Agent agent, Kernel kernel, OffsetPose opose, CarryInterface ci) {
		this.sim = sim;
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

	public static final long FIFTY_MSEC_IN_NANOS = 50000000L;
	public void update(Robot player, RoomWorld world, RoomMap roomMap,
			boolean floatYawWmes) {
		
		configurationIL.update();
		// FIXME: should be configurable
		//timeIL.update();
		runtime += FIFTY_MSEC_IN_NANOS; 
		timeIL.updateExact(runtime);
		selfIL.update(player);
		
		if (areaIL == null || oldLocationId != player.getState().getLocationId()) {
			oldLocationId = player.getState().getLocationId();
		
			if (areaIL != null) {
				areaIL.destroy();
			}
			
			Identifier areaDescription = agent.GetInputLink().CreateIdWME("area-description");
			boolean door = roomMap.getCell(player.getState().getLocation()).hasObjectWithProperty(Names.kPropertyGatewayRender);
			areaIL = new SoarRobotAreaDescriptionIL(areaDescription, player.getState().getLocationId(), opose, roomMap, door);
		}
		
		// objects
		Collection<RoomObject> roomObjects = roomMap.getRoomObjects();
		for (RoomObject rObj : roomObjects) {
			pose_t pose = rObj.getPose();
			if (rObj.getPose() == null) {
				// not on map
				continue;
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
						oIL = new SoarRobotObjectIL(sim, parent);
						oIL.initialize(pose, r);
						oIL.updateProperty("type", cObj.getProperty("name"));
						oIL.updateProperty("id", rObj.getId());
						oIL.updateProperty("color", cObj.getProperty("color"));
						oIL.updateProperty("height", cObj.getProperty("height"));
						oIL.updateProperty("smell", cObj.getProperty("smell"));
						oIL.updateProperty("shape", cObj.getProperty("shape"));
						oIL.updateProperty("weight", cObj.getProperty("weight", 0d, Double.class));
						oIL.updateProperty("diffusible", cObj.getProperty("diffusible", false, Boolean.class));
						oIL.updateProperty("diffuse-wire", cObj.getProperty("diffuse-wire"));
						oIL.updateProperty("diffused", cObj.getProperty("diffused", Boolean.class));
						objects.put(rObj.getId(), oIL);
						
					} else {
						oIL.updateProperty("diffused", cObj.getProperty("diffused", Boolean.class));
						oIL.update(pose, r);
					}
				}
			}
		}
		
		// players
		if (!world.getPlayers().isEmpty()) {
			for (Robot temp : world.getPlayers()) {
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
					pIL = new SoarRobotObjectIL(sim, parent);
					pIL.initialize(rTargetPose, r);
					pIL.updateProperty("type", "player");
					pIL.updateProperty("name", rName);
					pIL.updateProperty("color", rTarget.getColor().toString().toLowerCase());
					players.put(rName, pIL);
				
				} else {
					pIL.update(rTargetPose, r);
				}
			}
		}

		purge(sim.getWorldCount());

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
