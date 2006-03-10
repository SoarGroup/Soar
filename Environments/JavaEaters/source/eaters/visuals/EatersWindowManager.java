package eaters.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.events.*;

import eaters.*;
import simulation.*;
import simulation.visuals.*;

public class EatersWindowManager extends WindowManager implements SimulationListener {
	public static final int kMainMapCellSize = 20;
	public static final String kFoodRemaining = "Food remaining: ";
	public static final String kScoreRemaining = "Points remaining: ";

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
		
		EatersVisualWorld.remapFoodColors(m_Simulation.getEatersWorld().getFood());
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
			m_VisualWorld.setRepaint();
			m_VisualWorld.redraw();
			EatersVisualWorld.remapFoodColors(m_Simulation.getEatersWorld().getFood());
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
			m_Logger.log("Invalid event type received: " + new Integer(type));
			return;
		}		
	}
	
	void updateFoodAndScoreCount() {
		m_FoodCount.setText(Integer.toString(m_Simulation.getEatersWorld().getFoodCount()));
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
}
