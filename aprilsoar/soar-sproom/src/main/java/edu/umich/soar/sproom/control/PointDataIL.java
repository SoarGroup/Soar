package edu.umich.soar.sproom.control;

import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;

class PointDataIL {
	private Identifier parent;
	
	private FloatElement distance;
	private final double[] pos = new double[3];
	private YawWmes yawWmes;
	
	PointDataIL(double[] xyz, Identifier parent) {
		System.arraycopy(xyz, 0, this.pos, 0, xyz.length);
		this.parent = parent;
	}

	private interface YawWmes {
		public void create();
		public void update(double yawValueDeg, double relativeBearingValueDeg, double absRelativeBearingValueDeg);
	}
	
	private class YawIntWmes implements YawWmes {
		private IntElement yawWmeI;
		private IntElement relativeBearingWmeI;
		private IntElement absRelativeBearingWmeI;

		public void create() {
			yawWmeI = parent.CreateIntWME("yaw", 0);
			relativeBearingWmeI = parent.CreateIntWME("relative-bearing", 0);
			absRelativeBearingWmeI = parent.CreateIntWME("abs-relative-bearing", 0);
		}
		
		public void update(double yawValueDeg, double relativeBearingValueDeg, double absRelativeBearingValueDeg) {
			yawWmeI.Update((int)Math.round(yawValueDeg));
			relativeBearingWmeI.Update((int)Math.round(relativeBearingValueDeg));
			absRelativeBearingWmeI.Update((int)Math.round(absRelativeBearingValueDeg));
		}
	}

	private class YawFloatWmes implements YawWmes {
		private FloatElement yawWmeF;
		private FloatElement relativeBearingWmeF;
		private FloatElement absRelativeBearingWmeF;

		public void create() {
			yawWmeF = parent.CreateFloatWME("yaw", 0);
			relativeBearingWmeF = parent.CreateFloatWME("relative-bearing", 0);
			absRelativeBearingWmeF = parent.CreateFloatWME("abs-relative-bearing", 0);
		}
		
		public void update(double yawValueDeg, double relativeBearingValueDeg, double absRelativeBearingValueDeg) {
			yawWmeF.Update(yawValueDeg);
			relativeBearingWmeF.Update(relativeBearingValueDeg);
			absRelativeBearingWmeF.Update(absRelativeBearingValueDeg);
		}
	}
	

}
