package edu.umich.soar.room.soar;

import lcmtypes.pose_t;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import sml.WMElement;
import edu.umich.soar.robot.PointRelationship;
import edu.umich.soar.room.core.Simulation;

class SoarRobotObjectIL {
	private Identifier parent;
	private FloatElement distance;
	private FloatElement yaw;
	private FloatElement relativeBearing;
	private FloatElement absRelativeBearing;
	private FloatElement x, y, z;
	private StringElement visible;
	
	private int cycleTouched;
	private final Simulation sim;
	
	SoarRobotObjectIL(Simulation sim, Identifier parent) {
		this.parent = parent;
		this.sim = sim;
	}
	
	void initialize(pose_t pose, PointRelationship r) {
		this.visible = this.parent.CreateStringWME("visible", "yes");

		initInternal(pose, r);
	}
	
	<T> void updateProperty(String key, T value) {
		if (key == null) {
			throw new NullPointerException("updateProperty key is null");
		}
		
		WMElement wme = this.parent.FindByAttribute(key, 0);

		if (value == null) {
			if (wme != null) {
				wme.DestroyWME();
			}
		}
		
		if (value instanceof String) {
			if (wme == null) {
				this.parent.CreateStringWME(key, (String)value);
			} else {
				StringElement e = wme.ConvertToStringElement();
				e.Update((String)value);
			}
			
		} else if (value instanceof Integer) {
			if (wme == null) {
				this.parent.CreateIntWME(key, (Integer)value);
			} else {
				IntElement e = wme.ConvertToIntElement();
				e.Update((Integer)value);
			}
			
		} else if (value instanceof Double) {
			if (wme == null) {
				this.parent.CreateFloatWME(key, (Double)value);
			} else {
				FloatElement e = wme.ConvertToFloatElement();
				e.Update((Double)value);
			}
			
		} else if (value instanceof Boolean) {
			String bstring = (Boolean)value ? Boolean.TRUE.toString() : Boolean.FALSE.toString();
			if (wme == null) {
				this.parent.CreateStringWME(key, bstring);
			} else {
				StringElement e = wme.ConvertToStringElement();
				e.Update(bstring);
			}
		}
	}
	
	private void initInternal(pose_t pose, PointRelationship r) {
		this.x = this.parent.CreateFloatWME("x", pose.pos[0]);
		this.y = this.parent.CreateFloatWME("y", pose.pos[1]);
		this.z = this.parent.CreateFloatWME("z", pose.pos[2]);

		this.distance = this.parent.CreateFloatWME("distance", r.getDistance());
		this.yaw = this.parent.CreateFloatWME("yaw", Math.toDegrees(r.getYaw()));
		this.relativeBearing = this.parent.CreateFloatWME("relative-bearing", Math.toDegrees(r.getRelativeBearing()));
		this.absRelativeBearing = this.parent.CreateFloatWME("abs-relative-bearing", Math.abs(Math.toDegrees(r.getRelativeBearing())));
		
		touch(sim.getWorldCount());
	}
	
	void update(pose_t pose, PointRelationship r) {
		this.x.Update(pose.pos[0]);
		this.y.Update(pose.pos[1]);
		this.z.Update(pose.pos[2]);
		
		this.distance.Update(r.getDistance());
		this.yaw.Update(Math.toDegrees(r.getYaw()));
		this.relativeBearing.Update(Math.toDegrees(r.getRelativeBearing()));
		this.absRelativeBearing.Update(Math.abs(Math.toDegrees(r.getRelativeBearing())));
		
		touch(sim.getWorldCount());
	}
	
	void destroy() {
		parent.DestroyWME();
	}
	
	void touch(int cycle) {
		cycleTouched = cycle;
		if (visible.GetValue().equals("no")) {
			visible.Update("yes");
		}
	}
	
	int getCycleTouched() {
		return cycleTouched;
	}
	
	void makeInvisible() {
		if (visible.GetValue().equals("yes")) {
			visible.Update("no");
		}
	}

}
