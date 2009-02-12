package org.msoar.gridmap2d.visuals;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.msoar.gridmap2d.CognitiveArchitecture;
import org.msoar.gridmap2d.Gridmap2D;
import org.msoar.gridmap2d.map.GridMap;
import org.msoar.gridmap2d.players.Player;
import org.msoar.gridmap2d.world.World;


public class EatersAgentDisplay extends AgentDisplay {
	
	public static final int kAgentMapCellSize = 20;
	static final int kTableHeight = 120;
	static final int kNameWidth = 75;
	static final int kScoreWidth = 55;
	
	Group m_Group;
	Table m_AgentTable;
	EatersVisualWorld m_AgentWorld;
	World world;
	Player selectedPlayer;
	Player[] players;
	TableItem[] m_Items = new TableItem[0];
	Composite m_AgentButtons;
	Button m_NewAgentButton;
	Button m_CloneAgentButton;
	Button m_DestroyAgentButton;
	Button m_ReloadProductionsButton;
	Label location;

	public EatersAgentDisplay(final Composite parent, World world, final CognitiveArchitecture cogArch) {
		super(parent);
		this.world = world;

		players = world.getPlayers();
		
		setLayout(new FillLayout());
		
		m_Group = new Group(this, SWT.NONE);
		m_Group.setText("Agents");
		{
			GridLayout gl = new GridLayout();
			gl.numColumns = 2;
			m_Group.setLayout(gl);
		}
		
		
		m_AgentButtons = new Composite(m_Group, SWT.NONE);
		m_AgentButtons.setLayout(new FillLayout());
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.BEGINNING;
			gd.horizontalSpan = 2;
			m_AgentButtons.setLayoutData(gd);
		}
		
		m_NewAgentButton = new Button(m_AgentButtons, SWT.PUSH);
		m_NewAgentButton.setText("New");
		m_NewAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				new CreateAgentDialog(parent.getShell(), cogArch).open();
			}
		});
		
		m_CloneAgentButton = new Button(m_AgentButtons, SWT.PUSH);
		m_CloneAgentButton.setText("Clone");
		m_CloneAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				clonePlayer(selectedPlayer.getID());
			}
		});
		
		m_DestroyAgentButton = new Button(m_AgentButtons, SWT.PUSH);
		m_DestroyAgentButton.setText("Destroy");
		m_DestroyAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (selectedPlayer == null) {
					return;
				}
				try {
					Gridmap2D.simulation.destroyPlayer(selectedPlayer);
				} catch (Exception ignored) {
				}
			}
		});
				
		m_ReloadProductionsButton = new Button(m_AgentButtons, SWT.PUSH);
		m_ReloadProductionsButton.setText("Reload");
		m_ReloadProductionsButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (selectedPlayer == null) {
					return;
				}
				Gridmap2D.simulation.reloadPlayer(selectedPlayer);
			}
		});
				
		m_AgentTable = new Table(m_Group, SWT.BORDER | SWT.FULL_SELECTION);
		{
			GridData gd = new GridData();
			gd.heightHint = kTableHeight;
			m_AgentTable.setLayoutData(gd);
		}
		TableColumn tc1 = new TableColumn(m_AgentTable, SWT.CENTER);
		TableColumn tc2 = new TableColumn(m_AgentTable, SWT.CENTER);
		tc1.setText("Name");
		tc1.setWidth(kNameWidth);
		tc2.setText("Score");
		tc2.setWidth(kScoreWidth);
		m_AgentTable.setHeaderVisible(true);
		m_AgentTable.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				selectPlayer(players[m_AgentTable.getSelectionIndex()]);
				updateButtons();
			}
		});
		
		Composite worldGroup = new Composite(m_Group, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.numColumns = 2;
			worldGroup.setLayout(gl);
		}

		m_AgentWorld = new EatersVisualWorld(worldGroup, SWT.BORDER, kAgentMapCellSize);
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			gd.heightHint = m_AgentWorld.getMiniHeight() + 4;
			gd.widthHint = m_AgentWorld.getMiniWidth() + 4;
			m_AgentWorld.setLayoutData(gd);
		}
		
		Label locationLabel = new Label(worldGroup, SWT.NONE);
		locationLabel.setText("Location: ");
		{
			GridData gd = new GridData();
			locationLabel.setLayoutData(gd);
		}
		
		location = new Label(worldGroup, SWT.NONE);
		location.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 35;
			location.setLayoutData(gd);
		}

		updateEaterList();
		updateButtons();		
	}
	
	void selectPlayer(Player player) {
		selectedPlayer = player;
		int index;
		for(index = 0; index < players.length; ++index) {
			if (players[index].equals(selectedPlayer)) {
				break;
			}
		}
		m_AgentTable.setSelection(index);
		int [] playerLocation = selectedPlayer.getLocation();
		m_AgentWorld.setAgentLocation(playerLocation);
		m_AgentWorld.enable();
		m_AgentWorld.redraw();
		location.setText("(" + playerLocation[0] + "," + playerLocation[1] + ")");
		updateButtons();
	}
	
	void agentEvent() {
		updateEaterList();
		updateButtons();
	}

	void worldChangeEvent() {
		if (selectedPlayer != null) {
			int [] playerLocation = selectedPlayer.getLocation();
			m_AgentWorld.setAgentLocation(playerLocation);
			m_AgentWorld.redraw();
			location.setText("(" + playerLocation[0] + "," + playerLocation[1] + ")");
		}
		
		for (int i = 0; i < m_Items.length; ++i) {
			m_Items[i].setText(1, Integer.toString(players[i].getPoints()));
		}
	}
	
	void updateEaterList() {
		players = world.getPlayers();
		m_AgentTable.removeAll();
		boolean foundSelected = false;
		
		m_Items = new TableItem[world.numberOfPlayers()];
		for (int i = 0; i < world.numberOfPlayers(); ++i) {
			m_Items[i] = new TableItem(m_AgentTable, SWT.NONE);
			m_Items[i].setText(new String[] {players[i].getName(), Integer.toString(players[i].getPoints())});
			if (selectedPlayer == players[i]) {
				foundSelected = true;
				m_AgentTable.setSelection(i);
			}
		}
		
		if (!foundSelected) {
			selectedPlayer = null;			
			m_AgentTable.deselectAll();
			m_AgentWorld.disable();
			m_AgentWorld.redraw();
			location.setText("-");
		}
	}
	
	void updateButtons() {
		boolean running = Gridmap2D.control.isRunning();
		boolean slotsAvailable = Gridmap2D.simulation.getUnusedColors().size() > 0;
		boolean hasPlayers = world.numberOfPlayers() > 0;
		boolean selectedEater = (selectedPlayer != null);
		
		m_NewAgentButton.setEnabled(!running && slotsAvailable);
		m_CloneAgentButton.setEnabled(!running && slotsAvailable && selectedEater);
		m_DestroyAgentButton.setEnabled(!running && hasPlayers && selectedEater);
		m_ReloadProductionsButton.setEnabled(!running && hasPlayers && selectedEater);
 	}
	
	public void setMap(GridMap map) {
		m_AgentWorld.setMap(map);
	}
}
