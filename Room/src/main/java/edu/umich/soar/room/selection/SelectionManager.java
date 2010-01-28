/*
 * Copyright (c) 2008  Dave Ray <daveray@gmail.com>
 *
 * Created on Oct 23, 2008
 */
package edu.umich.soar.room.selection;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * @author ray
 */
public class SelectionManager
{
    private static final SelectionProvider NULL_PROVIDER = new NullProvider();
    
    private List<SelectionListener> listeners = new CopyOnWriteArrayList<SelectionListener>();
    private SelectionProvider provider = NULL_PROVIDER;
    
    public void addListener(SelectionListener listener)
    {
        listeners.add(listener);
    }
    
    public void removeListener(SelectionListener listener)
    {
        listeners.remove(listener);
    }
    
    public void setSelectionProvider(SelectionProvider provider)
    {
        this.provider.deactivate();
        this.provider = provider != null ? provider : NULL_PROVIDER;
        this.provider.activate(this);
        fireSelectionChanged();
    }

    /**
     * 
     */
    public void fireSelectionChanged()
    {
        for(SelectionListener listener : listeners)
        {
            listener.selectionChanged(this);
        }
    }
    
    public Object getSelectedObject()
    {
        return provider.getSelectedObject();
    }

    public List<Object> getSelection()
    {
        return provider.getSelection();
    }
    
    
    private static class NullProvider implements SelectionProvider
    {
        /* (non-Javadoc)
         * @see org.jsoar.debugger.selection.SelectionProvider#activate(org.jsoar.debugger.selection.SelectionManager)
         */
        @Override
        public void activate(SelectionManager manager)
        {
        }

        /* (non-Javadoc)
         * @see org.jsoar.debugger.selection.SelectionProvider#deactivate()
         */
        @Override
        public void deactivate()
        {
        }

        /* (non-Javadoc)
         * @see org.jsoar.debugger.selection.SelectionProvider#getSelection()
         */
        @Override
        public List<Object> getSelection()
        {
            return new ArrayList<Object>();
        }

        /* (non-Javadoc)
         * @see org.jsoar.debugger.selection.SelectionProvider#getSelectedObject()
         */
        @Override
        public Object getSelectedObject()
        {
            return null;
        }
        
    }
}
