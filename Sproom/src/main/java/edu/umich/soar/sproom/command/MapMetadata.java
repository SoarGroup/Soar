package edu.umich.soar.sproom.command;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import jmat.LinAlg;

import lcm.lcm.LCM;
import lcmtypes.pose_t;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.command.MapMetadata.Walls.WallDir;

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

	public static class Walls {
		public enum WallDir { NORTH, SOUTH, EAST, WEST };
		private final HashMap<WallDir, double[]> walls = new HashMap<WallDir, double[]>(4);
		
		private Walls(double[] pos, double[] xySize) {
			double[] midpoint = LinAlg.scale(xySize, 0.5);
			walls.put(WallDir.NORTH, new double[] { pos[0] + midpoint[0], pos[1] + xySize[1], 0 });
			walls.put(WallDir.SOUTH, new double[] { pos[0] + midpoint[0], pos[1], 0 });
			walls.put(WallDir.EAST, new double[] { pos[0] + xySize[0], pos[1] + midpoint[1], 0 });
			walls.put(WallDir.WEST, new double[] { pos[0], pos[1] + midpoint[1], 0 });
		}
		
		public double[] getPos(WallDir dir) {
			double[] temp = new double[3];
			System.arraycopy(walls.get(dir), 0, temp, 0, temp.length);
			return temp;
		}
	}
	
	public static class Area {
		private final int id;
		private final boolean door;
		private final double[] pos;
		private final double[] xySize;
		private final List<Gateway> gateways = new ArrayList<Gateway>();
		private final List<WallDir> dirs = new ArrayList<WallDir>();
		private final Walls walls;
		
		private Area(int id, boolean door, double[] pos, double[] xySize) {
			this.id = id;
			this.door = door;
			this.pos = pos;
			this.xySize = xySize;
			
			this.walls = new Walls(pos, xySize);
		}
		
		public int getId() {
			return id;
		}
		
		public boolean isDoor() {
			return door;
		}
		
		public List<Gateway> getGateways() {
			return Collections.unmodifiableList(gateways);
		}
		
		public List<WallDir> getDirs() {
			return Collections.unmodifiableList(dirs);
		}
		
		public Walls getWalls() {
			return walls;
		}
		
		@Override
		public String toString() {
			StringBuilder sb = new StringBuilder("Area ");
			sb.append(id);
			if (door) {
				sb.append(" (door)");
			}
			sb.append(String.format(" [%2.2f, %2.2f]", pos[0], pos[1]));
			sb.append(String.format(" [%2.2f, %2.2f]", xySize[0], xySize[1]));
			sb.append(" gateway ids:");
			for (int i = 0; i < gateways.size(); ++i) {
				sb.append(" ");
				sb.append(gateways.get(i).id);
				sb.append(dirs.get(i).toString().charAt(0));
			}
			return sb.toString();
		}
	}
	
	public static class Gateway {
		private final int id;
		private final double[] pos;
		private final List<Area> to = new ArrayList<Area>(2);
		
		private Gateway(int id, double[] pos) {
			this.id = id;
			this.pos = pos;
		}
		
		public int getId() {
			return id;
		}
		
		public List<Area> getTo() {
			return Collections.unmodifiableList(to);
		}
		
		public double[] getPos() {
			return new double[] { pos[0], pos[1], 0 };
		}
		
		@Override
		public String toString() {
			StringBuilder sb = new StringBuilder("Gateway ");
			sb.append(id);
			sb.append(" to:");
			for (Area area : to) {
				sb.append(" ");
				sb.append(area.id);
			}
			sb.append(String.format(" [%2.2f, %2.2f]", pos[0], pos[1]));
			return sb.toString();
		}
	}
	
	private final List<Area> areaList;
	private final List<Gateway> gatewayList;
	private final LCM lcm = LCM.getSingleton();
	
	private class UnitConverter {
		final double[] origin;
		final double scale;
		UnitConverter() {
			origin = null; // in meters, no conversion necessary
			scale = 1;
		}
		UnitConverter(int[] origin, double scale) {
			this.origin = new double[] { origin[0] * scale, origin[1] * scale }; // pixels -> meters
			this.scale = scale;
		}
		double[] getPos(Config config, String nick) {
			return getPos(config, nick, new double[] {0, 0});
		}
		
		double[] getPos(Config config, String nick, double[] size) {
			if (origin == null) {
				return config.getDoubles("metadata." + nick + ".pos");
			}
			int[] loc = config.getInts("metadata." + nick + ".pos");
			double[] pos = new double[] { loc[0] * scale, loc[1] * scale };
			return new double[] { pos[0] - origin[0], ((pos[1] + size[1]) - origin[1]) * -1 };
		}
		
		double[] getSize(Config config, String nick) {
			if (origin == null) {
				return config.getDoubles("metadata." + nick + ".size");
			}
			int[] size = config.getInts("metadata." + nick + ".size");
			
			return new double[] { size[0] * scale, size[1] * scale };
		}
	}

	public MapMetadata(Config config) {
		UnitConverter u;
		if (config.getString("metadata.units", "meters").equals("pixels")) {
			int[] origin = config.getInts("metadata.origin");
			double scale = config.requireDouble("metadata.scale");
			u = new UnitConverter(origin, scale);
		} else {
			u = new UnitConverter();
		}
		
		int numAreas = config.getStrings("metadata.areas", new String[0]).length;
		Map<String, Area> areaMap = new HashMap<String, Area>(numAreas);
		areaList = new ArrayList<Area>(numAreas);
		for(String areaNickname : config.getStrings("metadata.areas", new String[0])) {
			boolean door = config.getBoolean("metadata." + areaNickname + ".door", false);
			
			double[] xySize = u.getSize(config, areaNickname);
			if (xySize == null) {
				logger.error("no xySize on " + areaNickname);
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
