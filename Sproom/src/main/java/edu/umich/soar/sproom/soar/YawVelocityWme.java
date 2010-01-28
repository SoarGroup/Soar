package edu.umich.soar.sproom.soar;

import sml.Identifier;
import edu.umich.soar.FloatWme;
import edu.umich.soar.sproom.command.CommandConfig;

public class YawVelocityWme {
    public static YawVelocityWme newInstance(Identifier parent, String attr) {
    	return new YawVelocityWme(parent, attr);
    }
    
    public static YawVelocityWme newInstance(Identifier parent, String attr, double radiansPerSec) {
    	YawVelocityWme temp = new YawVelocityWme(parent, attr);
    	temp.update(radiansPerSec);
    	return temp;
    }
    
	private final CommandConfig c = CommandConfig.CONFIG;
    private final FloatWme fwme;
    
    private YawVelocityWme(Identifier parent, String attr) {
    	fwme = FloatWme.newInstance(parent, attr);
    }
    
    public void update(double radiansPerSec) {
    	double view = c.angleToView(radiansPerSec);
   		fwme.update(view);
    }
    
    public void destroy() {
		fwme.destroy();
    }
}
