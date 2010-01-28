package edu.umich.soar;

import sml.FloatElement;
import sml.Identifier;

public class FloatWme
{
    public static FloatWme newInstance(Identifier parent, String attr) {
	return new FloatWme(parent, attr);
    }
    
    public static FloatWme newInstance(Identifier parent, String attr, double val) {
	FloatWme temp = new FloatWme(parent, attr);
	temp.update(val);
	return temp;
    }
    
    private final Identifier parent;
    private final String attr;
    private FloatElement wme;
    
    private FloatWme(Identifier parent, String attr) {
	this.parent = parent;
	this.attr = attr;
    }
    
    public void update(double val) {
	if (wme != null) {
	    wme.Update(val);
	} else {
	    wme = parent.CreateFloatWME(attr, val);
	}
    }
    
    public void destroy() {
	if (wme != null) {
	    wme.DestroyWME();
	    wme = null;
	}
    }
}
