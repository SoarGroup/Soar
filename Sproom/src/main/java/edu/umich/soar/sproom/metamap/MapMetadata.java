package edu.umich.soar.sproom.metamap;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


import lcm.lcm.LCM;
import lcmtypes.pose_t;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;


import april.config.Config;
import april.config.ConfigUtil;

/**
 * Holds on to simulated data about the map (such as room IDs, wall locations, gateways)
 * keyed by the agent's current location.
 *
 * @author voigtjr@gmail.com
 */
public class MapMetadata {
	private static final Log logger = LogFactory.getLog(MapMetadata.class);

	private final List<Area> areaList;
	private final List<Gateway> gatewayList;
	private final LCM lcm = LCM.getSingleton();
	
	public MapMetadata(Config config) {
		UnitConverter u = UnitConverter.getInstance(config);
		
		int numAreas = config.getStrings("metadata.areas", new String[0]).length;
		Map<String, Area> areaMap = new HashMap<String, Area>(numAreas);
		areaList = new ArrayList<Area>(numAreas);
		for(String areaNickname : config.getStrings("metadata.areas", new String[0])) {
			boolean door = config.getBoolean("metadata." + areaNickname + ".door", false);
			
			double[] xySize = u.getSize(config, areaNickname);
			if (xySize == null) {
				logger.error("no size on " + areaNickname);
				throw new IllegalStateException();
			}
			
			double[] pos = u.getPos(config, areaNickname, xySize);
			if (pos == null) {
				logger.error("no pos on " + areaNickname);
				throw new IllegalStateException();
			}
			
			Area area = new Area(areaMap.size(), door, pos, xySize);
			areaMap.put(areaNickname, area);
			
			logger.trace("initial area add " + area);
			areaList.add(area);
		}
		
		int numGateways = config.getStrings("metadata.gateways", new String[0]).length;
		gatewayList = new ArrayList<Gateway>(numGateways);
		for(String gatewayNickname : config.getStrings("metadata.gateways", new String[0])) {
			
			double[] pos = u.getPos(config, gatewayNickname);
			if (pos == null) {
				logger.error("no pos on " + gatewayNickname);
				throw new IllegalStateException();
			}
			
			Gateway gateway = new Gateway(areaMap.size() + gatewayList.size(), pos);

			for (int i = 0; i < 2; ++i) {
				String to = config.getStrings("metadata." + gatewayNickname + ".to")[i];
				WallDir d = WallDir.valueOf(config.getStrings("metadata." + gatewayNickname + ".dir")[i].toUpperCase());

				Area area = areaMap.get(to);
				if (area == null) {
					logger.error("no area for " + to + " on " + gatewayNickname);
					throw new IllegalStateException();
				}
				gateway.to.add(area);
				area.gateways.add(gateway);
				area.dirs.add(d);
			}
			
			logger.debug("adding " + gateway);
			gatewayList.add(gateway);
		}
		
		if (logger.isDebugEnabled()) {
			for (Area area : areaList) {
				logger.debug("final area: " + area);
			}
		}
	}
	
	public Area getArea(double[] pos) {

		// TODO: bsp
		for (Area area : areaList) {
			
			if (pos[0] < area.pos[0]) {
				continue;
			}
			if (pos[1] < area.pos[1]) {
				continue;
			}
			if (pos[0] >= area.pos[0] + area.xySize[0]) {
				continue;
			}
			if (pos[1] >= area.pos[1] + area.xySize[1]) {
				continue;
			}

			return area;
		}
		return null;
	}
	
	private final long PUBLISH_DELAY_NANOS = 50000000L;
	private long lastPublish = 0;
	
	public void publish(Area area) {
		long now = System.nanoTime();
		long elapsed = now - lastPublish;
		if (elapsed > PUBLISH_DELAY_NANOS) {
			lastPublish = now;
			pose_t pose = new pose_t();
			pose.utime = now;
			pose.pos = new double[] { area.pos[0], area.pos[1], 0 };
			pose.vel = new double[] { area.xySize[0], area.xySize[1], 0 };
			lcm.publish("AREA_DESCRIPTIONS", pose);
		}
	}

	public static final void main(String[] args) {
		new MapMetadata(ConfigUtil.getDefaultConfig(args));
	}
}
