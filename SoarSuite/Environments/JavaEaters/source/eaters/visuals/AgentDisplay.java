package eaters.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import eaters.*;

import simulation.*;
import simulation.visuals.*;
import utilities.*;

public class AgentDisplay extends Composite {
	public static final int kAgentMapCellSize = 20;
	static final int kTableHeight = 120;
	static final int kNameWidth = 75;
	static final int kScoreWidth = 50;
	
	Group m_Group;
	Logger m_Logger = Logger.logger;
	EatersSimulation m_Simulation;
	Table m_AgentTable;
	EatersVisualWorld m_AgentWorld;
	WorldEntity m_SelectedEntity;
	TableItem[] m_Items;
	WorldEntity[] m_Entities;
	Composite m_AgentButtons;
	Button m_NewAgentButton;
	Button m_CloneAgentButton;
	Button m_DestroyAgentButton;
	Button m_SpawnDebuggersButton;

	public AgentDisplay(final Composite parent, EatersSimulation simulation) {
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
				for (int i = 0; i < EatersWindowManager.kColors.length; ++i) {
					boolean notTaken = true;
					for (int j = 0; j < m_Entities.length; ++j) {
						if (m_Entities[j].getColor().equalsIgnoreCase(EatersWindowManager.kColors[i])) {
							notTaken = false;
							break;
						}
					}
					if (notTaken) {
						color = EatersWindowManager.kColors[i];
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
		tc1.setText("Name");
		tc1.setWidth(kNameWidth);
		tc2.setText("Score");
		tc2.setWidth(kScoreWidth);
		m_AgentTable.setHeaderVisible(true);
		m_AgentTable.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (m_Entities == null) {
					return;
				}
				for (int i = 0; i < m_Entities.length; ++i) {
					selectEntity(m_Entities[m_AgentTable.getSelectionIndex()]);
				}
				updateButtons();
			}
		});
		m_AgentWorld = new EatersVisualWorld(m_Group, SWT.BORDER, m_Simulation, kAgentMapCellSize);
		gd = new GridData();
		gd.heightHint = m_AgentWorld.getMiniHeight() + 4;
		gd.widthHint = m_AgentWorld.getMiniWidth() + 4;		
		m_AgentWorld.setLayoutData(gd);

		updateEaterList();
		updateButtons();		
	}
	
	void selectEntity(WorldEntity entity) {
		m_SelectedEntity = entity;
		for (int i = 0; i < m_Entities.length; ++i) {
			if (m_SelectedEntity == m_Entities[i]) {
				m_AgentTable.setSelection(i);
				break;
			}
		}
		m_AgentWorld.setAgentLocation(m_SelectedEntity.getLocation());
		m_AgentWorld.enable();
		m_AgentWorld.redraw();
	}
	
	void agentEvent() {
		updateEaterList();
		updateButtons();
	}

	void worldChangeEvent() {
		if (m_SelectedEntity != null) {
			m_AgentWorld.setAgentLocation(m_SelectedEntity.getLocation());
			m_AgentWorld.redraw();
		}
		
		if (m_Items != null) {
			for (int i = 0; i < m_Items.length; ++i) {
				m_Items[i].setText(1, Integer.toString(m_Entities[i].getPoints()));
			}
		}
	}
	
	void updateEaterList() {
		m_Entities = m_Simulation.getEatersWorld().getEaters();
		m_AgentTable.removeAll();
		boolean foundSelected = false;
		
		if (m_Entities == null) {
			m_Items = null;
		} else {
			m_Items = new TableItem[m_Entities.length];
			for (int i = 0; i < m_Entities.length; ++i) {
				m_Items[i] = new TableItem(m_AgentTable, SWT.NONE);
				m_Items[i].setText(new String[] {m_Entities[i].getName(), Integer.toString(m_Entities[i].getPoints())});
				if (m_SelectedEntity == m_Entities[i]) {
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
		if (m_Entities != null) {
			agentsFull = (m_Entities.length == EatersSimulation.kMaxEaters);
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
