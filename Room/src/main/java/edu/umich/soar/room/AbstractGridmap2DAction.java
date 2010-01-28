package edu.umich.soar.room;

import javax.swing.AbstractAction;
import javax.swing.Icon;
import javax.swing.KeyStroke;

/**
 * @author ray
 */
public abstract class AbstractGridmap2DAction extends AbstractAction
{
    private static final long serialVersionUID = -8312655935200259621L;
    
    private final ActionManager manager;
    
    public AbstractGridmap2DAction(String label)
    {
        super(label);
        this.manager = null;
    }

    public AbstractGridmap2DAction(String label, Icon icon)
    {
        super(label, icon);
        this.manager = null;
    }
    
    public AbstractGridmap2DAction(ActionManager manager, String label)
    {
        super(label);
        this.manager = manager;
        this.manager.addAction(this);
    }

    public AbstractGridmap2DAction(ActionManager manager, String label, Icon icon)
    {
        super(label, icon);
        this.manager = manager;
        this.manager.addAction(this);
    }
    
    public AbstractGridmap2DAction(ActionManager manager, String label, Class<?> klass, boolean adapt)
    {
        super(label);
        this.manager = manager;
        manager.addObjectAction(this, klass, adapt);
    }

    public AbstractGridmap2DAction(ActionManager manager, String label, Icon icon, Class<?> klass, boolean adapt)
    {
        super(label, icon);
        this.manager = manager;
        manager.addObjectAction(this, klass, adapt);
    }
    
    public void setToolTip(String tip)
    {
        this.putValue(SHORT_DESCRIPTION, tip);
    }
    
    public void setAcceleratorKey(KeyStroke key)
    {
        this.putValue(ACCELERATOR_KEY, key);
    }
    
    public void setLabel(String label)
    {
        this.putValue(NAME, label);
    }

    public abstract void update();
    
    public String getId()
    {
        return getClass().getCanonicalName();
    }
    
    public ActionManager getActions()
    {
        return manager;
    }
    
    public Application getApplication()
    {
        return manager != null ? manager.getApplication() : null;
    }
    
}
