package edu.umich.soar;

import sml.Identifier;
import sml.StringElement;

/**
 * Wrapper for String valued working memory elements.
 * 
 * @author voigtjr
 * 
 */
public class StringWme implements Wme
{
    /**
     * Create working memory element instance with no value. The value will be
     * set (and underlying SML Wme created) on first update.
     * 
     * @param parent
     *            Parent identifier
     * @param attr
     *            Attribute
     * @return Wme ready for update
     */
    public static StringWme newInstance(Identifier parent, String attr)
    {
        return new StringWme(parent, attr);
    }

    /**
     * Create working memory element instance with known initial value.
     * 
     * @param parent
     *            Parent identifier
     * @param attr
     *            Attribute
     * @param val
     *            Initial value
     * @return Wme ready for update
     */
    public static StringWme newInstance(Identifier parent, String attr,
            String val)
    {
        StringWme temp = new StringWme(parent, attr);
        temp.update(val);
        return temp;
    }

    private final Identifier parent;

    private final String attr;

    private StringElement wme;

    private StringWme(Identifier parent, String attr)
    {
        this.parent = parent;
        this.attr = attr;
    }

    /**
     * <p>
     * Update to possibly new value. See Agent.BlinkIfNoChange()
     * 
     * @param val
     *            The new value
     * @throws NullPointerException
     *             if val is null
     */
    public void update(String val)
    {
        if (val == null)
            throw new NullPointerException();
        
        if (wme != null)
            wme.Update(val);
        else
            wme = parent.CreateStringWME(attr, val);
    }

    /**
     * Get the current value.
     * 
     * @throws IllegalStateException
     *             if destroyed or value was not set
     * @return Current value
     */
    public String getValue()
    {
        if (wme == null)
            throw new IllegalStateException();
        return wme.GetValue();
    }

    @Override
    public void destroy()
    {
        if (wme != null)
        {
            wme.DestroyWME();
            wme = null;
        }
    }

    @Override
    public String toString()
    {
        if (wme == null)
            return "(invalid: " + attr + ")";
        
        StringBuilder sb = new StringBuilder("(");
        sb.append(wme.GetIdentifierName());
        sb.append(" ^");
        sb.append(attr);
        sb.append(" ");
        sb.append(wme.GetValueAsString());
        sb.append(")");
        return sb.toString();
    }
}
