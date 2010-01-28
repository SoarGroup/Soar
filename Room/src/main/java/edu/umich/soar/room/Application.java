package edu.umich.soar.room;

import java.awt.BorderLayout;
import java.awt.GraphicsEnvironment;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.SynchronousQueue;
import java.util.prefs.Preferences;

import javax.swing.JCheckBoxMenuItem;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;

import org.flexdock.docking.Dockable;
import org.flexdock.docking.DockingConstants;
import org.flexdock.docking.DockingManager;
import org.flexdock.docking.activation.ActiveDockableTracker;
import org.flexdock.view.Viewport;

import edu.umich.soar.sps.control.robot.SendMessagesInterface;
import edu.umich.soar.room.config.SimConfig;
import edu.umich.soar.room.core.Simulation;
import edu.umich.soar.room.core.events.AfterTickEvent;
import edu.umich.soar.room.core.events.PlayerAddedEvent;
import edu.umich.soar.room.core.events.PlayerRemovedEvent;
import edu.umich.soar.room.core.events.ResetEvent;
import edu.umich.soar.room.core.events.StopEvent;
import edu.umich.soar.room.events.SimEvent;
import edu.umich.soar.room.events.SimEventListener;
import edu.umich.soar.room.selection.SelectionManager;
import edu.umich.soar.room.selection.SelectionProvider;

public class Application extends JPanel implements Adaptable {
	private static final long serialVersionUID = 4313201967156814057L;

	public static final Preferences PREFERENCES = Preferences.userRoot().node("edu/umich/RoomEnvironment");

	public static void main(final String[] args) {
		try {
			final SimConfig config = args.length > 0 ? SimConfig.getInstance(args[0]) : SimConfig.newInstance();
			final Simulation sim = new Simulation();
			final BlockingQueue<Boolean> doneQueue = new SynchronousQueue<Boolean>();
			
			if (GraphicsEnvironment.isHeadless() || config.generalConfig().headless) {
				sim.initialize(config);
				sim.getEvents().addListener(StopEvent.class, new SimEventListener() {
					@Override
					public void onEvent(SimEvent event) {
						try {
							doneQueue.put(Boolean.TRUE);
						} catch (InterruptedException e) {
							// TODO handle correctly
							e.printStackTrace();
						}
					}
				});
				
				sim.addInitialPlayers();
				sim.run(Simulation.RUN_FOREVER, Double.MAX_VALUE);
				
				try {
					doneQueue.take();
				} catch (InterruptedException e) {
					// TODO handle correctly
					e.printStackTrace();
				}
				sim.shutdown();
			} else {
				try {
					// Use the look and feel of the system we're running on rather
					// than Java. If an error occurs, we proceed normally using
					// whatever L&F we get.
					UIManager.setLookAndFeel(UIManager
							.getSystemLookAndFeelClassName());
				} catch (UnsupportedLookAndFeelException e) {
				} catch (ClassNotFoundException e) {
				} catch (InstantiationException e) {
				} catch (IllegalAccessException e) {
				}
				
				SwingUtilities.invokeLater(new Runnable() {
					@Override
					public void run() {
						initialize(args, sim, config);
					}
				});
			}
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			return;
		}
	}
	
	public static Application initialize(String[] args, Simulation sim, SimConfig config) {
		DockingManager.setFloatingEnabled(true);
		
		sim.initialize(config);
		
		Application app = new Application(sim);
		final JFrame frame = new JFrame();
		frame.setContentPane(app);
		
		app.initialize(frame);
		frame.setVisible(true);
		return app;
	}

	private JFrame frame;
    private final Viewport viewport = new Viewport();
    private final SelectionManager selectionManager = new SelectionManager();
	private final ActionManager actionManager = new ActionManager(this);
    private final List<AbstractAdaptableView> views = new ArrayList<AbstractAdaptableView>();
    private final Simulation sim;
    private final List<SimEventListener> simEventListeners = new ArrayList<SimEventListener>();
    private PropertyChangeListener dockTrackerListener = null;

    private Application(Simulation sim) {
    	super(new BorderLayout());
    	
		this.sim = sim;
    }
    
    private void initialize(JFrame frame) {
		this.frame = frame;
		
        frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        frame.addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e)
            {
                exit();
            }});

        frame.setSize(1000, 930);
		frame.setTitle("Room Environment");
		
		initActions();
		//initToolbar();

		add(viewport, BorderLayout.CENTER);

		initViews();
		initMenuBar();
		
        // Track selection to active view
        ActiveDockableTracker.getTracker(frame).addPropertyChangeListener(
                dockTrackerListener = new PropertyChangeListener() {

            @Override
            public void propertyChange(PropertyChangeEvent evt)
            {
                Dockable newDockable = (Dockable) evt.getNewValue();
                SelectionProvider provider = Adaptables.adapt(newDockable, SelectionProvider.class);
                if(provider != null)
                {
                    selectionManager.setSelectionProvider(provider);
                }
            }});

        sim.getEvents().addListener(AfterTickEvent.class, saveListener(new UpdateListener()));
        sim.getEvents().addListener(StopEvent.class, saveListener(new UpdateListener()));
        sim.getEvents().addListener(ResetEvent.class, saveListener(new UpdateListener()));
        sim.getEvents().addListener(PlayerAddedEvent.class, saveListener(new UpdateListener()));
        sim.getEvents().addListener(PlayerRemovedEvent.class, saveListener(new UpdateListener()));
        
        sim.addInitialPlayers();
        
        update();
	}
    
    private class UpdateListener implements SimEventListener {
    	@Override
    	public void onEvent(SimEvent event) {
    		fireUpdate();
    	}
    }
    
    void fireUpdate() {
    	SwingUtilities.invokeLater(new Runnable() {
    		@Override
    		public void run() {
    			update();
    		}
    	});
    }
    
    private void update()
    {
        updateActionsAndStatus();
        
        List<Refreshable> refreshables = Adaptables.adaptCollection(views, Refreshable.class);
        for(Refreshable r : refreshables)
        {
            r.refresh();
        }
    }
    
    private SimEventListener saveListener(SimEventListener listener)
    {
        simEventListeners.add(listener);
        return listener;
    }
    
//    private void initToolbar()
//    {
//        JToolBar bar = new JToolBar();
//        bar.setFloatable(false);
//        
//        bar.add(new RunControlPanel(this));
//        bar.add(new AgentControlPanel(this));
//        
//        add(bar, BorderLayout.NORTH);
//    }
    
	private void initActions() {
		//new RunAction(actionManager);
		//new StepAction(actionManager);
		new StopAction(actionManager);
		new ResetAction(actionManager);
		new GoAction(actionManager);
		
		new ExitAction(actionManager);
		new ToggleBreadcrumbsAction(actionManager);
		new ClearBreadcrumbsAction(actionManager);
		
		new CreatePlayerAction(actionManager);
		new ClonePlayerAction(actionManager);
		new RemovePlayerAction(actionManager);
	}

	private void initMenuBar() {
		JMenuBar menubar = new JMenuBar();

		{
			JMenu fileMenu = new JMenu("File");
			fileMenu.add(actionManager.getAction(ExitAction.class));
			menubar.add(fileMenu);
		}
		{
			JMenu viewMenu = new JMenu("View");
			
			viewMenu.add(new JCheckBoxMenuItem(actionManager.getAction(ToggleBreadcrumbsAction.class)));
			viewMenu.add(actionManager.getAction(ClearBreadcrumbsAction.class));
			
//			viewMenu.add(new JSeparator());
//			
//			for (AbstractAdaptableView view : views) {
//				viewMenu.add(new RestoreViewAction(view));
//			}

			menubar.add(viewMenu);
		}		
		
		frame.setJMenuBar(menubar);
	}
	
	private void initViews() {
		final WorldView worldView = addView(new WorldView(this));
		//final World3DView worldView = addView(new World3DView(this));
		viewport.dock(worldView);
		
		final AbstractAdaptableView agentView = addView(new RoomAgentView(this));
		worldView.dock(agentView, DockingConstants.EAST_REGION, 0.63f);

		final AbstractAdaptableView simulationControlView = addView(new SimulationControlView(this));
		agentView.dock(simulationControlView, DockingConstants.SOUTH_REGION, 0.62f);

		final AbstractAdaptableView logView = addView(new LogView(this));
		worldView.dock(logView, DockingConstants.SOUTH_REGION, 0.73f);
		
		final AbstractAdaptableView commView = addView(new CommView(this));
		logView.dock(commView);
		
		final AbstractAdaptableView operatorCommView = addView(new OperatorCommView(this));
		logView.dock(operatorCommView);
		
		operatorCommView.setActive(true);
	}
	
    private <T extends AbstractAdaptableView> T addView(T view)
    {
        views.add(view);
        return view;
    }
    
	void exit() {
		detatch();
		frame.dispose();
		sim.shutdown();
	}
	
	public void detatch() {
        // clean up dock property listener
        ActiveDockableTracker.getTracker(frame).removePropertyChangeListener(dockTrackerListener);
        dockTrackerListener = null;
        
//        // clean up soar prop listeners
//        for(PropertyListenerHandle<?> listener : propertyListeners)
//        {
//            listener.removeListener();
//        }
        
        // clean up soar event listener
        for(SimEventListener listener : simEventListeners)
        {
            sim.getEvents().removeListener(null, listener);
        }
        simEventListeners.clear();

        for(Disposable d : Adaptables.adaptCollection(views, Disposable.class))
        {
            d.dispose();
        }
        views.clear();

        if(frame.isVisible())
        {
            frame.setVisible(false);
        }
	}

	@Override
	public Object getAdapter(Class<?> klass) {
        if(klass.equals(Simulation.class)) {
        	return sim;
        }
        if(klass.equals(ActionManager.class)) {
        	return actionManager;
        }
        if(klass.equals(SelectionManager.class))
        {
            return selectionManager;
        }
        if(klass.equals(SendMessagesInterface.class))
        {
            return sim.getSendMessageInterface();
        }
        Object o = Adaptables.findAdapter(views, klass);
        if(o != null) {
            return o;
        }
		return null;
	}
	
    public void updateActionsAndStatus()
    {
        if(SwingUtilities.isEventDispatchThread())
        {
            actionManager.updateActions();
            //status.refresh(false);
        }
        else
        {
            SwingUtilities.invokeLater(new Runnable() { public void run() {
                updateActionsAndStatus();
            } });
        }
    }
    
    public Simulation getSim() {
    	return sim;
    }

	public SelectionManager getSelectionManager() {
        return selectionManager;
	}

	public void createPlayer() {
		CreateRobotDialog cpd = new CreateRobotDialog(frame, sim);
		cpd.setVisible(true);
	}
	
	void doRunForever(double timeScale) {
		sim.run(Simulation.RUN_FOREVER, timeScale);
	}
	
	void doRunTick(int quantity, double timeScale) {
		sim.run(quantity, timeScale);
	}
	
}
