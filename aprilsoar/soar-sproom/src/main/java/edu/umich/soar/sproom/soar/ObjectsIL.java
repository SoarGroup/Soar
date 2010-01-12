package edu.umich.soar.sproom.soar;

import java.util.HashMap;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import sml.Identifier;
import edu.umich.soar.IntWme;
import edu.umich.soar.StringWme;
import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.CommandConfig;
import edu.umich.soar.sproom.command.Pose;
import edu.umich.soar.sproom.command.VirtualObject;
import edu.umich.soar.sproom.command.VirtualObjects;
import edu.umich.soar.sproom.command.Pose.RelativePointData;

public class ObjectsIL implements InputLinkElement {
	private static final Log logger = LogFactory.getLog(ObjectsIL.class);
	
	private static final CommandConfig c = CommandConfig.CONFIG;
	
	private class ObjectIL {
		final PointDataIL pointData;
		Identifier objectwme;
		long invisibleTimestamp = 0;
		StringWme visible;
		final int id;
		
		ObjectIL(Identifier objectwme, VirtualObject vo) {
			this.objectwme = objectwme;
			pointData = new PointDataIL(objectwme, vo.getPos());
			
			StringWme.newInstance(objectwme, SharedNames.TYPE, vo.getType().toString().toLowerCase());
			id = vo.getId();
			IntWme.newInstance(objectwme, SharedNames.ID, id);
			visible = StringWme.newInstance(objectwme, SharedNames.VISIBLE, SharedNames.TRUE);
			if (logger.isTraceEnabled()) {
				logger.trace("Object " + id + " created.");
			}
		}
		
		void update(Adaptable app) {
			if (objectwme == null) {
				return;
			}
			
			pointData.update(app);
			RelativePointData rpd = pointData.getRelativePointData();
			
			if (invisibleTimestamp <= 0) {
				if (!isVisible(rpd.relativeYaw)) {
					invisibleTimestamp = System.nanoTime();
					visible.update(SharedNames.FALSE);
					
					if (logger.isTraceEnabled()) {
						logger.trace("Object " + id + " went invisible.");
					}
				}
			} else {
				if (isVisible(rpd.relativeYaw)) {
					invisibleTimestamp = 0;
					visible.update(SharedNames.TRUE);
					if (logger.isTraceEnabled()) {
						logger.trace("Object " + id + " went visible.");
					}
				}
			}

			if (invisibleTimestamp > 0) {
				// destroy if invalid
				if (System.nanoTime() - invisibleTimestamp > c.getVisibleNanoTime()) {
					objectwme.DestroyWME();
					objectwme = null;
					if (logger.isTraceEnabled()) {
						double secondsOld = (System.nanoTime() - invisibleTimestamp) / 1000000000.0;
						logger.trace("Object " + id + " went invalid (" + secondsOld + " sec).");
					}
				}
			}
		}
		
		boolean isValid() {
			return objectwme != null;
		}
		
	}
	
	private static boolean isVisible(double relativeYaw) {
		double fovHalf = c.getFieldOfView() / 2;
		double absRelYaw = Math.abs(relativeYaw);
		return fovHalf >= absRelYaw;
	}

	private final Identifier root;
	private Map<VirtualObject, ObjectIL> objMap = new HashMap<VirtualObject, ObjectIL>();

	public ObjectsIL(Identifier root, Adaptable app) {
		this.root = root;
	}

	@Override
	public void update(Adaptable app) {
		VirtualObjects vobjs = (VirtualObjects)app.getAdapter(VirtualObjects.class);
		Map<VirtualObject, ObjectIL> newObjMap = new HashMap<VirtualObject, ObjectIL>();
		
		Pose pose = (Pose)app.getAdapter(Pose.class);

		// for each object
		for (VirtualObject vo : vobjs) {
			
			// retrieve or create input link representation of object
			ObjectIL oil = objMap.remove(vo);
			if (oil == null) {
				RelativePointData rpd = pose.getRelativePointData(vo.getPos());
				if (!isVisible(rpd.relativeYaw)) {
					continue;
				}
				
				oil = new ObjectIL(root.CreateIdWME(SharedNames.OBJECT), vo);
			}
			
			// update the object
			oil.update(app);
			
			if (oil.isValid()) {
				newObjMap.put(vo, oil);
			}
		}
		
		objMap = newObjMap;
	}

}
