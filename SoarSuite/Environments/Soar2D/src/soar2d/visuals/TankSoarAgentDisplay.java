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
	Label facing;
	Label energyCharger;
	Label healthCharger;
	Label resurrect;
	Label shields;
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
			gl.numColumns = 3;
			m_Group.setLayout(gl);
		}
		
		
		m_AgentTable = new Table(m_Group, SWT.BORDER | SWT.FULL_SELECTION);
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 3;
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
		
		m_AgentButtons = new Composite(m_Group, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.BEGINNING;
			m_AgentButtons.setLayoutData(gd);

			GridLayout gl = new GridLayout();
			gl.numColumns = 4;
			gl.horizontalSpacing = 0;
			m_AgentButtons.setLayout(gl);
		}
		
		m_NewAgentButton = new Button(m_AgentButtons, SWT.PUSH);
		m_NewAgentButton.setText("New");
		m_NewAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				new CreateAgentDialog(parent.getShell()).open();
			}
		});
		{
			GridData gd = new GridData();
			m_NewAgentButton.setLayoutData(gd);
		}
		
		m_CloneAgentButton = new Button(m_AgentButtons, SWT.PUSH);
		m_CloneAgentButton.setText("Clone");
		m_CloneAgentButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				Soar2D.simulation.clonePlayer(selectedPlayer.getName(), null);
			}
		});
		{
			GridData gd = new GridData();
			m_CloneAgentButton.setLayoutData(gd);
		}
		
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
		{
			GridData gd = new GridData();
			m_DestroyAgentButton.setLayoutData(gd);
		}
			
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
		{
			GridData gd = new GridData();
			m_ReloadProductionsButton.setLayoutData(gd);
		}

		Group row5 = new Group(m_Group, SWT.NONE);
		row5.setText("Radar");
		{
			GridData gd = new GridData();
			gd.verticalSpan = 3;
			gd.verticalAlignment = SWT.BOTTOM;
			row5.setLayoutData(gd);

			GridLayout gl = new GridLayout();
			gl.numColumns = 2;
			row5.setLayout(gl);
		}
		
		m_AgentWorld = new TankSoarAgentWorld(row5, SWT.BORDER);
		{
			GridData gd = new GridData();
			gd.heightHint = m_AgentWorld.getHeight();
			gd.widthHint = m_AgentWorld.getWidth();		
			m_AgentWorld.setLayoutData(gd);
		}
		

		m_Radar = new ProgressBar(row5, SWT.NONE | SWT.VERTICAL);
		m_Radar.setMinimum(0);
		m_Radar.setMaximum(Soar2D.config.kRadarHeight);
		{
			GridData gd = new GridData();
			gd.heightHint = m_AgentWorld.getHeight();
			m_Radar.setLayoutData(gd);
		}
		
		Group smellGroup = new Group(m_Group, SWT.NONE);
		smellGroup.setText("Smell");
		smellGroup.setLayout(new GridLayout());
		{
			GridData gd = new GridData();
			gd.verticalSpan = 3;
			gd.verticalAlignment = SWT.BOTTOM;
			smellGroup.setLayoutData(gd);
			
			GridLayout gl = new GridLayout();
			smellGroup.setLayout(gl);
		}
		m_Smell = new ProgressBar(smellGroup, SWT.VERTICAL);
		m_Smell.setMinimum(0);
		// TODO: 
		//m_Smell.setMaximum(m_Simulation.getTankSoarWorld().getMaxManhattanDistance());
		m_Smell.setMaximum(25);
		{
			GridData gd = new GridData();
			gd.heightHint = m_AgentWorld.getHeight();
			m_Smell.setLayoutData(gd);
		}
		
		Group locGroup = new Group(m_Group, SWT.NONE);
		locGroup.setText("Other Sensors");
		{
			GridData gd = new GridData();
			locGroup.setLayoutData(gd);

			GridLayout gl = new GridLayout();
			gl.numColumns = 2;
			locGroup.setLayout(gl);
		}
		
		Label locationLabel = new Label(locGroup, SWT.NONE);
		locationLabel.setText("Location:");
		{
			GridData gd = new GridData();
			locationLabel.setLayoutData(gd);
		}
		
		location = new Label(locGroup, SWT.NONE);
		location.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 35;
			location.setLayoutData(gd);
		}
		
		Label facingLabel = new Label(locGroup, SWT.NONE);
		facingLabel.setText("Facing:");
		{
			GridData gd = new GridData();
			facingLabel.setLayoutData(gd);
		}
		
		facing = new Label(locGroup, SWT.NONE);
		facing.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 35;
			facing.setLayoutData(gd);
		}
		
		Label energyChargerLabel = new Label(locGroup, SWT.NONE);
		energyChargerLabel.setText("On Energy Charger:");
		{
			GridData gd = new GridData();
			energyChargerLabel.setLayoutData(gd);
		}
		
		energyCharger = new Label(locGroup, SWT.NONE);
		energyCharger.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 35;
			energyCharger.setLayoutData(gd);
		}
		
		Label healthChargerLabel = new Label(locGroup, SWT.NONE);
		healthChargerLabel.setText("On Health Charger:");
		{
			GridData gd = new GridData();
			healthChargerLabel.setLayoutData(gd);
		}
		
		healthCharger = new Label(locGroup, SWT.NONE);
		healthCharger.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 35;
			healthCharger.setLayoutData(gd);
		}
		
		Label resurrectLabel = new Label(locGroup, SWT.NONE);
		resurrectLabel.setText("Resurrect:");
		{
			GridData gd = new GridData();
			resurrectLabel.setLayoutData(gd);
		}
		
		resurrect = new Label(locGroup, SWT.NONE);
		resurrect.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 35;
			resurrect.setLayoutData(gd);
		}
		
		Label shieldsLabel = new Label(locGroup, SWT.NONE);
		shieldsLabel.setText("Shields:");
		{
			GridData gd = new GridData();
			shieldsLabel.setLayoutData(gd);
		}
		
		shields = new Label(locGroup, SWT.NONE);
		shields.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 35;
			shields.setLayoutData(gd);
		}
		
		Composite fourWaySensors = new Composite(m_Group, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.verticalAlignment = SWT.BOTTOM;
			gd.horizontalAlignment = GridData.END;
			fourWaySensors.setLayoutData(gd);

			GridLayout gl = new GridLayout();
			gl.numColumns = 2;
			gl.horizontalSpacing = 0;
			fourWaySensors.setLayout(gl);
		}

		Group blockedGroup = new Group(fourWaySensors, SWT.NONE);
		blockedGroup.setText("Blocked");
		blockedGroup.setLayoutData(new GridData());
		blockedGroup.setLayout(new FillLayout());
		m_Blocked = new BlockedDiagram(blockedGroup, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.heightHint = 50;
			gd.widthHint = 55;
			gd.horizontalAlignment = SWT.END;
			gd.verticalAlignment = SWT.BOTTOM;
			gd.grabExcessVerticalSpace = true;
			blockedGroup.setLayoutData(gd);
		}
		
		Group rwavesGroup = new Group(fourWaySensors, SWT.NONE);
		rwavesGroup.setText("RWaves");
		rwavesGroup.setLayoutData(new GridData());
		rwavesGroup.setLayout(new FillLayout());
		m_RWaves = new BlockedDiagram(rwavesGroup, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.heightHint = 50;
			gd.widthHint = 55;
			gd.horizontalAlignment = SWT.END;
			gd.verticalAlignment = SWT.BOTTOM;
			gd.grabExcessVerticalSpace = true;
			rwavesGroup.setLayoutData(gd);
		}
		
		Group soundGroup = new Group(fourWaySensors, SWT.NONE);
		soundGroup.setText("Sound");
		soundGroup.setLayoutData(new GridData());
		soundGroup.setLayout(new FillLayout());
		m_Sound = new BlockedDiagram(soundGroup, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.heightHint = 50;
			gd.widthHint = 55;
			gd.horizontalAlignment = SWT.END;
			gd.verticalAlignment = SWT.BOTTOM;
			gd.grabExcessVerticalSpace = true;
			soundGroup.setLayoutData(gd);
		}
		
		Group incomingGroup = new Group(fourWaySensors, SWT.NONE);
		incomingGroup.setText("Incoming");
		incomingGroup.setLayoutData(new GridData());
		incomingGroup.setLayout(new FillLayout());
		m_Incoming = new BlockedDiagram(incomingGroup, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.heightHint = 50;
			gd.widthHint = 55;
			gd.horizontalAlignment = SWT.END;
			gd.verticalAlignment = SWT.BOTTOM;
			gd.grabExcessVerticalSpace = true;
			incomingGroup.setLayoutData(gd);
		}
		
		updatePlayerList();
		updateButtons();		
	}
	
	void selectPlayer(Player player) {
		selectedPlayer = player;
		m_AgentTable.setSelection(players.indexOf(selectedPlayer));
		m_AgentWorld.enable();
		updateSensors();
		updateButtons();
	}
	
	private void updateSensors() {
		m_AgentWorld.update(selectedPlayer);
		int facingInt = selectedPlayer.getFacingInt();
		m_Radar.setSelection(selectedPlayer.getRadarPower());
		m_Radar.setToolTipText(Integer.toString(selectedPlayer.getRadarPower()));
		m_RWaves.set(selectedPlayer.getRWaves(), facingInt);
		m_Blocked.set(selectedPlayer.getBlocked(), facingInt);
		facing.setText(Direction.stringOf[facingInt]);
		energyCharger.setText(selectedPlayer.getOnEnergyCharger() ? "yes" : "no");
		healthCharger.setText(selectedPlayer.getOnHealthCharger() ? "yes" : "no");
		resurrect.setText(selectedPlayer.getResurrect() ? "yes" : "no");
		shields.setText(selectedPlayer.shieldsUp() ? "yes" : "no");
		m_Incoming.set(selectedPlayer.getIncoming(), facingInt);
		switch (selectedPlayer.getSound()) {
		case Direction.kNorthInt:
			m_Sound.set(Direction.kNorthIndicator, facingInt);
			break;
		case Direction.kSouthInt:
			m_Sound.set(Direction.kSouthIndicator, facingInt);
			break;
		case Direction.kEastInt:
			m_Sound.set(Direction.kEastIndicator, facingInt);
			break;
		case Direction.kWestInt:
			m_Sound.set(Direction.kWestIndicator, facingInt);
			break;
		default:
			m_Sound.set(0, facingInt);
		}
		m_Smell.setSelection(selectedPlayer.getSmellDistance());
		if (selectedPlayer.getSmellColor() != null) {
			m_Smell.setForeground(WindowManager.getColor(selectedPlayer.getSmellColor()));
			m_Smell.setToolTipText(selectedPlayer.getSmellColor() + " is " + Integer.toString(selectedPlayer.getSmellDistance()) + " spaces away");
		} else {
			m_Smell.setToolTipText("no smell");
		}
		java.awt.Point playerLocation = Soar2D.simulation.world.getLocation(selectedPlayer);
		location.setText("(" + playerLocation.x + "," + playerLocation.y + ")");
		m_AgentWorld.redraw();
	}
	
	private void disableSensors() {
		m_AgentTable.deselectAll();
		m_AgentWorld.disable();
		m_AgentWorld.redraw();
		m_RWaves.disable();
		m_Blocked.disable();
		facing.setText("-");
		energyCharger.setText("-");
		healthCharger.setText("-");
		resurrect.setText("-");
		shields.setText("-");
		m_Incoming.disable();
		m_Sound.disable();
		m_Radar.setSelection(0);
		m_Radar.setToolTipText("0");
		m_Smell.setSelection(0);
		m_Smell.setToolTipText("no smell");
		location.setText("-");
	}
	
	void agentEvent() {
		updatePlayerList();
		updateButtons();
	}

	void worldChangeEvent() {
		if (selectedPlayer != null) {
			updateSensors();
		}
		
		synchronized(Soar2D.wm) {
			Soar2D.wm.agentDisplayUpdated = true;
			Soar2D.wm.notify();
		}
		
		for (int i = 0; i < m_Items.length; ++i) {
			m_Items[i].setText(new String[] {
					players.get(i).getName(), 
					Integer.toString(players.get(i).getPoints()),
					Integer.toString(players.get(i).getMissiles()),
					Integer.toString(players.get(i).getHealth()),
					Integer.toString(players.get(i).getEnergy())					
					});
		}
	}
	
	void updatePlayerList() {
		players = Soar2D.simulation.world.getPlayers();
		m_AgentTable.removeAll();
		boolean foundSelected = false;
		
		m_Items = new TableItem[players.size()];
		for (int i = 0; i < players.size(); ++i) {
			m_Items[i] = new TableItem(m_AgentTable, SWT.NONE);
			m_Items[i].setText(new String[] {
					players.get(i).getName(), 
					Integer.toString(players.get(i).getPoints()),
					Integer.toString(players.get(i).getMissiles()),
					Integer.toString(players.get(i).getHealth()),
					Integer.toString(players.get(i).getEnergy())					
					});
			if (selectedPlayer == players.get(i)) {
				foundSelected = true;
				m_AgentTable.setSelection(i);
			}
		}
		
		if (!foundSelected) {
			selectedPlayer = null;			
			disableSensors();
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
