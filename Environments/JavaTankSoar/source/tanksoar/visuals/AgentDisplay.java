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
	static final int kScoreWidth = 55;
	static final int kMissilesWidth = 66;
	static final int kHealthWidth = 58;
	static final int kEnergyWidth = 60;
	
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
	BlockedDiagram m_Blocked;
	BlockedDiagram m_RWaves;
	BlockedDiagram m_Sound;
	BlockedDiagram m_Incoming;

	public AgentDisplay(final Composite parent, TankSoarSimulation simulation) {
		super(parent, SWT.NONE);
		m_Simulation = simulation;

		GridLayout gl = new GridLayout();
		gl.numColumns = 1;
		setLayout(gl);

		GridData gd;	
		
		Composite row1 = new Composite(this, SWT.NONE);
		row1.setLayoutData(new GridData());
		row1.setLayout(new FillLayout());
		
		m_NewAgentButton = new Button(row1, SWT.PUSH);
		m_NewAgentButton.setText("New");
		m_NewAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				new CreateAgentDialog(parent.getShell(), m_Simulation).open();
			}
		});
		
		m_CloneAgentButton = new Button(row1, SWT.PUSH);
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
				m_Simulation.createEntity(color, m_SelectedEntity.getProductions(), color, null, null, -1, -1, -1);
			}
		});
		
		m_DestroyAgentButton = new Button(row1, SWT.PUSH);
		m_DestroyAgentButton.setText("Destroy");
		m_DestroyAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (m_SelectedEntity == null) {
					return;
				}
				m_Simulation.destroyEntity(m_SelectedEntity);
			}
		});
				
		m_SpawnDebuggersButton = new Button(this, SWT.CHECK);
		m_SpawnDebuggersButton.setLayoutData(new GridData());
		m_SpawnDebuggersButton.setText("Spawn debugger");
		m_SpawnDebuggersButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				m_Simulation.setSpawnDebuggers(m_SpawnDebuggersButton.getSelection());
			}
		});		

		m_AgentTable = new Table(this, SWT.BORDER | SWT.FULL_SELECTION);
		gd = new GridData();
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
		
		Composite row3 = new Composite(this, SWT.NONE);
		row3.setLayoutData(new GridData());
		gl = new GridLayout();
		gl.numColumns = 4;
		row3.setLayout(gl);
		
		Group blockedGroup = new Group(row3, SWT.NONE);
		gd = new GridData();
		gd.heightHint = 50;
		gd.widthHint = 50;
		blockedGroup.setLayoutData(gd);
		blockedGroup.setText("Blocked");
		blockedGroup.setLayout(new FillLayout());
		m_Blocked = new BlockedDiagram(blockedGroup, SWT.NONE);
		
		Group rwavesGroup = new Group(row3, SWT.NONE);
		gd = new GridData();
		gd.heightHint = 50;
		gd.widthHint = 50;
		rwavesGroup.setLayoutData(gd);
		rwavesGroup.setText("RWaves");
		rwavesGroup.setLayout(new FillLayout());
		m_RWaves = new BlockedDiagram(rwavesGroup, SWT.NONE);
		
		Group soundGroup = new Group(row3, SWT.NONE);
		gd = new GridData();
		gd.heightHint = 50;
		gd.widthHint = 50;
		soundGroup.setLayoutData(gd);
		soundGroup.setText("Sound");
		soundGroup.setLayout(new FillLayout());
		m_Sound = new BlockedDiagram(soundGroup, SWT.NONE);
		
		Group incomingGroup = new Group(row3, SWT.NONE);
		gd = new GridData();
		gd.heightHint = 50;
		gd.widthHint = 50;
		incomingGroup.setLayoutData(gd);
		incomingGroup.setText("Incoming");
		incomingGroup.setLayout(new FillLayout());
		m_Incoming = new BlockedDiagram(incomingGroup, SWT.NONE);
		
		Group smellGroup = new Group(this, SWT.NONE);
		smellGroup.setText("Smell distance");
		smellGroup.setLayout(new FillLayout());
		gd = new GridData();
		gd.verticalAlignment = SWT.TOP;
		smellGroup.setLayoutData(gd);
		m_Smell = new ProgressBar(smellGroup, SWT.HORIZONTAL);
		m_Smell.setMinimum(0);
		m_Smell.setMaximum(m_Simulation.getTankSoarWorld().getMaxManhattanDistance());

		Group row5 = new Group(this, SWT.NONE);
		row5.setText("Radar data and setting");
		gl = new GridLayout();
		row5.setLayout(gl);
		
		m_AgentWorld = new TankSoarAgentWorld(row5, SWT.BORDER, m_Simulation);
		gd = new GridData();
		gd.heightHint = m_AgentWorld.getHeight();
		gd.widthHint = m_AgentWorld.getWidth();		
		m_AgentWorld.setLayoutData(gd);

		m_Radar = new ProgressBar(row5, SWT.NONE);
		gd = new GridData();
		gd.widthHint = m_AgentWorld.getWidth();
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
		m_Blocked.updateBlocked(m_Simulation.getTankSoarWorld(), m_Tanks[m_AgentTable.getSelectionIndex()]);
		m_RWaves.updateRWaves(m_Simulation.getTankSoarWorld(), m_Tanks[m_AgentTable.getSelectionIndex()]);
		m_Sound.updateSound(m_Simulation.getTankSoarWorld(), m_Tanks[m_AgentTable.getSelectionIndex()]);
		m_Incoming.updateIncoming(m_Simulation.getTankSoarWorld(), m_Tanks[m_AgentTable.getSelectionIndex()]);
		m_Radar.setSelection(m_Tanks[m_AgentTable.getSelectionIndex()].getRadarSetting());
		m_Radar.setToolTipText(Integer.toString(m_Radar.getSelection()));
		m_Smell.setSelection(m_Tanks[m_AgentTable.getSelectionIndex()].getSmellDistance());
		m_Smell.setToolTipText(Integer.toString(m_Smell.getSelection()));
		m_AgentWorld.enable();
		m_AgentWorld.redraw();
		m_Blocked.redraw();
		m_RWaves.redraw();
		m_Sound.redraw();
		m_Incoming.redraw();
	}
	
	void agentEvent() {
		updateTankList();
		updateButtons();
	}

	void worldChangeEvent() {
		m_Smell.setMaximum(m_Simulation.getTankSoarWorld().getMaxManhattanDistance());
		if (m_SelectedEntity != null) {
			m_AgentWorld.update(m_Tanks[m_AgentTable.getSelectionIndex()]);
			m_Blocked.updateBlocked(m_Simulation.getTankSoarWorld(), m_Tanks[m_AgentTable.getSelectionIndex()]);
			m_RWaves.updateRWaves(m_Simulation.getTankSoarWorld(), m_Tanks[m_AgentTable.getSelectionIndex()]);
			m_Sound.updateSound(m_Simulation.getTankSoarWorld(), m_Tanks[m_AgentTable.getSelectionIndex()]);
			m_Incoming.updateIncoming(m_Simulation.getTankSoarWorld(), m_Tanks[m_AgentTable.getSelectionIndex()]);
			m_Radar.setSelection(m_Tanks[m_AgentTable.getSelectionIndex()].getRadarSetting());
			m_Radar.setToolTipText(Integer.toString(m_Radar.getSelection()));
			m_Smell.setSelection(m_Tanks[m_AgentTable.getSelectionIndex()].getSmellDistance());
			m_Smell.setToolTipText(Integer.toString(m_Smell.getSelection()));
			m_AgentWorld.redraw();
			m_Blocked.redraw();
			m_RWaves.redraw();
			m_Sound.redraw();
			m_Incoming.redraw();
		} else {
			m_Radar.setSelection(0);
			m_Radar.setToolTipText("0");
			m_Smell.setSelection(0);
			m_Smell.setToolTipText("0");
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
			m_Blocked.disable();
			m_RWaves.disable();
			m_Sound.disable();
			m_Incoming.disable();
			m_AgentWorld.redraw();
			m_Blocked.redraw();
			m_RWaves.redraw();
			m_Sound.redraw();
			m_Incoming.redraw();
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
