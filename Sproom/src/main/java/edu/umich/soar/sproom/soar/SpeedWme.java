package edu.umich.soar.sproom.soar;

import sml.Identifier;
import edu.umich.soar.FloatWme;
import edu.umich.soar.sproom.command.CommandConfig;

public class SpeedWme {
    public static SpeedWme newInstance(Identifier parent, String attr) {
    	return new SpeedWme(parent, attr);
    }
    
    public static SpeedWme newInstance(Identifier parent, String attr, double speed) {
    	SpeedWme temp = new SpeedWme(parent, attr);
    	temp.update(speed);
    	return temp;
    }
    
	private final CommandConfig c = CommandConfig.CONFIG;
    private final FloatWme wme;
    
    private SpeedWme(Identifier parent, String attr) {
    	wme = FloatWme.newInstance(parent, attr);
    }
    
    public void update(double speed) {
   		wme.update(c.speedToView(speed));
    }
    
    public void destroy() {
		wme.destroy();
    }
}
