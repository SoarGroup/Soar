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
	
	Group m_Group;
	Logger m_Logger = Logger.logger;
	TankSoarSimulation m_Simulation;
	Table m_AgentTable;
	WorldEntity m_SelectedEntity;
	TableItem[] m_Items;
	Tank[] m_Tanks;
	Composite m_AgentButtons;
	Button m_NewAgentButton;
	Button m_CloneAgentButton;
	Button m_DestroyAgentButton;
	Button m_SpawnDebuggersButton;
	TankSoarAgentWorld m_AgentWorld;

	public AgentDisplay(final Composite parent, TankSoarSimulation simulation) {
		super(parent, SWT.NONE);
		m_Simulation = simulation;

		setLayout(new FillLayout());
		
		m_Group = new Group(this, SWT.NONE);
		m_Group.setText("Agents");
		GridLayout gl = new GridLayout();
		gl.numColumns = 2;
		m_Group.setLayout(gl);
		
		GridData gd;

		m_AgentButtons = new Composite(m_Group, SWT.NONE);
		m_AgentButtons.setLayout(new FillLayout());
		gd = new GridData();
		gd.horizontalAlignment = GridData.BEGINNING;
		gd.horizontalSpan = 2;
		m_AgentButtons.setLayoutData(gd);
		
		m_NewAgentButton = new Button(m_AgentButtons, SWT.PUSH);
		m_NewAgentButton.setText("New");
		m_NewAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				new CreateAgentDialog(parent.getShell(), m_Simulation).open();
			}
		});
		
		m_CloneAgentButton = new Button(m_AgentButtons, SWT.PUSH);
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
		
		m_DestroyAgentButton = new Button(m_AgentButtons, SWT.PUSH);
		m_DestroyAgentButton.setText("Destroy");
		m_DestroyAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (m_SelectedEntity == null) {
					return;
				}
				m_Simulation.destroyEntity(m_SelectedEntity);
			}
		});
				
		m_SpawnDebuggersButton = new Button(m_Group, SWT.CHECK);
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

		m_AgentTable = new Table(m_Group, SWT.BORDER | SWT.FULL_SELECTION);
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
		
		m_AgentWorld = new TankSoarAgentWorld(m_Group, SWT.BORDER, m_Simulation);
		gd = new GridData();
		gd.heightHint = m_AgentWorld.getHeight();
		gd.widthHint = m_AgentWorld.getWidth();		
		m_AgentWorld.setLayoutData(gd);

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
				m_Items[i].setText(new String[] {m_Tanks[i].getName(), Integer.toString(m_Tanks[i].getPoints())});
				if (m_SelectedEntity == m_Tanks[i]) {
					foundSelected = true;
					m_AgentTable.setSelection(i);
				}
			}
		}
		
		if (!foundSelected) {
			m_SelectedEntity = null;			
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
