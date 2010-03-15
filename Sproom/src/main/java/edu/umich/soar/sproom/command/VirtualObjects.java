package edu.umich.soar.sproom.command;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import lcm.lcm.LCM;
import lcmtypes.sim_obstacles_t;

import edu.umich.soar.sproom.command.VirtualObject.Type;

import april.config.Config;

/**
 * Collection of virtual objects managed by MapMetadata.
 *
 * @auther voigtjr@gmail.com
 */
public class VirtualObjects implements Iterable<VirtualObject> {
	private static final Log logger = LogFactory.getLog(VirtualObjects.class);

	private final Map<Integer, VirtualObject> objs = new ConcurrentHashMap<Integer, VirtualObject>();
	private ScheduledExecutorService exec = Executors.newSingleThreadScheduledExecutor();
	private final LCM lcm = LCM.getSingleton();
	
	public VirtualObjects(Config config) {
		for (String objectNickname : config.getStrings("metadata.objects", new String[0])) {
			
			String typeString = config.getString("metadata." + objectNickname + ".type");
			Type type = VirtualObject.Type.valueOf(typeString.toUpperCase());

			double[] pos = config.getDoubles("metadata." + objectNickname + ".pos");
			double[] size = config.getDoubles("metadata." + objectNickname + ".size");
			double theta = config.getDouble("metadata." + objectNickname + ".theta", 0);
			
			createObject(type, pos, size, theta);
		}
		
		exec.scheduleAtFixedRate(new Runnable() {
			@Override
			public void run() {
				List<double[]> rectList = new ArrayList<double[]>();
				
				for (VirtualObject object : objs.values()) {
					rectList.add(new double[] { object.getPos()[0], object.getPos()[1], 
							object.getSize()[0], object.getSize()[1], object.getTheta() });
				}
				sim_obstacles_t obs = new sim_obstacles_t();
				obs.nrects = rectList.size();
				obs.rects = rectList.toArray(new double[0][0]);
				obs.generation = 0;
				lcm.publish("SIM_OBSTACLES", obs);
			}
		}, 0, 250, TimeUnit.MILLISECONDS);
	}
	
	public VirtualObject createObject(Type type, double[] pos, double[] size) {
		return createObject(type, pos, size, 0);
	}
	
	public VirtualObject createObject(Type type, double[] pos, double[] size, double theta) {
		VirtualObject object = new VirtualObject(type, pos, size, theta);
		logger.trace("Created object " + object);
		addObject(object);
		return object;
	}
	
	public void addObject(VirtualObject object) {
		objs.put(object.getId(), object);
		logger.trace(String.format("Added object %d, count %d", object.getId(), objs.size()));
	}
	
	public VirtualObject getObject(int id) {
		return objs.get(id);
	}
	
	public VirtualObject removeObject(int id) {
		if (logger.isTraceEnabled()) {
			if (objs.containsKey(id)) {
				logger.trace(String.format("Removed object %d, count %d", id, objs.size() - 1));
			} else {
				logger.trace(String.format("No such object " + id));
			}
		}
		return objs.remove(id);
	}
	
	@Override
	public Iterator<VirtualObject> iterator() {
		return Collections.unmodifiableMap(objs).values().iterator();
	}
}
