package tanksoar.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.events.*;

import tanksoar.*;
import simulation.*;
import simulation.visuals.*;

public class TankSoarWindowManager extends WindowManager implements SimulationListener {
	public static final String kFoodRemaining = "Food remaining: ";
	public static final String kScoreRemaining = "Points remaining: ";

	TankSoarSimulation m_Simulation;
	final TankSoarSimulationButtons m_SimButtons;
	final MapButtons m_MapButtons;
	final TankSoarVisualWorld m_VisualWorld;
	final AgentDisplay m_AgentDisplay;
	final Group m_WorldGroup;
	
	public TankSoarWindowManager(TankSoarSimulation simulation) {
		m_Simulation = simulation;
		
		GridLayout gl = new GridLayout();
		gl.numColumns = 3;
		m_Shell.setLayout(gl);
		
		GridData gd;
		
		m_WorldGroup = new Group(m_Shell, SWT.NONE);
		m_WorldGroup.setLayout(new FillLayout());
		m_VisualWorld = new TankSoarVisualWorld(m_WorldGroup, SWT.NONE, m_Simulation);
		updateWorldGroup();
		gd = new GridData();
		gd.widthHint = m_VisualWorld.getWidth();
		gd.heightHint = m_VisualWorld.getHeight();
		gd.verticalSpan = 2;
		m_WorldGroup.setLayoutData(gd);
		m_VisualWorld.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				Tank tank = m_VisualWorld.getTankAtPixel(e.x, e.y);
				if (tank == null) {
					return;
				}
				m_AgentDisplay.selectEntity(tank);
			}
		});

		Group group1 = new Group(m_Shell, SWT.NONE);
		gd = new GridData();
		group1.setLayoutData(gd);
		group1.setText("Simulation");
		group1.setLayout(new FillLayout());
		m_SimButtons = new TankSoarSimulationButtons(group1, m_Simulation);
		
		Group group2 = new Group(m_Shell, SWT.NONE);
		gd = new GridData();
		group2.setLayoutData(gd);
		group2.setText("Map");
		group2.setLayout(new FillLayout());
		m_MapButtons = new MapButtons(group2, m_Simulation, TankSoarSimulation.kMapFilter);

		Group group3 = new Group(m_Shell, SWT.NONE);
		gd = new GridData();
		group3.setLayoutData(gd);
		group3.setText("Agents");
		group3.setLayout(new FillLayout());
		m_AgentDisplay = new AgentDisplay(group3, m_Simulation);
		gd = new GridData();
		gd.horizontalSpan = 2;
		group3.setLayoutData(gd);
		
		VisualWorld.remapEntityColors(m_Simulation.getWorldManager().getEntities());
		m_VisualWorld.generateBackground();

		m_Simulation.addSimulationListener(this);

		m_Shell.setText("Java TankSoar");
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
			MessageBox mb1 = new MessageBox(m_Shell, SWT.ICON_ERROR | SWT.OK | SWT.WRAP);
			mb1.setMessage(m_Simulation.getLastErrorMessage());
			mb1.setText("TankSoar Error");
			mb1.open();
			return;
			
		case SimulationListener.kNotificationEvent:
			MessageBox mb2 = new MessageBox(m_Shell, SWT.ICON_INFORMATION | SWT.OK | SWT.WRAP);
			mb2.setMessage(m_Simulation.getLastErrorMessage());
			mb2.setText("TankSoar");
			mb2.open();
			return;
			
		case SimulationListener.kUpdateEvent:
			m_VisualWorld.redraw();
			m_AgentDisplay.worldChangeEvent();
			return;
			
		case SimulationListener.kResetEvent:
			updateWorldGroup();
			m_VisualWorld.generateBackground();
			m_VisualWorld.setRepaint();
			m_VisualWorld.redraw();
			m_SimButtons.updateButtons();
			m_AgentDisplay.worldChangeEvent();
			return;
			
		case SimulationListener.kAgentCreatedEvent:
			m_VisualWorld.setRepaint();
			m_VisualWorld.redraw();
			VisualWorld.remapEntityColors(m_Simulation.getWorldManager().getEntities());
			m_SimButtons.updateButtons();
			m_AgentDisplay.agentEvent();
			return;
			
		case SimulationListener.kAgentDestroyedEvent:
			m_VisualWorld.setRepaint();
			m_VisualWorld.redraw();
			VisualWorld.remapEntityColors(m_Simulation.getWorldManager().getEntities());
			m_SimButtons.updateButtons();
			m_AgentDisplay.agentEvent();
			return;
			
		default:
			m_Logger.log("Invalid event type received: " + new Integer(type));
			return;
		}		
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
