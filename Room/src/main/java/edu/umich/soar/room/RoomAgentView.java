package edu.umich.soar.room;

import java.awt.BorderLayout;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import javax.swing.BorderFactory;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JToolBar;
import javax.swing.SwingUtilities;
import javax.swing.event.ListSelectionEvent;

import org.flexdock.docking.DockingConstants;
import org.jdesktop.swingx.JXTable;
import org.jdesktop.swingx.decorator.HighlighterFactory;

import edu.umich.soar.room.core.Simulation;
import edu.umich.soar.room.core.events.PlayerAddedEvent;
import edu.umich.soar.room.core.events.PlayerRemovedEvent;
import edu.umich.soar.room.events.SimEvent;
import edu.umich.soar.room.events.SimEventListener;
import edu.umich.soar.room.events.SimEventManager;
import edu.umich.soar.room.map.CommListener;
import edu.umich.soar.room.map.CommMessage;
import edu.umich.soar.room.map.Robot;
import edu.umich.soar.room.selection.SelectionListener;
import edu.umich.soar.room.selection.SelectionManager;
import edu.umich.soar.room.selection.SelectionProvider;
import edu.umich.soar.room.selection.TableSelectionProvider;

public class RoomAgentView extends AbstractAgentView implements SelectionListener {
	private static final long serialVersionUID = 88497381529040370L;
	
	private final Simulation sim;
	private final RobotTableModel model;
    private final JXTable table;
	private final JLabel properties = new JLabel();
	private final TableSelectionProvider selectionProvider;
	private final CommListenerFilter outputListener;
	private JScrollPane commPane = new JScrollPane();
	final JPanel pBottom = new JPanel(new BorderLayout());
	
	private static class CommListenerFilter implements CommListener, SimEventListener {
		private final Map<String, JTextArea> textMap = new HashMap<String, JTextArea>();
		
		public CommListenerFilter(Simulation sim) {
	        final SimEventManager eventManager = sim.getEvents();
			eventManager.addListener(PlayerAddedEvent.class, this);
			eventManager.addListener(PlayerRemovedEvent.class, this);
			
			for (Robot robot : sim.getWorld().getPlayers()) {
				addDestination(robot.getName());
			}
		}
		
		public JTextArea getTextArea(String name) {
			if (name == null) {
				return createTextArea();
			}
			return textMap.get(name);
		}
		
		@Override
		public void onEvent(SimEvent event) {
			if (event instanceof PlayerAddedEvent) {
				Robot robot = ((PlayerAddedEvent) event).getPlayer();
				addDestination(robot.getName());

			} else if (event instanceof PlayerRemovedEvent) {
				Robot robot = ((PlayerRemovedEvent) event).getPlayer();
				textMap.remove(robot.getName());
			}
		}
		
		private JTextArea createTextArea() {
	        JTextArea ta = new JTextArea();
	        ta.setEditable(false);
	        ta.setRows(12);
	        ta.setLineWrap(true);
	        ta.setWrapStyleWord(true);
	        return ta;
		}
		
		private void addDestination(String dest) {
	        textMap.put(dest, createTextArea());
		}
		
		@Override
		public void write(final CommMessage message) {
            SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    synchronized(this) // synchronized on outer.this like the flush() method
                    {
                    	if (message.isBroadcast()) {
                    		for (JTextArea ta : textMap.values()) {
                    			addMessage(message, ta);
                    		}
                    	} else {
                			addMessage(message, textMap.get(message.getFrom()));
                			addMessage(message, textMap.get(message.getDestination()));
                    	}
                    }
                }
            });
		}
		
		private void addMessage(CommMessage message, JTextArea ta) {
			if (ta == null) {
				// warn?
				return;
			}
			ta.append(message.toString());
			ta.append("\n");
			ta.setCaretPosition(ta.getDocument().getLength());
		}

	}

	
    public RoomAgentView(Adaptable app) {
        super("roomAgentView", "Agent View");
        addAction(DockingConstants.PIN_ACTION);
        addAction(DockingConstants.CLOSE_ACTION);

        this.sim = Adaptables.adapt(app, Simulation.class);
        Adaptables.adapt(app, Application.class).getSelectionManager().addListener(this);

        this.model = new RobotTableModel(this.sim);
        this.model.initialize();
        this.table = new JXTable(this.model);
        this.table.setVisibleRowCount(2);
            
        this.table.setShowGrid(false);
        this.table.setHighlighters(HighlighterFactory.createAlternateStriping());
        this.table.setColumnControlVisible(true);

        this.selectionProvider = new TableSelectionProvider(this.table) {
            @Override
            public void valueChanged(ListSelectionEvent e)
            {
                super.valueChanged(e);
                updateRobotProperties();
            }
        };
        
    	final JPanel p = new JPanel(new BorderLayout());
        final JPanel pTop = new JPanel(new BorderLayout());

        JToolBar bar = new JToolBar();
		bar.setFloatable(false);
		bar.add(new AgentControlPanel(app));     
		pTop.add(bar, BorderLayout.NORTH);
		
        final JScrollPane pane = new JScrollPane(table);
        pTop.add(pane, BorderLayout.SOUTH);
        p.add(pTop, BorderLayout.NORTH);

        properties.setBorder(BorderFactory.createTitledBorder("Robot Properties"));
        properties.setHorizontalAlignment(LEFT);
        properties.setVerticalAlignment(TOP);
        properties.setHorizontalTextPosition(JLabel.CENTER);
        properties.setVerticalTextPosition(JLabel.BOTTOM);
        
        pBottom.add(properties, BorderLayout.CENTER);
        
        outputListener = new CommListenerFilter(sim);
        commPane.setViewportView(outputListener.getTextArea(null));
		pBottom.add(commPane, BorderLayout.SOUTH);

        this.sim.getWorld().addCommListener(outputListener);
        
        p.add(pBottom, BorderLayout.CENTER);

        setContentPane(p);
    }
    
    private void updateRobotProperties() {
    	final Robot robot = (Robot)selectionProvider.getSelectedObject();
    	
    	if (robot != null) {
	    	SwingUtilities.invokeLater(new Runnable() {
	    		@Override
	    		public void run() {
	    			final String SPACES = "&nbsp;&nbsp;&nbsp;";
	    			final String BR = "<br />";
	    			StringBuilder sb = new StringBuilder("<html>");
	    			sb.append(" <b>Cell:</b>&nbsp;" + Arrays.toString(robot.getState().getLocation()) + SPACES);
	    			sb.append(" <b>Area:</b>&nbsp;" + robot.getState().getLocationId() + BR);
	    			sb.append(" <b>Location:</b>&nbsp;" + String.format("[%2.1f,%2.1f]", robot.getState().getPose().pos[0], robot.getState().getPose().pos[1]) + BR);
	    			sb.append(" <b>Yaw:</b>&nbsp;" + String.format("%2.1f", robot.getState().getYaw()) + BR);
	    			sb.append(" <b>X&nbsp;Collision:</b>&nbsp;" + robot.getState().isCollisionX() + SPACES);
	    			sb.append(" <b>Y&nbsp;Collision:</b>&nbsp;" + robot.getState().isCollisionY() + BR);
	    			sb.append(" <b>Carried:</b>&nbsp;" + (robot.getState().hasObject() ? robot.getState().getRoomObject() : "-") + SPACES);
	    			sb.append("</html>");
	    			properties.setText(sb.toString());
	    		}
	    	});
    	}
    }

	@Override
	public void refresh() {
        this.table.repaint();
        this.table.packAll();

        updateRobotProperties();
	}
	
    /* (non-Javadoc)
     * @see org.jsoar.debugger.AbstractAdaptableView#getAdapter(java.lang.Class)
     */
    @Override
    public Object getAdapter(Class<?> klass)
    {
        if(SelectionProvider.class.equals(klass))
        {
            return selectionProvider;
        }
        else if(RobotTableModel.class.equals(klass))
        {
            return model;
        }
        return super.getAdapter(klass);
    }

	public void selectRobot(Robot robot) {
		int index = model.getPlayers().indexOf(robot);
        if(index != -1) {
            table.getSelectionModel().setSelectionInterval(index, index);
            table.scrollRowToVisible(index);
        }
	}

	@Override
	public void selectionChanged(SelectionManager manager) {
    	Robot robot = null;
		if (manager.getSelectedObject() instanceof Robot) {
			robot = (Robot)manager.getSelectedObject();
			commPane.setViewportView(outputListener.getTextArea(robot.getName()));
		}
		
    	if (this.selectionProvider.isActive()) {
			return;
		}
		
    	if (robot != null) {
    		selectRobot(robot);
    		updateRobotProperties();
    	}
	}
}
