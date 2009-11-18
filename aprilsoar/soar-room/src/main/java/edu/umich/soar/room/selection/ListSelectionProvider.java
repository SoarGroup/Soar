/*
 * Copyright (c) 2008  Dave Ray <daveray@gmail.com>
 *
 * Created on Oct 23, 2008
 */
package edu.umich.soar.room.selection;

import java.util.ArrayList;
import java.util.List;

import javax.swing.JList;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

/**
 * Generic selection provider implementation for a JList
 * 
 * @author ray
 */
public class ListSelectionProvider implements SelectionProvider, ListSelectionListener
{
    private final JList list;
    private SelectionManager manager;
    
    /**
     * Construct a selection provider for a particular list
     * @param list the list to connect to
     */
    public ListSelectionProvider(JList list)
    {
        this.list = list;
        
    }

    /* (non-Javadoc)
     * @see org.jsoar.debugger.selection.SelectionProvider#activate(org.jsoar.debugger.selection.SelectionManager)
     */
    @Override
    public void activate(SelectionManager manager)
    {
        this.manager = manager;
        this.list.addListSelectionListener(this);
    }

    /* (non-Javadoc)
     * @see org.jsoar.debugger.selection.SelectionProvider#deactivate()
     */
    @Override
    public void deactivate()
    {
        this.manager = null;
        this.list.removeListSelectionListener(this);
    }

    /* (non-Javadoc)
     * @see org.jsoar.debugger.selection.SelectionProvider#getSelectedObject()
     */
    @Override
    public Object getSelectedObject()
    {
        return this.list.getSelectedValue();
    }

    /* (non-Javadoc)
     * @see org.jsoar.debugger.selection.SelectionProvider#getSelection()
     */
    @Override
    public List<Object> getSelection()
    {
        List<Object> result = new ArrayList<Object>();
        for(int i : this.list.getSelectedIndices())
        {
            result.add(this.list.getModel().getElementAt(i));
        }
        return result;
    }

    /* (non-Javadoc)
     * @see javax.swing.event.ListSelectionListener#valueChanged(javax.swing.event.ListSelectionEvent)
     */
    @Override
    public void valueChanged(ListSelectionEvent e)
    {
        if(this.manager != null)
        {
            this.manager.fireSelectionChanged();
        }
    }

}
