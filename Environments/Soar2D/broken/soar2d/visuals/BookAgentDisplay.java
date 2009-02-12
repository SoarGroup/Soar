package broken.soar2d.visuals;

import java.text.NumberFormat;

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

import soar2d.Soar2D;
import soar2d.players.Player;
import soar2d.world.PlayersManager;

public class BookAgentDisplay extends AgentDisplay {
	
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
	Label gridLocation;
	Label floatLocation;
	Label heading;
	Label area;
	Label collision;
	Label velocity;
	Label rotation;
	Label speed;
	Label carry;
	
	public BookAgentDisplay(final Composite parent) {
		super(parent);

		setLayout(new FillLayout());
		
		players = Soar2D.simulation.world.getPlayers();
		
		m_Group = new Group(this, SWT.NONE);
		m_Group.setText("Agents");
		{
			GridLayout gl = new GridLayout();
			gl.numColumns = 1;
			m_Group.setLayout(gl);
		}
		
		
		m_AgentButtons = new Composite(m_Group, SWT.NONE);
		m_AgentButtons.setLayout(new FillLayout());
		{
			GridData gd = new GridData();
			gd.horizontalAlignment = GridData.BEGINNING;
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
		
		Group locGroup = new Group(m_Group, SWT.NONE);
		locGroup.setText("Other Sensors");
		{
			GridData gd = new GridData();
			locGroup.setLayoutData(gd);

			GridLayout gl = new GridLayout();
			gl.numColumns = 2;
			locGroup.setLayout(gl);
		}
		
		Label gridLocationLabel = new Label(locGroup, SWT.NONE);
		gridLocationLabel.setText("Grid Location:");
		{
			GridData gd = new GridData();
			gridLocationLabel.setLayoutData(gd);
		}
		
		gridLocation = new Label(locGroup, SWT.NONE);
		gridLocation.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 80;
			gridLocation.setLayoutData(gd);
		}
		
		Label floatLocationLabel = new Label(locGroup, SWT.NONE);
		floatLocationLabel.setText("Float Location:");
		{
			GridData gd = new GridData();
			floatLocationLabel.setLayoutData(gd);
		}
		
		floatLocation = new Label(locGroup, SWT.NONE);
		floatLocation.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 80;
			floatLocation.setLayoutData(gd);
		}
		
		Label headingLabel = new Label(locGroup, SWT.NONE);
		headingLabel.setText("Heading:");
		{
			GridData gd = new GridData();
			headingLabel.setLayoutData(gd);
		}
		
		heading = new Label(locGroup, SWT.NONE);
		heading.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 80;
			heading.setLayoutData(gd);
		}

		Label areaLabel = new Label(locGroup, SWT.NONE);
		areaLabel.setText("Area ID:");
		{
			GridData gd = new GridData();
			areaLabel.setLayoutData(gd);
		}
		
		area = new Label(locGroup, SWT.NONE);
		area.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 80;
			area.setLayoutData(gd);
		}
		
		Label collisionLabel = new Label(locGroup, SWT.NONE);
		collisionLabel.setText("Collision:");
		{
			GridData gd = new GridData();
			collisionLabel.setLayoutData(gd);
		}
		
		collision = new Label(locGroup, SWT.NONE);
		collision.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 80;
			collision.setLayoutData(gd);
		}
		
		Label velocityLabel = new Label(locGroup, SWT.NONE);
		velocityLabel.setText("Velocity:");
		{
			GridData gd = new GridData();
			velocityLabel.setLayoutData(gd);
		}
		
		velocity = new Label(locGroup, SWT.NONE);
		velocity.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 80;
			velocity.setLayoutData(gd);
		}

		Label rotationLabel = new Label(locGroup, SWT.NONE);
		rotationLabel.setText("Rotation:");
		{
			GridData gd = new GridData();
			rotationLabel.setLayoutData(gd);
		}
		
		rotation = new Label(locGroup, SWT.NONE);
		rotation.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 80;
			rotation.setLayoutData(gd);
		}
		
		Label speedLabel = new Label(locGroup, SWT.NONE);
		speedLabel.setText("Speed:");
		{
			GridData gd = new GridData();
			speedLabel.setLayoutData(gd);
		}
		
		speed = new Label(locGroup, SWT.NONE);
		speed.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 80;
			speed.setLayoutData(gd);
		}
		
		Label carryLabel = new Label(locGroup, SWT.NONE);
		carryLabel.setText("Carry:");
		{
			GridData gd = new GridData();
			carryLabel.setLayoutData(gd);
		}
		
		carry = new Label(locGroup, SWT.NONE);
		carry.setText("-");
		{
			GridData gd = new GridData();
			gd.widthHint = 80;
			carry.setLayoutData(gd);
		}
		
		updatePlayerList();
		updateButtons();		
	}
	
	void selectPlayer(Player player) {
		selectedPlayer = player;
		m_AgentTable.setSelection(players.indexOf(player));
		updateButtons();
		updateSensors();
	}
	
	void agentEvent() {
		updatePlayerList();
		updateButtons();
	}

	void worldChangeEvent() {
		if (selectedPlayer != null) {
			updateSensors();
		}
		
		for (int i = 0; i < m_Items.length; ++i) {
			m_Items[i].setText(1, Integer.toString(players.get(i).getPoints()));
		}
	}
	
	void updateSensors() {
		assert selectedPlayer != null;
		int [] gl = players.getLocation(selectedPlayer);
		double [] fl = players.getFloatLocation(selectedPlayer);
		NumberFormat nf = NumberFormat.getInstance();
		nf.setMaximumFractionDigits(2);
		gridLocation.setText("(" + nf.format(gl[0]) + "," + nf.format(gl[1]) + ")");
		floatLocation.setText("(" + nf.format(fl[0]) + "," + nf.format(fl[1]) + ")");

		heading.setText(nf.format(selectedPlayer.getHeadingRadians()));

		area.setText(Integer.toString(selectedPlayer.getLocationId()));
		String col = selectedPlayer.getCollisionX() ? "true" : "false";
		col += selectedPlayer.getCollisionY() ? ",true" : ",false";
		collision.setText(col);
		String vel = nf.format(selectedPlayer.getVelocity()[0]) + "," + nf.format(selectedPlayer.getVelocity()[1]);
		velocity.setText(vel);
		rotation.setText(nf.format(selectedPlayer.getRotationSpeed()));
		speed.setText(nf.format(Math.sqrt((selectedPlayer.getVelocity()[0] * selectedPlayer.getVelocity()[0]) + (selectedPlayer.getVelocity()[1] * selectedPlayer.getVelocity()[1]))));
		
		carry.setText(selectedPlayer.getCarryType());
	}
	
	void disableSensors() {
		gridLocation.setText("-");
		floatLocation.setText("-");
		heading.setText("-");
		area.setText("-");
		collision.setText("-");
		velocity.setText("-");
		rotation.setText("-");
		speed.setText("-");
		carry.setText("-");
	}
	
	void updatePlayerList() {
		players = Soar2D.simulation.world.getPlayers();
		m_AgentTable.removeAll();
		boolean foundSelected = false;
		
		m_Items = new TableItem[players.numberOfPlayers()];
		for (int i = 0; i < players.numberOfPlayers(); ++i) {
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
			disableSensors();
		}
	}
	
	void updateButtons() {
		boolean running = Soar2D.control.isRunning();
		boolean slotsAvailable = Soar2D.simulation.getUnusedColors().size() > 0;
		boolean hasPlayers = players.numberOfPlayers() > 0;
		boolean haveSelected = (selectedPlayer != null);
		boolean selectedIsMutable = haveSelected && !selectedPlayer.getName().equals("cat") && !selectedPlayer.getName().equals("dog");
		
		m_NewAgentButton.setEnabled(!running && slotsAvailable);
		m_CloneAgentButton.setEnabled(!running && slotsAvailable && selectedIsMutable );
		m_DestroyAgentButton.setEnabled(!running && hasPlayers && haveSelected);
		m_ReloadProductionsButton.setEnabled(!running && hasPlayers && haveSelected && selectedIsMutable);
 	}
}
