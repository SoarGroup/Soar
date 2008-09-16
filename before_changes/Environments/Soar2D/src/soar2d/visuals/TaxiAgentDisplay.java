package soar2d.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.map.TaxiMap;
import soar2d.player.*;
import soar2d.world.PlayersManager;

public class TaxiAgentDisplay extends AgentDisplay {
	
	public static final int kAgentMapCellSize = 20;
	static final int kTableHeight = 16;
	static final int kNameWidth = 75;
	static final int kScoreWidth = 55;
	
	Group m_Group;
	Table m_AgentTable;
	Player selectedPlayer;
	TableItem[] m_Items = new TableItem[0];
	PlayersManager players;
	Composite m_AgentButtons;
	Button m_NewAgentButton;
	Button m_DestroyAgentButton;
	Button m_ReloadProductionsButton;
	Label location;
	Label destination;
	ProgressBar fuel;

	public TaxiAgentDisplay(final Composite parent) {
		super(parent);

		setLayout(new FillLayout());
		
		players = Soar2D.simulation.world.getPlayers();
		
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
			gd.heightHint = kTableHeight;
			gd.horizontalSpan = 2;
			m_AgentTable.setLayoutData(gd);
		}
		TableColumn tc1 = new TableColumn(m_AgentTable, SWT.CENTER);
		TableColumn tc2 = new TableColumn(m_AgentTable, SWT.CENTER);
		tc1.setText("Name");
		tc1.setWidth(kNameWidth);
		tc2.setText("Reward");
		tc2.setWidth(kScoreWidth);
		m_AgentTable.setHeaderVisible(true);
		m_AgentTable.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				selectPlayer(players.get(m_AgentTable.getSelectionIndex()));
				updateButtons();
			}
		});
		
		Composite worldGroup = new Composite(m_Group, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.numColumns = 2;
			worldGroup.setLayout(gl);
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

		Label destinationLabel = new Label(worldGroup, SWT.NONE);
		destinationLabel.setText("Destination: ");
		{
			GridData gd = new GridData();
			destinationLabel.setLayoutData(gd);
		}
		
		destination = new Label(worldGroup, SWT.NONE);
		TaxiMap xMap = (TaxiMap)Soar2D.simulation.world.getMap();
		if (xMap.getPassengerDestination() != null) {
			destination.setText(xMap.getPassengerDestination());
		} else {
			destination.setText("-");
		}
		{
			GridData gd = new GridData();
			gd.widthHint = 60;
			destination.setLayoutData(gd);
		}
		
		Label fuelLabel = new Label(worldGroup, SWT.NONE);
		fuelLabel.setText("Fuel: ");
		{
			GridData gd = new GridData();
			fuelLabel.setLayoutData(gd);
		}
		
		fuel = new ProgressBar(worldGroup, SWT.NONE);
		fuel.setMinimum(0);
		fuel.setMaximum(14);
		{
			GridData gd = new GridData();
			gd.widthHint = 80;
			fuel.setLayoutData(gd);
		}

		updateTaxiList();
		updateButtons();		
	}
	
	void selectPlayer(Player player) {
		selectedPlayer = player;
		m_AgentTable.setSelection(players.indexOf(player));
		java.awt.Point playerLocation = players.getLocation(selectedPlayer);
		int newY = Soar2D.simulation.world.getMap().getSize() - 1 - playerLocation.y;
		location.setText("(" + playerLocation.x + "," + newY + ")");
		updateButtons();
	}
	
	void agentEvent() {
		updateTaxiList();
		updateButtons();
	}

	void worldChangeEvent() {
		TaxiMap xMap = (TaxiMap)Soar2D.simulation.world.getMap();
		if (selectedPlayer != null) {
			java.awt.Point playerLocation = players.getLocation(selectedPlayer);
			int newY = Soar2D.simulation.world.getMap().getSize() - 1 - playerLocation.y;
			location.setText("(" + playerLocation.x + "," + newY + ")");
			fuel.setSelection(xMap.getFuel());
			fuel.setToolTipText(Integer.toString(xMap.getFuel()));
		}
		
		for (int i = 0; i < m_Items.length; ++i) {
			if (players.get(i).pointsChanged()) {
				m_Items[i].setText(1, Integer.toString(players.get(i).getPointsDelta()));
			} else {
				m_Items[i].setText(1, "0");
			}
		}
		
		if (xMap.getPassengerDestination() != null) {
			destination.setText(xMap.getPassengerDestination());
		} else {
			destination.setText("-");
		}
	}
	
	void updateTaxiList() {
		players = Soar2D.simulation.world.getPlayers();
		m_AgentTable.removeAll();
		boolean foundSelected = false;
		
		m_Items = new TableItem[players.numberOfPlayers()];
		for (int i = 0; i < players.numberOfPlayers(); ++i) {
			m_Items[i] = new TableItem(m_AgentTable, SWT.NONE);
			if (players.get(i).pointsChanged()) {
				m_Items[i].setText(new String[] {players.get(i).getName(), Integer.toString(players.get(i).getPointsDelta())});
			} else {
				m_Items[i].setText(new String[] {players.get(i).getName(), "0" });
			}
			if (selectedPlayer == players.get(i)) {
				foundSelected = true;
				m_AgentTable.setSelection(i);
			}
		}
		
		if (!foundSelected) {
			selectedPlayer = null;			
			m_AgentTable.deselectAll();
			location.setText("-");
		}
		
		if (players.numberOfPlayers() > 0) {
			selectPlayer(players.get(0));
		}
	}
	
	void updateButtons() {
		boolean running = Soar2D.control.isRunning();
		boolean slotsAvailable = Soar2D.simulation.getUnusedColors().size() > 6;
		boolean hasPlayers = players.numberOfPlayers() > 0;
		boolean selectedCook = (selectedPlayer != null);
		
		m_NewAgentButton.setEnabled(!running && slotsAvailable);
		m_DestroyAgentButton.setEnabled(!running && hasPlayers && selectedCook);
		m_ReloadProductionsButton.setEnabled(!running && hasPlayers && selectedCook);
 	}
}
