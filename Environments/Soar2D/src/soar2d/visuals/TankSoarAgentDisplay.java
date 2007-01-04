package soar2d.visuals;

import java.util.*;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.player.*;

public class TankSoarAgentDisplay extends AgentDisplay {
	
	public static final int kAgentMapCellSize = 20;
	static final int kTableHeight = 120;
	static final int kNameWidth = 75;
	static final int kScoreWidth = 55;
	static final int kMissilesWidth = 66;
	static final int kHealthWidth = 58;
	static final int kEnergyWidth = 60;
	
	Group m_Group;
	Table m_AgentTable;
	TankSoarAgentWorld m_AgentWorld;
	Player selectedPlayer;
	TableItem[] m_Items = new TableItem[0];
	ArrayList<Player> players = new ArrayList<Player>();
	Composite m_AgentButtons;
	Button m_NewAgentButton;
	Button m_CloneAgentButton;
	Button m_DestroyAgentButton;
	Button m_ReloadProductionsButton;
	Label location;
	BlockedDiagram m_Blocked;
	BlockedDiagram m_RWaves;
	BlockedDiagram m_Sound;
	BlockedDiagram m_Incoming;
	ProgressBar m_Smell;
	ProgressBar m_Radar;

	public TankSoarAgentDisplay(final Composite parent) {
		super(parent);

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
				new CreateAgentDialog(parent.getShell()).open();
			}
		});
		
		m_CloneAgentButton = new Button(m_AgentButtons, SWT.PUSH);
		m_CloneAgentButton.setText("Clone");
		m_CloneAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				Soar2D.simulation.clonePlayer(selectedPlayer.getName(), null);
			}
		});
		
		m_DestroyAgentButton = new Button(m_AgentButtons, SWT.PUSH);
		m_DestroyAgentButton.setText("Destroy");
		m_DestroyAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (selectedPlayer == null) {
					return;
				}
				Soar2D.simulation.destroyPlayer(selectedPlayer);
			}
		});
				
		m_ReloadProductionsButton = new Button(m_AgentButtons, SWT.PUSH);
		m_ReloadProductionsButton.setText("Reload");
		m_ReloadProductionsButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (selectedPlayer == null) {
					return;
				}
				Soar2D.simulation.reloadPlayer(selectedPlayer);
			}
		});
		
		m_AgentTable = new Table(m_Group, SWT.BORDER | SWT.FULL_SELECTION);
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			gd.heightHint = kTableHeight;
			m_AgentTable.setLayoutData(gd);
		}
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
				selectPlayer(players.get(m_AgentTable.getSelectionIndex()));
				updateButtons();
			}
		});
		
		Composite row3 = new Composite(m_Group, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			row3.setLayoutData(gd);
			
			GridLayout gl = new GridLayout();
			gl.numColumns = 4;
			row3.setLayout(gl);
		}
		
		Group blockedGroup = new Group(row3, SWT.NONE);
		blockedGroup.setText("Blocked");
		blockedGroup.setLayout(new FillLayout());
		m_Blocked = new BlockedDiagram(blockedGroup, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.heightHint = 50;
			gd.widthHint = 50;
			blockedGroup.setLayoutData(gd);
		}
		
		Group rwavesGroup = new Group(row3, SWT.NONE);
		rwavesGroup.setText("RWaves");
		rwavesGroup.setLayout(new FillLayout());
		m_RWaves = new BlockedDiagram(rwavesGroup, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.heightHint = 50;
			gd.widthHint = 50;
			rwavesGroup.setLayoutData(gd);
		}
		
		Group soundGroup = new Group(row3, SWT.NONE);
		soundGroup.setText("Sound");
		soundGroup.setLayout(new FillLayout());
		m_Sound = new BlockedDiagram(soundGroup, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.heightHint = 50;
			gd.widthHint = 50;
			soundGroup.setLayoutData(gd);
		}
		
		Group incomingGroup = new Group(row3, SWT.NONE);
		incomingGroup.setText("Incoming");
		incomingGroup.setLayout(new FillLayout());
		m_Incoming = new BlockedDiagram(incomingGroup, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.heightHint = 50;
			gd.widthHint = 50;
			incomingGroup.setLayoutData(gd);
		}
		
		Group smellGroup = new Group(m_Group, SWT.NONE);
		smellGroup.setText("Smell distance");
		smellGroup.setLayout(new FillLayout());
		m_Smell = new ProgressBar(smellGroup, SWT.HORIZONTAL);
		m_Smell.setMinimum(0);
		// TODO: 
		//m_Smell.setMaximum(m_Simulation.getTankSoarWorld().getMaxManhattanDistance());
		m_Smell.setMaximum(25);
		{
			GridData gd = new GridData();
			gd.verticalAlignment = SWT.TOP;
			gd.horizontalSpan = 2;
			smellGroup.setLayoutData(gd);
		}

		Label locationLabel = new Label(m_Group, SWT.NONE);
		locationLabel.setText("Location: ");
		{
			GridData gd = new GridData();
			locationLabel.setLayoutData(gd);
		}

		location = new Label(m_Group, SWT.NONE);
		location.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 35;
			location.setLayoutData(gd);
		}

		Group row5 = new Group(m_Group, SWT.NONE);
		row5.setText("Radar data and setting");
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			row5.setLayoutData(gd);

			GridLayout gl = new GridLayout();
			row5.setLayout(gl);
		}
		
		m_AgentWorld = new TankSoarAgentWorld(row5, SWT.BORDER);
		{
			GridData gd = new GridData();
			gd.heightHint = m_AgentWorld.getHeight();
			gd.widthHint = m_AgentWorld.getWidth();		
			m_AgentWorld.setLayoutData(gd);
		}
		

		m_Radar = new ProgressBar(row5, SWT.NONE);
		m_Radar.setMinimum(0);
		m_Radar.setMaximum(Soar2D.config.kRadarHeight);
		{
			GridData gd = new GridData();
			gd.widthHint = m_AgentWorld.getWidth();
			m_Radar.setLayoutData(gd);
		}
		
		updatePlayerList();
		updateButtons();		
	}
	
	void selectPlayer(Player player) {
		selectedPlayer = player;
		m_AgentWorld.update(selectedPlayer);
		m_Radar.setSelection(selectedPlayer.getRadarPower());
		m_Radar.setToolTipText(Integer.toString(selectedPlayer.getRadarPower()));
		m_AgentTable.setSelection(players.indexOf(player));
		java.awt.Point playerLocation = Soar2D.simulation.world.getLocation(selectedPlayer);
		m_AgentWorld.enable();
		m_AgentWorld.redraw();
		location.setText("(" + playerLocation.x + "," + playerLocation.y + ")");
		updateButtons();
	}
	
	void agentEvent() {
		updatePlayerList();
		updateButtons();
	}

	void worldChangeEvent() {
		if (selectedPlayer != null) {
			m_AgentWorld.update(selectedPlayer);
			m_Radar.setSelection(selectedPlayer.getRadarPower());
			m_Radar.setToolTipText(Integer.toString(selectedPlayer.getRadarPower()));
			java.awt.Point playerLocation = Soar2D.simulation.world.getLocation(selectedPlayer);
			m_AgentWorld.redraw();
			location.setText("(" + playerLocation.x + "," + playerLocation.y + ")");
		}
		
		for (int i = 0; i < m_Items.length; ++i) {
			m_Items[i].setText(1, Integer.toString(players.get(i).getPoints()));
		}
	}
	
	void updatePlayerList() {
		players = Soar2D.simulation.world.getPlayers();
		m_AgentTable.removeAll();
		boolean foundSelected = false;
		
		m_Items = new TableItem[players.size()];
		for (int i = 0; i < players.size(); ++i) {
			m_Items[i] = new TableItem(m_AgentTable, SWT.NONE);
			m_Items[i].setText(new String[] {players.get(i).getName(), Integer.toString(players.get(i).getPoints())});
			if (selectedPlayer == players.get(i)) {
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
		boolean running = Soar2D.control.isRunning();
		boolean slotsAvailable = Soar2D.simulation.getUnusedColors().size() > 0;
		boolean hasPlayers = Soar2D.simulation.hasPlayers();
		boolean selectedEater = (selectedPlayer != null);
		
		m_NewAgentButton.setEnabled(!running && slotsAvailable);
		m_CloneAgentButton.setEnabled(!running && slotsAvailable && selectedEater);
		m_DestroyAgentButton.setEnabled(!running && hasPlayers && selectedEater);
		m_ReloadProductionsButton.setEnabled(!running && hasPlayers && selectedEater);
 	}
}
