package edu.umich.visualsoar.misc;
import edu.umich.visualsoar.MainFrame;
import edu.umich.visualsoar.datamap.*;
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


public class FeedbackList extends JList {		
///////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////
	public FeedbackList() {
		addMouseListener(new MouseAdapter() {
			public void mouseClicked(MouseEvent e) {
				if (e.getClickCount() == 2) {
					int index = locationToIndex(e.getPoint());
					ListModel dlm = getModel();
					if (dlm.getElementAt(index) instanceof FeedbackListObject) {
						FeedbackListObject filo = (FeedbackListObject)dlm.getElementAt(index);
            if(!filo.isDataMapObject()) {
  						(filo.getNode()).openRules(MainFrame.getMainFrame(),filo.getLine());
            }
            else {
              // check to see if datamap already opened
              DataMap dm = MainFrame.getMainFrame().getDesktopPane().dmGetDataMap(filo.getDataMapId());

              // Only open a new window if the window does not already exist
              if( dm != null) {
                try {
			            if (dm.isIcon())
				            dm.setIcon(false);
			            dm.setSelected(true);
			            dm.moveToFront();
		            }
		            catch (java.beans.PropertyVetoException pve) {
                  System.err.println("Guess we can't do that");
	              }
              }
              else {
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
	
	class FeedbackCellRenderer extends JLabel implements ListCellRenderer {
		private Border lineBorder = BorderFactory.createLineBorder(Color.red,2),
					   emptyBorder = BorderFactory.createEmptyBorder(2,2,2,2);
		private Color amber;
    private Color green;
					   
		public FeedbackCellRenderer() {
			setOpaque(true);
			amber = Color.orange.darker();
      green = Color.green.darker();
			setFont(new Font("SansSerif",Font.PLAIN,12));
		}
		
		public Component getListCellRendererComponent(JList list,Object value,int index,boolean isSelected,boolean cellHasFocus) {
			setText(value.toString());
			if(isSelected) {
				setForeground(list.getSelectionForeground());
				setBackground(list.getSelectionBackground());
			}
			else {
				if(value instanceof FeedbackListObject) {
          if(((FeedbackListObject)value).isGenerated()) {
            setForeground(green);
          }
          else {
					  if(((FeedbackListObject)value).isError())
						  setForeground(Color.red);
					  else
						  setForeground(amber);
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
