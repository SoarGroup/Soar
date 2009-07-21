package edu.umich.soar.gridmap2d.visuals;

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
import org.eclipse.swt.widgets.ProgressBar;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;

import edu.umich.soar.gridmap2d.CognitiveArchitecture;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.map.TaxiMap;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.Taxi;
import edu.umich.soar.gridmap2d.world.World;

public class TaxiAgentDisplay extends AgentDisplay {
	
	public static final int kAgentMapCellSize = 20;
	static final int kTableHeight = 30;
	static final int kNameWidth = 75;
	static final int kScoreWidth = 55;
	
	Group m_Group;
	Table m_AgentTable;
	Player selectedPlayer;
	TableItem[] m_Items = new TableItem[0];
	Player[] players;
	Composite m_AgentButtons;
	Button m_NewAgentButton;
	Button m_DestroyAgentButton;
	Button m_ReloadProductionsButton;
	Label location;
	Label destination;
	ProgressBar fuel;
	World world;

	public TaxiAgentDisplay(final Composite parent, World world, final CognitiveArchitecture cogArch) {
		super(parent);

		setLayout(new FillLayout());
		
		players = world.getPlayers();
		this.world = world;
		
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
			gd.minimumHeight = kTableHeight;
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
		TaxiMap xMap = (TaxiMap)world.getMap();
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
		int index;
		for(index = 0; index < players.length; ++index) {
			if (players[index].equals(selectedPlayer)) {
				break;
			}
		}
		m_AgentTable.setSelection(index);
		int [] playerLocation = selectedPlayer.getLocation();
		int newY = world.getMap().size() - 1 - playerLocation[1];
		location.setText("(" + playerLocation[0] + "," + newY + ")");
		updateButtons();
	}
	
	void agentEvent() {
		updateTaxiList();
		updateButtons();
	}

	void worldChangeEvent() {
		TaxiMap xMap = (TaxiMap)world.getMap();
		if (selectedPlayer != null) {
			int [] playerLocation = selectedPlayer.getLocation();
			int newY = world.getMap().size() - 1 - playerLocation[1];
			location.setText("(" + playerLocation[0] + "," + newY + ")");
			Taxi taxi = (Taxi)selectedPlayer;
			fuel.setSelection(taxi.getFuel());
			fuel.setToolTipText(Integer.toString(taxi.getFuel()));
		}
		
		for (int i = 0; i < m_Items.length; ++i) {
			if (players[i].pointsChanged()) {
				m_Items[i].setText(1, Integer.toString(players[i].getPointsDelta()));
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
		players = world.getPlayers();
		m_AgentTable.removeAll();
		boolean foundSelected = false;
		
		m_Items = new TableItem[players.length];
		for (int i = 0; i < players.length; ++i) {
			m_Items[i] = new TableItem(m_AgentTable, SWT.NONE);
			if (players[i].pointsChanged()) {
				m_Items[i].setText(new String[] {players[i].getName(), Integer.toString(players[i].getPointsDelta())});
			} else {
				m_Items[i].setText(new String[] {players[i].getName(), "0" });
			}
			if (selectedPlayer == players[i]) {
				foundSelected = true;
				m_AgentTable.setSelection(i);
			}
		}
		
		if (!foundSelected) {
			selectedPlayer = null;			
			m_AgentTable.deselectAll();
			location.setText("-");
		}
		
		if (players.length > 0) {
			selectPlayer(players[0]);
		}
	}
	
	void updateButtons() {
		boolean running = Gridmap2D.control.isRunning();
		boolean slotsAvailable = Gridmap2D.simulation.getUnusedColors().size() > 6;
		boolean hasPlayers = players.length > 0;
		boolean selectedCook = (selectedPlayer != null);
		
		m_NewAgentButton.setEnabled(!running && slotsAvailable);
		m_DestroyAgentButton.setEnabled(!running && hasPlayers && selectedCook);
		m_ReloadProductionsButton.setEnabled(!running && hasPlayers && selectedCook);
 	}
}
