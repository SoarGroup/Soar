package edu.umich.soar;

import sml.Identifier;
import sml.IntElement;

public class IntWme
{
    public static IntWme newInstance(Identifier parent, String attr) {
	return new IntWme(parent, attr);
    }
    
    public static IntWme newInstance(Identifier parent, String attr, int val) {
	IntWme temp = new IntWme(parent, attr);
	temp.update(val);
	return temp;
    }
    
    private final Identifier parent;
    private final String attr;
    private IntElement wme;
    
    private IntWme(Identifier parent, String attr) {
	this.parent = parent;
	this.attr = attr;
    }
    
    public void update(int val) {
	if (wme != null) {
	    wme.Update(val);
	} else {
	    wme = parent.CreateIntWME(attr, val);
	}
    }
    
    public void destroy() {
	if (wme != null) {
	    wme.DestroyWME();
	    wme = null;
	}
    }
}
