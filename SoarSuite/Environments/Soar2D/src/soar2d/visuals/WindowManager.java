package soar2d.visuals;

import java.util.logging.Logger;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.world.Cell;

public class WindowManager {
	private static Logger logger = Logger.getLogger("soar2d");
	public static final String kColors[] = { "red", "blue", "purple", "yellow", "orange", "black", "green" };

	public static Color white = null;
	public static Color blue = null;
	public static Color red = null;
	public static Color widget_background = null;
	public static Color yellow = null;
	public static Color orange = null;
	public static Color black = null;
	public static Color green = null;
	public static Color purple = null;

	protected Display display;
	protected Shell shell;
	final Label foodCount;
	final Label scoreCount;
	final SimulationButtons simButtons;
	final MapButtons mapButtons;
	final VisualWorld visualWorld;
	final AgentDisplay agentDisplay;
	final Group worldGroup;
	Configuration config = Soar2D.config;
	
	public static final int kMainMapCellSize = 20;
	public static final String kFoodRemaining = "Food remaining: ";
	public static final String kScoreRemaining = "Points remaining: ";

	private static void initColors(Display d) {
	    white = d.getSystemColor(SWT.COLOR_WHITE);
		widget_background = d.getSystemColor(SWT.COLOR_WIDGET_BACKGROUND);
		blue = d.getSystemColor(SWT.COLOR_BLUE);
		red = d.getSystemColor(SWT.COLOR_RED);
		yellow = d.getSystemColor(SWT.COLOR_YELLOW);
		green = d.getSystemColor(SWT.COLOR_GREEN);
		purple = d.getSystemColor(SWT.COLOR_DARK_MAGENTA);
		orange = new Color(d, 255, 127, 0);
		black = d.getSystemColor(SWT.COLOR_BLACK);
	}
	
	public static Color getColor(String color) {
		if (color.equalsIgnoreCase("white")) {
			return white;
		}
		if (color.equalsIgnoreCase("blue")) {
			return blue;
		}
		if (color.equalsIgnoreCase("red")) {
			return red;
		}
		if (color.equalsIgnoreCase("yellow")) {
			return yellow;
		}
		if (color.equalsIgnoreCase("green")) {
			return green;
		}
		if (color.equalsIgnoreCase("purple")) {
			return purple;
		}
		if (color.equalsIgnoreCase("orange")) {
			return orange;
		}
		if (color.equalsIgnoreCase("black")) {
			return black;
		}
		return null;
	}

	public boolean initialize() {
		display = new Display();
		if (display == null) {
			System.err.println("Error creating display.");
			return false;
		}
		shell = new Shell(display, SWT.BORDER | SWT.CLOSE | SWT.MIN | SWT.TITLE);
		if (shell == null) {
			System.err.println("Error creating shell.");
			return false;
		}
		initColors(display);
		return true;
	}
	
	public boolean using() {
		return shell != null;
	}
	
	public void run() {
		GridLayout gl = new GridLayout();
		gl.numColumns = 2;
		shell.setLayout(gl);
		
		GridData gd;
		
		worldGroup = new Group(shell, SWT.NONE);
		worldGroup.setLayout(new FillLayout());
		visualWorld = new VisualWorld(worldGroup, SWT.NONE, simulation, kMainMapCellSize);
		updateWorldGroup();
		gd = new GridData();
		gd.widthHint = m_VisualWorld.getWidth();
		gd.heightHint = m_VisualWorld.getHeight();
		gd.verticalSpan = 3;
		m_WorldGroup.setLayoutData(gd);
		m_VisualWorld.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				Eater eater = m_VisualWorld.getEaterAtPixel(e.x, e.y);
				if (eater == null) {
					return;
				}
				m_AgentDisplay.selectEntity(eater);
			}
		});

		Group group1 = new Group(shell, SWT.NONE);
		gd = new GridData();
		group1.setLayoutData(gd);
		group1.setText("Simulation");
		group1.setLayout(new FillLayout());
		m_SimButtons = new EatersSimulationButtons(group1, m_Simulation);
		
		Group group2 = new Group(shell, SWT.NONE);
		gd = new GridData();
		group2.setLayoutData(gd);
		group2.setText("Map");
		gl = new GridLayout();
		gl.numColumns = 2;
		group2.setLayout(gl);
		Label foodLabel = new Label(group2, SWT.NONE);
		gd = new GridData();
		foodLabel.setLayoutData(gd);
		foodLabel.setText(kFoodRemaining);
		m_FoodCount = new Label(group2, SWT.NONE);
		gd = new GridData();
		gd.widthHint = 50;
		m_FoodCount.setLayoutData(gd);
		Label scoreLabel = new Label(group2, SWT.NONE);
		gd = new GridData();
		scoreLabel.setLayoutData(gd);
		scoreLabel.setText(kScoreRemaining);
		m_ScoreCount = new Label(group2, SWT.NONE);
		gd = new GridData();
		gd.widthHint = 50;
		m_ScoreCount.setLayoutData(gd);
		updateFoodAndScoreCount();
		m_MapButtons = new MapButtons(group2, m_Simulation, EatersSimulation.kMapFilter);
		gd = new GridData();
		gd.horizontalSpan = 2;
		m_MapButtons.setLayoutData(gd);

		m_AgentDisplay = new AgentDisplay(shell, m_Simulation);
		gd = new GridData();
		m_AgentDisplay.setLayoutData(gd);
		
		EatersVisualWorld.remapFoodColors();
		VisualWorld.remapEntityColors(m_Simulation.getEatersWorld().getEaters());

		m_Simulation.addSimulationListener(this);

		shell.setText("Java Eaters");
		shell.setSize(shell.computeSize(SWT.DEFAULT, SWT.DEFAULT));
		
		shell.addShellListener(new ShellAdapter() {
			public void shellDeactivated(ShellEvent e) {
				m_AgentDisplay.worldChangeEvent();			
				m_VisualWorld.redraw();			
			}
			public void shellActivated(ShellEvent e) {
				m_AgentDisplay.worldChangeEvent();			
				m_VisualWorld.setRepaint();
				m_VisualWorld.redraw();			
			}
			public void shellDeiconified(ShellEvent e) {
				m_AgentDisplay.worldChangeEvent();			
				m_VisualWorld.setRepaint();
				m_VisualWorld.redraw();			
			}
		});
		
		shell.addControlListener(new ControlAdapter() {
			public void controlMoved(ControlEvent e) {
				m_AgentDisplay.worldChangeEvent();			
				m_VisualWorld.setRepaint();
				m_VisualWorld.redraw();			
			}
			public void controlResized(ControlEvent e) {
				m_AgentDisplay.worldChangeEvent();			
				m_VisualWorld.setRepaint();
				m_VisualWorld.redraw();			
			}
		});
		
		shell.open();

		while (!shell.isDisposed()) {
			if (!m_Display.readAndDispatch()) {
				m_Display.sleep();
			}
		}
		m_Simulation.removeSimulationListener(this);
		m_Simulation.shutdown();
		m_Display.dispose();		

	}
	
	public void errorMessage(String title, String message) {
		MessageBox mb = new MessageBox(shell, SWT.ICON_ERROR | SWT.WRAP);
		mb.setMessage(message);
		mb.setText(title);
		mb.open();
	}

	EatersSimulation m_Simulation;
	final Label m_FoodCount;
	final Label m_ScoreCount;
	final EatersSimulationButtons m_SimButtons;
	final MapButtons m_MapButtons;
	final EatersVisualWorld m_VisualWorld;
	final AgentDisplay m_AgentDisplay;
	final Group m_WorldGroup;
	
	public EatersWindowManager(EatersSimulation simulation) {
		m_Simulation = simulation;
		
		GridLayout gl = new GridLayout();
		gl.numColumns = 2;
		m_Shell.setLayout(gl);
		
		GridData gd;
		
		m_WorldGroup = new Group(m_Shell, SWT.NONE);
		m_WorldGroup.setLayout(new FillLayout());
		m_VisualWorld = new EatersVisualWorld(m_WorldGroup, SWT.NONE, m_Simulation, kMainMapCellSize);
		updateWorldGroup();
		gd = new GridData();
		gd.widthHint = m_VisualWorld.getWidth();
		gd.heightHint = m_VisualWorld.getHeight();
		gd.verticalSpan = 3;
		m_WorldGroup.setLayoutData(gd);
		m_VisualWorld.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				Eater eater = m_VisualWorld.getEaterAtPixel(e.x, e.y);
				if (eater == null) {
					return;
				}
				m_AgentDisplay.selectEntity(eater);
			}
		});

		Group group1 = new Group(m_Shell, SWT.NONE);
		gd = new GridData();
		group1.setLayoutData(gd);
		group1.setText("Simulation");
		group1.setLayout(new FillLayout());
		m_SimButtons = new EatersSimulationButtons(group1, m_Simulation);
		
		Group group2 = new Group(m_Shell, SWT.NONE);
		gd = new GridData();
		group2.setLayoutData(gd);
		group2.setText("Map");
		gl = new GridLayout();
		gl.numColumns = 2;
		group2.setLayout(gl);
		Label foodLabel = new Label(group2, SWT.NONE);
		gd = new GridData();
		foodLabel.setLayoutData(gd);
		foodLabel.setText(kFoodRemaining);
		m_FoodCount = new Label(group2, SWT.NONE);
		gd = new GridData();
		gd.widthHint = 50;
		m_FoodCount.setLayoutData(gd);
		Label scoreLabel = new Label(group2, SWT.NONE);
		gd = new GridData();
		scoreLabel.setLayoutData(gd);
		scoreLabel.setText(kScoreRemaining);
		m_ScoreCount = new Label(group2, SWT.NONE);
		gd = new GridData();
		gd.widthHint = 50;
		m_ScoreCount.setLayoutData(gd);
		updateFoodAndScoreCount();
		m_MapButtons = new MapButtons(group2, m_Simulation, EatersSimulation.kMapFilter);
		gd = new GridData();
		gd.horizontalSpan = 2;
		m_MapButtons.setLayoutData(gd);

		m_AgentDisplay = new AgentDisplay(m_Shell, m_Simulation);
		gd = new GridData();
		m_AgentDisplay.setLayoutData(gd);
		
		EatersVisualWorld.remapFoodColors();
		VisualWorld.remapEntityColors(m_Simulation.getEatersWorld().getEaters());

		m_Simulation.addSimulationListener(this);

		m_Shell.setText("Java Eaters");
		m_Shell.setSize(m_Shell.computeSize(SWT.DEFAULT, SWT.DEFAULT));
		
		m_Shell.addShellListener(new ShellAdapter() {
			public void shellDeactivated(ShellEvent e) {
				m_AgentDisplay.worldChangeEvent();			
				m_VisualWorld.redraw();			
			}
			public void shellActivated(ShellEvent e) {
				m_AgentDisplay.worldChangeEvent();			
				m_VisualWorld.setRepaint();
				m_VisualWorld.redraw();			
			}
			public void shellDeiconified(ShellEvent e) {
				m_AgentDisplay.worldChangeEvent();			
				m_VisualWorld.setRepaint();
				m_VisualWorld.redraw();			
			}
		});
		
		m_Shell.addControlListener(new ControlAdapter() {
			public void controlMoved(ControlEvent e) {
				m_AgentDisplay.worldChangeEvent();			
				m_VisualWorld.setRepaint();
				m_VisualWorld.redraw();			
			}
			public void controlResized(ControlEvent e) {
				m_AgentDisplay.worldChangeEvent();			
				m_VisualWorld.setRepaint();
				m_VisualWorld.redraw();			
			}
		});
		
		m_Shell.open();

		while (!m_Shell.isDisposed()) {
			if (!m_Display.readAndDispatch()) {
				m_Display.sleep();
			}
		}
		m_Simulation.removeSimulationListener(this);
		m_Simulation.shutdown();
		m_Display.dispose();		
	}
	
	void updateWorldGroup() {
		String currentMap = m_Simulation.getCurrentMap();
		m_WorldGroup.setText(kMapPrefix + currentMap.substring(currentMap.lastIndexOf(System.getProperty("file.separator")) + 1));
		m_VisualWorld.setSize(m_VisualWorld.getWidth(), m_VisualWorld.getHeight());
		GridData gd = new GridData();
		gd.widthHint = m_VisualWorld.getWidth();
		gd.heightHint = m_VisualWorld.getHeight();
		gd.verticalSpan = 3;
		m_WorldGroup.setLayoutData(gd);
		m_Shell.setSize(m_Shell.computeSize(SWT.DEFAULT, SWT.DEFAULT));
	}
	
	void dispatchEvent(int type) {
		switch (type) {
		case SimulationListener.kStartEvent:
			m_SimButtons.updateButtons();
			m_MapButtons.updateButtons();
			m_AgentDisplay.updateButtons();
			return;
			
		case SimulationListener.kStopEvent:
			m_VisualWorld.setRepaint();
			m_VisualWorld.redraw();
			m_SimButtons.updateButtons();
			m_MapButtons.updateButtons();
			m_AgentDisplay.updateButtons();
			return;
			
		case SimulationListener.kErrorMessageEvent:
			MessageBox mb = new MessageBox(m_Shell, SWT.ICON_ERROR | SWT.OK | SWT.WRAP);
			mb.setMessage(m_Simulation.getLastErrorMessage());
			mb.setText("Eaters Error");
			mb.open();
			return;
			
		case SimulationListener.kUpdateEvent:
			m_VisualWorld.redraw();
			updateFoodAndScoreCount();
			m_AgentDisplay.worldChangeEvent();
			return;
			
		case SimulationListener.kResetEvent:
			updateWorldGroup();
			EatersVisualWorld.remapFoodColors();
			m_VisualWorld.setRepaint();
			m_VisualWorld.redraw();
			updateFoodAndScoreCount();
			m_SimButtons.updateButtons();
			m_AgentDisplay.worldChangeEvent();
			return;
			
		case SimulationListener.kAgentCreatedEvent:
			m_VisualWorld.setRepaint();
			m_VisualWorld.redraw();
			EatersVisualWorld.remapEntityColors(m_Simulation.getEatersWorld().getEaters());
			updateFoodAndScoreCount();
			m_SimButtons.updateButtons();
			m_AgentDisplay.agentEvent();
			return;
			
		case SimulationListener.kAgentDestroyedEvent:
			m_VisualWorld.setRepaint();
			m_VisualWorld.redraw();
			EatersVisualWorld.remapEntityColors(m_Simulation.getEatersWorld().getEaters());
			m_SimButtons.updateButtons();
			m_AgentDisplay.agentEvent();
			return;
			
		default:
			logger.warning("Invalid event type received: " + new Integer(type));
			return;
		}		
	}
	
	void updateFoodAndScoreCount() {
		m_FoodCount.setText(Integer.toString(Cell.getFoodCount()));
		m_ScoreCount.setText(Integer.toString(m_Simulation.getEatersWorld().getScoreCount()));
	}

	public void simulationEventHandler(final int type) {
		if (m_Display.isDisposed() || m_Shell.isDisposed()) {
			return;
		}
		m_Display.syncExec(new Runnable() {
			public void run() {
				dispatchEvent(type);
			}
		});
	}

	public void update() {
		// TODO Auto-generated method stub
		
	}

	public void start() {
		// TODO Auto-generated method stub
		
	}

	public void stop() {
		// TODO Auto-generated method stub
		
	}

	public void reset() {
		// TODO Auto-generated method stub
		
	}

	public void shutdown() {
		// TODO Auto-generated method stub
		
	}
}
