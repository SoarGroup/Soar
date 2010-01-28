/*
 * Copyright (c) 2009 Dave Ray <daveray@gmail.com>
 *
 * Created on Jun 10, 2009
 */
package edu.umich.soar.room.selection;

import java.util.ArrayList;
import java.util.List;

import javax.swing.JTable;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

/**
 * @author ray
 */
public class TableSelectionProvider implements SelectionProvider, ListSelectionListener
{
    private final JTable table;
    private SelectionManager manager;
    
    
    /**
     * @param table
     */
    public TableSelectionProvider(JTable table)
    {
        this.table = table;
    }

    public boolean isActive()
    {
        return manager != null;
    }
    
    /* (non-Javadoc)
     * @see org.jsoar.debugger.selection.SelectionProvider#activate(org.jsoar.debugger.selection.SelectionManager)
     */
    @Override
    public void activate(SelectionManager manager)
    {
        this.manager = manager;
        this.table.getSelectionModel().addListSelectionListener(this);
    }

    /* (non-Javadoc)
     * @see org.jsoar.debugger.selection.SelectionProvider#deactivate()
     */
    @Override
    public void deactivate()
    {
        this.manager = null;
        this.table.getSelectionModel().removeListSelectionListener(this);
    }

    /* (non-Javadoc)
     * @see org.jsoar.debugger.selection.SelectionProvider#getSelectedObject()
     */
    @Override
    public Object getSelectedObject()
    {
        List<Object> s = getSelection();
        return !s.isEmpty() ? s.get(0) : null;
    }

    /* (non-Javadoc)
     * @see org.jsoar.debugger.selection.SelectionProvider#getSelection()
     */
    @Override
    public List<Object> getSelection()
    {
        List<Object> result = new ArrayList<Object>();
        for(int row : table.getSelectedRows())
        {
            final Object value = getValueAt(row);
            result.add(value);
        }
        return result;
    }

    /* (non-Javadoc)
     * @see javax.swing.event.ListSelectionListener#valueChanged(javax.swing.event.ListSelectionEvent)
     */
    @Override
    public void valueChanged(ListSelectionEvent e)
    {
        if(manager != null)
        {
            manager.fireSelectionChanged();
        }
    }
    
    protected Object getValueAt(int row)
    {
        return table.getValueAt(row, 0);
    }
    
}
