package soar2d.visuals;

import java.awt.geom.Point2D;
import java.text.NumberFormat;
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
		java.awt.Point gl = Soar2D.simulation.world.getLocation(selectedPlayer);
		Point2D.Double fl = Soar2D.simulation.world.getFloatLocation(selectedPlayer);
		NumberFormat nf = NumberFormat.getInstance();
		nf.setMaximumFractionDigits(2);
		gridLocation.setText("(" + nf.format(gl.x) + "," + nf.format(gl.y) + ")");
		floatLocation.setText("(" + nf.format(fl.x) + "," + nf.format(fl.y) + ")");

		heading.setText(nf.format(selectedPlayer.getHeadingRadians()));

		area.setText(Integer.toString(selectedPlayer.getLocationId()));
		String col = selectedPlayer.getCollisionX() ? "true" : "false";
		col += selectedPlayer.getCollisionY() ? ",true" : ",false";
		collision.setText(col);
		String vel = nf.format(selectedPlayer.getVelocity().x) + "," + nf.format(selectedPlayer.getVelocity().y);
		velocity.setText(vel);
		rotation.setText(nf.format(selectedPlayer.getRotationSpeed()));
		speed.setText(nf.format(Math.sqrt((selectedPlayer.getVelocity().x * selectedPlayer.getVelocity().x) + (selectedPlayer.getVelocity().y * selectedPlayer.getVelocity().y))));
		
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
