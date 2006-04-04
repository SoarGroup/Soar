package tanksoar.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import tanksoar.*;

import simulation.*;
import simulation.visuals.*;
import utilities.*;

public class AgentDisplay extends Composite {
	public static final int kAgentMapCellSize = 20;
	static final int kTableHeight = 120;
	static final int kNameWidth = 75;
	static final int kScoreWidth = 40;
	static final int kMissilesWidth = 48;
	static final int kHealthWidth = 45;
	static final int kEnergyWidth = 46;
	
	Logger m_Logger = Logger.logger;
	TankSoarSimulation m_Simulation;
	Table m_AgentTable;
	WorldEntity m_SelectedEntity;
	TableItem[] m_Items;
	Tank[] m_Tanks;
	Button m_NewAgentButton;
	Button m_CloneAgentButton;
	Button m_DestroyAgentButton;
	Button m_SpawnDebuggersButton;
	TankSoarAgentWorld m_AgentWorld;
	ProgressBar m_Smell;
	ProgressBar m_Radar;

	public AgentDisplay(final Composite parent, TankSoarSimulation simulation) {
		super(parent, SWT.NONE);
		m_Simulation = simulation;

		setLayout(new FillLayout());
		
		Group outerGroup = new Group(this, SWT.NONE);
		outerGroup.setText("Agents");
		
		GridLayout gl = new GridLayout();
		gl.numColumns = 2;
		outerGroup.setLayout(gl);
		
		GridData gd;
		
		Composite leftComposite = new Composite(outerGroup, SWT.NONE);
		gd = new GridData();
		gd.verticalAlignment = GridData.BEGINNING;
		leftComposite.setLayoutData(gd);

		gl = new GridLayout();
		gl.numColumns = 2;
		leftComposite.setLayout(gl);

		Composite agentButtons;
		agentButtons = new Composite(leftComposite, SWT.NONE);
		agentButtons.setLayout(new FillLayout());
		gd = new GridData();
		gd.horizontalAlignment = GridData.BEGINNING;
		gd.horizontalSpan = 2;
		agentButtons.setLayoutData(gd);
		
		m_NewAgentButton = new Button(agentButtons, SWT.PUSH);
		m_NewAgentButton.setText("New");
		m_NewAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				new CreateAgentDialog(parent.getShell(), m_Simulation).open();
			}
		});
		
		m_CloneAgentButton = new Button(agentButtons, SWT.PUSH);
		m_CloneAgentButton.setText("Clone");
		m_CloneAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				String color = null;
				// TODO: this probably isn't the most efficient way of doing this, but this is not a bottleneck point
				for (int i = 0; i < TankSoarWindowManager.kColors.length; ++i) {
					boolean notTaken = true;
					for (int j = 0; j < m_Tanks.length; ++j) {
						if (m_Tanks[j].getColor().equalsIgnoreCase(TankSoarWindowManager.kColors[i])) {
							notTaken = false;
							break;
						}
					}
					if (notTaken) {
						color = TankSoarWindowManager.kColors[i];
						break;
					}
				}
				
				// Risking null exception here, but that should not be possible ;)
				m_Simulation.createEntity(color, m_SelectedEntity.getProductions(), color, null, null);
			}
		});
		
		m_DestroyAgentButton = new Button(agentButtons, SWT.PUSH);
		m_DestroyAgentButton.setText("Destroy");
		m_DestroyAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (m_SelectedEntity == null) {
					return;
				}
				m_Simulation.destroyEntity(m_SelectedEntity);
			}
		});
				
		m_SpawnDebuggersButton = new Button(leftComposite, SWT.CHECK);
		gd = new GridData();
		gd.horizontalAlignment = GridData.BEGINNING;
		gd.horizontalSpan = 2;
		m_SpawnDebuggersButton.setLayoutData(gd);
		m_SpawnDebuggersButton.setText("Spawn debugger");
		m_SpawnDebuggersButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.setSpawnDebuggers(m_SpawnDebuggersButton.getSelection());
			}
		});		

		m_AgentTable = new Table(leftComposite, SWT.BORDER | SWT.FULL_SELECTION);
		gd = new GridData();
		gd.horizontalAlignment = GridData.BEGINNING;
		gd.horizontalSpan = 2;
		gd.heightHint = kTableHeight;
		m_AgentTable.setLayoutData(gd);
		TableColumn tc1 = new TableColumn(m_AgentTable, SWT.CENTER);
		TableColumn tc2 = new TableColumn(m_AgentTable, SWT.CENTER);
		TableColumn tc3 = new TableColumn(m_AgentTable, SWT.CENTER);
		TableColumn tc4 = new TableColumn(m_AgentTable, SWT.CENTER);
		TableColumn tc5 = new TableColumn(m_AgentTable, SWT.CENTER);
		tc1.setText("Name");
		tc1.setWidth(kNameWidth);
		tc2.setText("Score");
		tc2.setWidth(kScoreWidth);
		tc3.setText("Missiles");
		tc3.setWidth(kMissilesWidth);
		tc4.setText("Health");
		tc4.setWidth(kHealthWidth);
		tc5.setText("Energy");
		tc5.setWidth(kEnergyWidth);
		m_AgentTable.setHeaderVisible(true);
		m_AgentTable.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (m_Tanks == null) {
					return;
				}
				for (int i = 0; i < m_Tanks.length; ++i) {
					selectEntity(m_Tanks[m_AgentTable.getSelectionIndex()]);
				}
				updateButtons();
			}
		});
		
		Label blocked = new Label(leftComposite, SWT.BORDER);
		gd = new GridData();
		gd.grabExcessHorizontalSpace = true;
		gd.horizontalAlignment = GridData.CENTER;
		gd.heightHint = 50;
		gd.widthHint = 50;
		blocked.setLayoutData(gd);
		blocked.setText("blocked");

		Label rwaves = new Label(leftComposite, SWT.BORDER);
		gd = new GridData();
		gd.grabExcessHorizontalSpace = true;
		gd.horizontalAlignment = GridData.CENTER;
		gd.heightHint = 50;
		gd.widthHint = 50;
		rwaves.setLayoutData(gd);
		rwaves.setText("rwaves");

		Label sound = new Label(leftComposite, SWT.BORDER);
		gd = new GridData();
		gd.grabExcessHorizontalSpace = true;
		gd.horizontalAlignment = GridData.CENTER;
		gd.heightHint = 50;
		gd.widthHint = 50;
		sound.setLayoutData(gd);
		sound.setText("sound");

		Label incoming = new Label(leftComposite, SWT.BORDER);
		gd = new GridData();
		gd.grabExcessHorizontalSpace = true;
		gd.horizontalAlignment = GridData.CENTER;
		gd.heightHint = 50;
		gd.widthHint = 50;
		incoming.setLayoutData(gd);
		incoming.setText("incoming");
		
		Group smellGroup = new Group(leftComposite, SWT.NONE);
		smellGroup.setText("Smell");
		smellGroup.setLayout(new FillLayout());
		gd = new GridData();
		gd.horizontalSpan = 2;
		smellGroup.setLayoutData(gd);
		m_Smell = new ProgressBar(smellGroup, SWT.HORIZONTAL);
		m_Smell.setMinimum(0);
		m_Smell.setMaximum(m_Simulation.getTankSoarWorld().getMaxManhattanDistance());

		Group rightGroup = new Group(outerGroup, SWT.NONE);
		rightGroup.setText("Radar");
		gl = new GridLayout();
		gl.numColumns = 2;
		rightGroup.setLayout(gl);
		
		m_AgentWorld = new TankSoarAgentWorld(rightGroup, SWT.BORDER, m_Simulation);
		gd = new GridData();
		gd.heightHint = m_AgentWorld.getHeight();
		gd.widthHint = m_AgentWorld.getWidth();		
		m_AgentWorld.setLayoutData(gd);

		m_Radar = new ProgressBar(rightGroup, SWT.VERTICAL);
		gd = new GridData();
		gd.heightHint = m_AgentWorld.getHeight();
		m_Radar.setLayoutData(gd);
		m_Radar.setMinimum(0);
		m_Radar.setMaximum(14);
		
		updateTankList();
		updateButtons();		
	}
	
	void selectEntity(WorldEntity entity) {
		m_SelectedEntity = entity;
		for (int i = 0; i < m_Tanks.length; ++i) {
			if (m_SelectedEntity == m_Tanks[i]) {
				m_AgentTable.setSelection(i);
				break;
			}
		}
		m_AgentWorld.update(m_Tanks[m_AgentTable.getSelectionIndex()]);
		m_Radar.setSelection(m_Tanks[m_AgentTable.getSelectionIndex()].getRadarSetting());
		m_Radar.setToolTipText(Integer.toString(m_Radar.getSelection()));
		m_Smell.setSelection(m_Tanks[m_AgentTable.getSelectionIndex()].getSmellDistance());
		m_Smell.setToolTipText(Integer.toString(m_Smell.getSelection()));
		m_AgentWorld.enable();
		m_AgentWorld.redraw();
	}
	
	void agentEvent() {
		updateTankList();
		updateButtons();
	}

	void worldChangeEvent() {
		if (m_SelectedEntity != null) {
			m_AgentWorld.update(m_Tanks[m_AgentTable.getSelectionIndex()]);
			m_Radar.setSelection(m_Tanks[m_AgentTable.getSelectionIndex()].getRadarSetting());
			m_Radar.setToolTipText(Integer.toString(m_Radar.getSelection()));
			m_Smell.setSelection(m_Tanks[m_AgentTable.getSelectionIndex()].getSmellDistance());
			m_Smell.setToolTipText(Integer.toString(m_Smell.getSelection()));
			m_AgentWorld.redraw();
		}
		
		if (m_Items != null) {
			for (int i = 0; i < m_Items.length; ++i) {
				m_Items[i].setText(1, Integer.toString(m_Tanks[i].getPoints()));
				m_Items[i].setText(2, Integer.toString(m_Tanks[i].getMissiles()));
				m_Items[i].setText(3, Integer.toString(m_Tanks[i].getHealth()));
				m_Items[i].setText(4, Integer.toString(m_Tanks[i].getEnergy()));
			}
		}
	}
	
	void updateTankList() {
		m_Tanks = m_Simulation.getTankSoarWorld().getTanks();
		m_AgentTable.removeAll();
		boolean foundSelected = false;
		
		if (m_Tanks == null) {
			m_Items = null;
		} else {
			m_Items = new TableItem[m_Tanks.length];
			for (int i = 0; i < m_Tanks.length; ++i) {
				m_Items[i] = new TableItem(m_AgentTable, SWT.NONE);
				m_Items[i].setText(0, m_Tanks[i].getName());
				m_Items[i].setText(1, Integer.toString(m_Tanks[i].getPoints()));
				m_Items[i].setText(2, Integer.toString(m_Tanks[i].getMissiles()));
				m_Items[i].setText(3, Integer.toString(m_Tanks[i].getHealth()));
				m_Items[i].setText(4, Integer.toString(m_Tanks[i].getEnergy()));
				if (m_SelectedEntity == m_Tanks[i]) {
					foundSelected = true;
					m_AgentTable.setSelection(i);
				}
			}
		}
		
		if (!foundSelected) {
			m_SelectedEntity = null;			
			m_Radar.setSelection(0);
			m_Radar.setToolTipText("0");
			m_Smell.setSelection(0);
			m_Smell.setToolTipText("0");
			m_AgentTable.deselectAll();
			m_AgentWorld.disable();
			m_AgentWorld.redraw();
		}
	}
	
	void updateButtons() {
		boolean running = m_Simulation.isRunning();
		boolean agentsFull = false;
		boolean noAgents = false;
		if (m_Tanks != null) {
			agentsFull = (m_Tanks.length == TankSoarSimulation.kMaxTanks);
		} else {
			noAgents = true;
		}
		boolean selectedEater = (m_SelectedEntity != null);
		boolean spawnDebuggers = m_Simulation.getSpawnDebuggers();
		
		m_NewAgentButton.setEnabled(!running && !agentsFull);
		m_CloneAgentButton.setEnabled(!running && !agentsFull && selectedEater);
		m_DestroyAgentButton.setEnabled(!running && !noAgents && selectedEater);
		m_SpawnDebuggersButton.setSelection(spawnDebuggers);
 	}
}
