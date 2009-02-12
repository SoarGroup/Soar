package broken.soar2d.visuals;

import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;

import soar2d.Soar2D;
import soar2d.players.Player;
import soar2d.world.PlayersManager;

public class KitchenAgentDisplay extends AgentDisplay {
	
	public static final int kAgentMapCellSize = 20;
	static final int kTableHeight = 120;
	static final int kNameWidth = 75;
	static final int kScoreWidth = 55;
	
	Group m_Group;
	Table m_AgentTable;
	Player selectedPlayer;
	TableItem[] m_Items = new TableItem[0];
	PlayersManager players;
	Composite m_AgentButtons;
	Button m_NewAgentButton;
	Button m_CloneAgentButton;
	Button m_DestroyAgentButton;
	Button m_ReloadProductionsButton;
	Label location;

	public KitchenAgentDisplay(final Composite parent) {
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

		updateCookList();
		updateButtons();		
	}
	
	void selectPlayer(Player player) {
		selectedPlayer = player;
		m_AgentTable.setSelection(players.indexOf(player));
		int [] playerLocation = players.getLocation(selectedPlayer);
		location.setText("(" + playerLocation[0] + "," + playerLocation[1] + ")");
		updateButtons();
	}
	
	void agentEvent() {
		updateCookList();
		updateButtons();
	}

	void worldChangeEvent() {
		if (selectedPlayer != null) {
			int [] playerLocation = players.getLocation(selectedPlayer);
			location.setText("(" + playerLocation[0] + "," + playerLocation[1] + ")");
		}
		
		for (int i = 0; i < m_Items.length; ++i) {
			if (players.get(i).pointsChanged()) {
				m_Items[i].setText(1, Integer.toString(players.get(i).getPointsDelta()));
			} else {
				m_Items[i].setText(1, "0");
			}
		}
	}
	
	void updateCookList() {
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
	}
	
	void updateButtons() {
		boolean running = Soar2D.control.isRunning();
		boolean slotsAvailable = Soar2D.simulation.getUnusedColors().size() > 0;
		boolean hasPlayers = players.numberOfPlayers() > 0;
		boolean selectedCook = (selectedPlayer != null);
		
		m_NewAgentButton.setEnabled(!running && slotsAvailable);
		m_CloneAgentButton.setEnabled(!running && slotsAvailable && selectedCook);
		m_DestroyAgentButton.setEnabled(!running && hasPlayers && selectedCook);
		m_ReloadProductionsButton.setEnabled(!running && hasPlayers && selectedCook);
 	}
}
