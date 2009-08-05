package edu.umich.soar.gridmap2d.visuals;

import java.text.NumberFormat;

import jmat.LinAlg;

import lcmtypes.pose_t;

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

import edu.umich.soar.gridmap2d.CognitiveArchitecture;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.Robot;
import edu.umich.soar.gridmap2d.world.World;

public class RoomAgentDisplay extends AgentDisplay {

	static final int kTableHeight = 120;
	static final int kNameWidth = 75;
	static final int kScoreWidth = 55;
	
	World world;
	Group m_Group;
	Table m_AgentTable;
	Player selectedPlayer;
	TableItem[] m_Items = new TableItem[0];
	Player[] players;
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
	
	public RoomAgentDisplay(final Composite parent, World world, final CognitiveArchitecture cogArch) {
		super(parent);
		
		this.world = world;

		setLayout(new FillLayout());
		
		players = world.getPlayers();
		
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
//				try {
					Gridmap2D.simulation.destroyPlayer(selectedPlayer);
//				} catch (Exception ignored) {
//				}
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
		
		updatePlayerList();
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
			m_Items[i].setText(1, Integer.toString(players[i].getPoints()));
		}
	}
	
	void updateSensors() {
		assert selectedPlayer != null;
		Robot selectedRoomPlayer = (Robot)selectedPlayer;
		int [] gl = selectedRoomPlayer.getLocation();
		pose_t pose = selectedRoomPlayer.getState().getPose();
		NumberFormat nf = NumberFormat.getInstance();
		nf.setMaximumFractionDigits(2);
		gridLocation.setText("(" + nf.format(gl[0]) + "," + nf.format(gl[1]) + ")");
		floatLocation.setText("(" + nf.format(pose.pos[0]) + "," + nf.format(pose.pos[1]) + ")");

		heading.setText(nf.format(Math.toDegrees(selectedRoomPlayer.getState().getYaw())));

		area.setText(Integer.toString(selectedRoomPlayer.getState().getLocationId()));
		String col = selectedRoomPlayer.getState().isCollisionX() ? "true" : "false";
		col += selectedRoomPlayer.getState().isCollisionY() ? ",true" : ",false";
		collision.setText(col);
		String vel = nf.format(pose.vel[0]) + "," + nf.format(pose.vel[1]);
		velocity.setText(vel);
		rotation.setText(nf.format(pose.rotation_rate[2]));
		speed.setText(nf.format(LinAlg.magnitude(pose.vel)));
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
	}
	
	void updatePlayerList() {
		players = world.getPlayers();
		m_AgentTable.removeAll();
		boolean foundSelected = false;
		
		m_Items = new TableItem[players.length];
		for (int i = 0; i < players.length; ++i) {
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
			disableSensors();
		}
	}
	
	void updateButtons() {
		boolean running = Gridmap2D.control.isRunning();
		boolean slotsAvailable = Gridmap2D.simulation.getUnusedColors().size() > 0;
		boolean hasPlayers = players.length > 0;
		boolean haveSelected = (selectedPlayer != null);
		boolean selectedIsMutable = haveSelected && !selectedPlayer.getName().equals("cat") && !selectedPlayer.getName().equals("dog");
		
		m_NewAgentButton.setEnabled(!running && slotsAvailable);
		m_CloneAgentButton.setEnabled(!running && slotsAvailable && selectedIsMutable );
		m_DestroyAgentButton.setEnabled(!running && hasPlayers && haveSelected);
		m_ReloadProductionsButton.setEnabled(!running && hasPlayers && haveSelected && selectedIsMutable);
 	}
	
}
