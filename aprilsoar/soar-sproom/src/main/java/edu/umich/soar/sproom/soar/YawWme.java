package edu.umich.soar.sproom.soar;

import sml.Identifier;
import edu.umich.soar.FloatWme;
import edu.umich.soar.IntWme;
import edu.umich.soar.sproom.command.CommandConfig;

public class YawWme {
    public static YawWme newInstance(Identifier parent, String attr) {
    	return new YawWme(parent, attr);
    }
    
    public static YawWme newInstance(Identifier parent, String attr, double radians) {
    	YawWme temp = new YawWme(parent, attr);
    	temp.update(radians);
    	return temp;
    }
    
    private final FloatWme fwme;
    private final IntWme iwme;
    
    private YawWme(Identifier parent, String attr) {
    	iwme = IntWme.newInstance(parent, attr);
    	fwme = FloatWme.newInstance(parent, attr);
    }
    
    public void update(double radians) {
    	CommandConfig c = CommandConfig.CONFIG;
    	double view = c.angleToView(radians);
    	
    	switch (CommandConfig.CONFIG.getAngleResolution()) {
    	case FLOAT:
    		iwme.destroy();
   			fwme.update(view);
    		break;
    		
    	case INT:
    		fwme.destroy();
    		iwme.update((int)Math.round(view));
    		break;
    	}
    }
    
    public void destroy() {
		iwme.destroy();
		fwme.destroy();
    }
}
