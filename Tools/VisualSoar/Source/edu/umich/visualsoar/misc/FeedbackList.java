package edu.umich.visualsoar.misc;
import edu.umich.visualsoar.datamap.*;
import edu.umich.visualsoar.MainFrame;
import java.util.*;
import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;

/**
 * A class that is the FeedbackList window in the MainFrame
 * its job is to provide various forms of messages from 
 * Visual Soar to the user
 * @author Brad Jones
 * @version 0.5a 4 Aug 1999
 */


public class FeedbackList extends JList 
{
///////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////
    DefaultListModel dlm = new DefaultListModel();

///////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////
    public FeedbackList()
    {
        setModel(dlm);
        
        addMouseListener(
            new MouseAdapter() 
            {
                public void mouseClicked(MouseEvent e) 
                {
                    if (e.getClickCount() == 2) 
                    {
                        int index = locationToIndex(e.getPoint());
                        ListModel dlm = getModel();
                        if (dlm.getElementAt(index) instanceof FeedbackListObject) 
                        {
                            FeedbackListObject filo = (FeedbackListObject)dlm.getElementAt(index);
                            if(!filo.isDataMapObject()) 
                            {
                                filo.DisplayFile();
                            }
                            else 
                            {
                                // check to see if datamap already opened
                                DataMap dm = MainFrame.getMainFrame().getDesktopPane().dmGetDataMap(filo.getDataMapId());
                                
                                // Only open a new window if the window does not already exist
                                if( dm != null) 
                                {
                                    try 
                                    {
                                        if (dm.isIcon())
                                        dm.setIcon(false);
                                        dm.setSelected(true);
                                        dm.moveToFront();
                                    }
                                    catch (java.beans.PropertyVetoException pve) 
                                    {
                                        System.err.println("Guess we can't do that");
                                    }
                                }
                                else 
                                {
                                    dm = filo.createDataMap(MainFrame.getMainFrame().getOperatorWindow().getDatamap());
                                    MainFrame.getMainFrame().getDesktopPane().dmAddDataMap(filo.getDataMapId(), dm);
                                    dm.setVisible(true);
                                    MainFrame.getMainFrame().addDataMap(dm);
                                }
                                // Highlight the proper node within the datamap
                                dm.selectEdge(filo.getEdge());
                            }
                        }
                    }
                }
            });
        
        setCellRenderer(new FeedbackCellRenderer());

    }

    /**
     * Override the default implementation.  We want to update the
     * DefaultListModel class we're using here.
     *
     * BUG?:  Can this be done more efficiently???
     */
    public void setListData(Vector v)
    {
        dlm.removeAllElements();
        int vecSize = v.size();
        dlm.ensureCapacity(vecSize*2);
        for(int i = 0; i < vecSize; i++)
        {
            dlm.add(i, v.get(i));
        }

        //Make sure no-one's been fiddling with the list model
        if (getModel() != dlm)
        {
            setModel(dlm);
        }
    }//setListData
    
    /**
     * Remove all the data in the list.
     *
     */
    public void clearListData()
    {
        dlm.removeAllElements();
        
        //Make sure no-one's been fiddling with the list model
        if (getModel() != dlm)
        {
            setModel(dlm);
        }
    }//clearListData
    
    /**
     * Add an item to the list
     *
     */
    public void insertElementAt(Object obj, int index)
    {
        dlm.insertElementAt(obj, index);
        
        //Make sure no-one's been fiddling with the list model
        if (getModel() != dlm)
        {
            setModel(dlm);
        }
    }//setListData

    /**
     * Append a vector to the existing list content
     *
     */
    public void appendListData(Vector v)
    {
        dlm.ensureCapacity((v.size() + dlm.size())*2);
        
        for(int i = dlm.size(), j = 0;
            j < v.size();
            i++, j++)
        {
            dlm.add(i, v.get(j));
        }

        //Make sure no-one's been fiddling with the list model
        if (getModel() != dlm)
        {
            setModel(dlm);
        }
    }//setListData


    class FeedbackCellRenderer extends JLabel implements ListCellRenderer 
    {
        private Border lineBorder = BorderFactory.createLineBorder(Color.red,2),
            emptyBorder = BorderFactory.createEmptyBorder(2,2,2,2);
        private Color textColor;
        private Color floTextColor;
                       
        public FeedbackCellRenderer() 
        {
            setOpaque(true);
            textColor = Color.blue.darker();
            floTextColor = Color.green.darker().darker();
            setFont(new Font("SansSerif",Font.PLAIN,12));
        }
        
        public Component getListCellRendererComponent(JList list,Object value,
                                                      int index,
                                                      boolean isSelected,
                                                      boolean cellHasFocus) 
        {
            setText(value.toString());
            if(isSelected) 
            {
                setForeground(list.getSelectionForeground());
                setBackground(list.getSelectionBackground());
            }
            else 
            {
                if(value instanceof FeedbackListObject) 
                {
                    if(((FeedbackListObject)value).isGenerated()) 
                    {
                        setForeground(floTextColor);
                    }
                    else 
                    {
                        if(((FeedbackListObject)value).isError())
                        setForeground(Color.red);
                        else
                        setForeground(textColor);
                    }
                }
                else
                setForeground(list.getForeground());
                setBackground(list.getBackground());
            }
            
            setBorder(emptyBorder);
            return this;
        }
    }
}
