package soar2d.visuals;

import java.util.*;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.player.*;

public class BookAgentDisplay extends AgentDisplay {
	
	static final int kTableHeight = 120;
	static final int kNameWidth = 75;
	static final int kScoreWidth = 55;
	
	Group m_Group;
	Table m_AgentTable;
	Player selectedPlayer;
	TableItem[] m_Items = new TableItem[0];
	ArrayList<Player> players = new ArrayList<Player>();
	Composite m_AgentButtons;
	Button m_NewAgentButton;
	Button m_CloneAgentButton;
	Button m_DestroyAgentButton;
	Button m_ReloadProductionsButton;

	public BookAgentDisplay(final Composite parent) {
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
				selectPlayer(players.get(m_AgentTable.getSelectionIndex()));
				updateButtons();
			}
		});
		
		updatePlayerList();
		updateButtons();		
	}
	
	void selectPlayer(Player player) {
		selectedPlayer = player;
		m_AgentTable.setSelection(players.indexOf(player));
		updateButtons();
	}
	
	void agentEvent() {
		updatePlayerList();
		updateButtons();
	}

	void worldChangeEvent() {
		if (selectedPlayer == null) {
			synchronized(Soar2D.wm) {
				Soar2D.wm.agentDisplayUpdated = true;
				Soar2D.wm.notify();
			}
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
