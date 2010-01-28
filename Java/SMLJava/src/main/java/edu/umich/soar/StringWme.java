package edu.umich.soar;

import sml.StringElement;
import sml.Identifier;

public class StringWme
{
    public static StringWme newInstance(Identifier parent, String attr) {
	return new StringWme(parent, attr);
    }
    
    public static StringWme newInstance(Identifier parent, String attr, String val) {
	StringWme temp = new StringWme(parent, attr);
	temp.update(val);
	return temp;
    }
    
    private final Identifier parent;
    private final String attr;
    private StringElement wme;
    
    private StringWme(Identifier parent, String attr) {
	this.parent = parent;
	this.attr = attr;
    }
    
    public void update(String val) {
	if (wme != null) {
	    wme.Update(val);
	} else {
	    wme = parent.CreateStringWME(attr, val);
	}
    }
    
    public void destroy() {
	if (wme != null) {
	    wme.DestroyWME();
	    wme = null;
	}
    }
}
