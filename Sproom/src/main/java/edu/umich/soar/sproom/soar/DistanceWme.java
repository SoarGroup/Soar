package edu.umich.soar.sproom.soar;

import sml.Identifier;
import edu.umich.soar.FloatWme;
import edu.umich.soar.sproom.command.CommandConfig;

/**
 * Input link working memory element wrapper for distance wmes, automatically 
 * converting units.
 * 
 * @author voigtjr@gmail.com
 */
public class DistanceWme {
    public static DistanceWme newInstance(Identifier parent, String attr) {
    	return new DistanceWme(parent, attr);
    }
    
    public static DistanceWme newInstance(Identifier parent, String attr, double distance) {
    	DistanceWme temp = new DistanceWme(parent, attr);
    	temp.update(distance);
    	return temp;
    }
    
	private final CommandConfig c = CommandConfig.CONFIG;
    private final FloatWme wme;
    
    private DistanceWme(Identifier parent, String attr) {
    	wme = FloatWme.newInstance(parent, attr);
    }
    
    public void update(double distance) {
   		wme.update(c.lengthToView(distance));
    }
    
    public void destroy() {
		wme.destroy();
    }
}
