package edu.umich.soar;

import sml.FloatElement;
import sml.Identifier;

/**
 * Wrapper for float valued working memory elements.
 * 
 * @author voigtjr
 *
 */
public class FloatWme implements Wme
{
    /**
     * Create working memory element instance with no value. The value will be set
     * (and underlying SML Wme created) on first update.
     * @param parent Parent identifier
     * @param attr Attribute
     * @return Wme ready for update
     */
    public static FloatWme newInstance(Identifier parent, String attr) {
	return new FloatWme(parent, attr);
    }
    
    /**
     * Create working memory element instance with known initial value.
     * @param parent Parent identifier
     * @param attr Attribute
     * @param val Initial value
     * @return Wme ready for update
     */
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
    
    /**
     * Update to possibly new value. See Agent.BlinkIfNoChange()
     * @param val The new value
     */
    public void update(double val) {
	if (wme != null) {
	    wme.Update(val);
	} else {
	    wme = parent.CreateFloatWME(attr, val);
	}
    }
    
    /**
     * Get the current value.
     * @throws IllegalStateException if destroyed or value was not set
     * @return Current value
     */
    public double getValue() {
	if (wme == null) 
	    throw new IllegalStateException();
	return wme.GetValue();
    }
    
    @Override
    public void destroy() {
	if (wme != null) {
	    wme.DestroyWME();
	    wme = null;
	}
    }
}
