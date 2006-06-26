package tanksoar;

import simulation.*;
import utilities.*;
import sml.*;

public class Tank  extends WorldEntity {
	private final static String kMoveID = "move";
	private final static String kDirectionID = "direction";
	private final static String kBackwardID = "backward";
	private final static String kForwardID = "forward";
	public final static String kLeftID = "left";
	public final static String kRightID = "right";
	private final static String kFireID = "fire";
	private final static String kRadarID = "radar";
	private final static String kSwitchID = "switch";
	//private final static String kOff = "off";
	private final static String kOn = "on";
	private final static String kRadarPowerID = "radar-power";
	private final static String kSettingID = "setting";
	private final static String kShieldsID = "shields";
	private final static String kRotateID = "rotate";

	private TankSoarCell[][] radarCells = new TankSoarCell[kRadarWidth][kRadarHeight];

	public final static int kRadarWidth = 3;
	public final static int kRadarHeight = 14;
	
	private final static int kSheildEnergyUsage = 20;
	private final static int kMissileHealthDamage = 400;
	private final static int kMissileEnergyDamage = 250;
	private final static int kCollisionHealthDamage = 100;
	
	private final static int kMaximumEnergy = 1000;
	private final static int kMaximumHealth = 1000;
	
	private RelativeDirections m_RD = new RelativeDirections();
	private TankSoarWorld m_World;

	private String m_InitialFacing;
	private MapPoint m_InitialLocation;
	
	private InputLinkManager m_ILM;
	private MoveInfo m_LastMove;
	private int m_RWaves;
	private int m_Missiles;
	private boolean m_ShieldStatus;
	private int m_Health;
	private int m_Energy;
	private int m_RadarPower;
	private boolean m_RadarSwitch;
	private int m_RadarDistance;
	private int m_SmellDistance;
	private String m_SmellColor;
	
	private int m_InitialEnergy = 1000;
	private int m_InitialHealth = 1000;
	private int m_InitialMissiles = 15;
	

	public Tank(Agent agent, String productions, String color, MapPoint location, String facing, int energy, int health, int missiles, TankSoarWorld world) {
		super(agent, productions, color, location);
		
		m_World = world;
		
		if (facing == null) {			
			facing = WorldEntity.kNorth;
		}
		m_InitialFacing = facing;		
		m_InitialLocation = location;
		
		if (energy != -1) {
			m_InitialEnergy = energy;
		}
		if (health != -1) {
			m_InitialHealth = health;
		}
		if (missiles != -1) {
			m_InitialMissiles = missiles;
		}
	
		m_LastMove = new MoveInfo();
		if (m_Agent != null) {
			m_ILM = new InputLinkManager(m_World, this);
		}
		
		reset();
	}
	
	public MapPoint getInitialLocation() {
		return m_InitialLocation;
	}
	
	public void reset() {
		m_RadarPower = 0;
		m_RadarDistance = 0;
		m_SmellDistance = 0;
		m_SmellColor = null;
		m_RadarSwitch = false;
		m_RWaves = 0;
		m_Missiles = m_InitialMissiles;
		m_ShieldStatus = false;
		m_Health = m_InitialHealth;
		m_Energy = m_InitialEnergy;
				
		m_LastMove.reset();	
		
		m_Facing = m_InitialFacing;
		setFacingInt();
		m_RD.calculate(getFacingInt());
		
		clearRadar();
		
		if (m_Agent != null) {
			m_ILM.clear();
		}
	}
	
	public boolean getRadarStatus() {
		return m_RadarSwitch;
	}
	
	public int getRadarSetting() {
		return m_RadarPower;
	}
	
	// TODO: possibly move calculation in to this class
	void setSmell(int distance, String color) {
		m_SmellDistance = distance;
		m_SmellColor = color;
	}
	
	public TankSoarCell[][] getRadarCells() {
		return radarCells;
	}
	
	public int getRadarDistance() {
		return m_RadarDistance;
	}
	
	public int getSmellDistance() {
		return m_SmellDistance;
	}
	
	public String getSmellColor() {
		return m_SmellColor;
	}
	
	public void humanInput(MoveInfo move) {
		assert move != null;
		
		m_LastMove.reset();
		m_LastMove = move;
		m_RWaves = 0;
		
		// Must check missile count now
		if (m_LastMove.fire) {
			if (m_Missiles <= 0) {
				m_Logger.log(getName() + ": Attempted to fire missle with no missiles.");	
				m_LastMove.fire = false;
			}
		}

		// Shields must be handled pronto
		if (m_LastMove.shields) {
			handleShields();
		}
		
		if (m_LastMove.rotate) {
			rotate(m_LastMove.rotateDirection);
			// Do not allow a move if we rotated.
			if (m_LastMove.move) {
				m_Logger.log("Tried to move with a rotation, rotating only.");
				m_LastMove.move = false;
			}
		}
	}
	
	public void readOutputLink() {
		m_LastMove.reset();
		m_RWaves = 0;
		
		assert m_Agent != null;
		int numberOfCommands = m_Agent.GetNumberCommands();
		if (numberOfCommands == 0) {
			m_Logger.log(getName() + " issued no command.");
			return;
		}
		Identifier moveId = null;
		
		for (int i = 0; i < numberOfCommands; ++i) {
			Identifier commandId = m_Agent.GetCommand(i);
			String commandName = commandId.GetAttribute();

			if (commandName.equalsIgnoreCase(kMoveID)) {
				if (m_LastMove.move == true) {
					m_Logger.log(getName() + ": Detected more than one move command on output link, ignoring.");
					commandId.AddStatusError();
					continue;
				}

				String moveDirection = commandId.GetParameterValue(kDirectionID);
				if (moveDirection == null) {
					m_Logger.log(getName() + ": null move direction.");
					commandId.AddStatusError();
					continue;
				}
				
				if (moveDirection.equalsIgnoreCase(kForwardID)) {
					m_LastMove.moveDirection = m_RD.forward;
				} else if (moveDirection.equalsIgnoreCase(kBackwardID)) {
					m_LastMove.moveDirection = m_RD.backward;
				} else if (moveDirection.equalsIgnoreCase(kLeftID)) {
					m_LastMove.moveDirection = m_RD.left;
				} else if (moveDirection.equalsIgnoreCase(kRightID)) {
					m_LastMove.moveDirection = m_RD.right;
				} else {
					m_Logger.log(getName() + ": illegal move direction: " + moveDirection);
					commandId.AddStatusError();
					continue;
				}
				moveId = commandId;
				m_LastMove.move = true;
				
			} else if (commandName.equalsIgnoreCase(kFireID)) {
				if (m_LastMove.fire == true) {
					m_Logger.log(getName() + ": Detected more than one fire command on output link, ignoring.");
					commandId.AddStatusError();
					continue;
				}
				
		 		if (m_Missiles > 0) {
		 			m_LastMove.fire = true;
		 		} else {
					m_Logger.log(getName() + ": Attempted to fire missle with no missiles.");
					commandId.AddStatusError();
					continue;
				}
				// Weapon ignored
				
			} else if (commandName.equalsIgnoreCase(kRadarID)) {
				if (m_LastMove.radar == true) {
					m_Logger.log(getName() + ": Detected more than one radar command on output link, ignoring.");
					commandId.AddStatusError();
					continue;
				}
				
				String radarSwitch = commandId.GetParameterValue(kSwitchID);
				if (radarSwitch == null) {
					m_Logger.log(getName() + ": null radar switch.");
					commandId.AddStatusError();
					continue;
				}
				m_LastMove.radar = true;
				m_LastMove.radarSwitch = radarSwitch.equalsIgnoreCase(kOn) ? true : false;  
				
			} else if (commandName.equalsIgnoreCase(kRadarPowerID)) {
				if (m_LastMove.radarPower == true) {
					m_Logger.log(getName() + ": Detected more than one radar power command on output link, ignoring.");
					commandId.AddStatusError();
					continue;
				}
				
				String powerValue = commandId.GetParameterValue(kSettingID);
				if (powerValue == null) {
					m_Logger.log(getName() + ": null radar power value.");
					commandId.AddStatusError();
					continue;
				}
				
				try {
					m_LastMove.radarPowerSetting = Integer.decode(powerValue).intValue();
				} catch (NumberFormatException e) {
					m_Logger.log(getName() + ": Unable to parse radar power setting " + powerValue + ": " + e.getMessage());
					commandId.AddStatusError();
					continue;
				}
				m_LastMove.radarPower = true;
				
			} else if (commandName.equalsIgnoreCase(kShieldsID)) {
				if (m_LastMove.shields == true) {
					m_Logger.log(getName() + ": Detected more than one shields command on output link, ignoring.");
					commandId.AddStatusError();
					continue;
				}
				
				String shieldsSetting = commandId.GetParameterValue(kSwitchID);
				if (shieldsSetting == null) {
					m_Logger.log(getName() + ": null shields setting.");
					commandId.AddStatusError();
					continue;
				}
				m_LastMove.shields = true;
				m_LastMove.shieldsSetting = shieldsSetting.equalsIgnoreCase(kOn) ? true : false; 
				
				// Shields must be handled pronto
				handleShields();
				
			} else if (commandName.equalsIgnoreCase(kRotateID)) {
				if (m_LastMove.rotate == true) {
					m_Logger.log(getName() + ": Detected more than one rotate command on output link, ignoring.");
					commandId.AddStatusError();
					continue;
				}
				
				m_LastMove.rotateDirection = commandId.GetParameterValue(kDirectionID);
				if (m_LastMove.rotateDirection == null) {
					m_Logger.log(getName() + ": null rotation direction.");
					commandId.AddStatusError();
					continue;
				}
				
				m_LastMove.rotate = true;
				
				// Rotation must be handled pronto and never fails.
				if (m_LastMove.rotate) {
					rotate(m_LastMove.rotateDirection);
				}
				
			} else {
				m_Logger.log(getName() + ": Unknown command: " + commandName);
				commandId.AddStatusError();
				continue;
			}
			commandId.AddStatusComplete();
		}
		
    	m_Agent.ClearOutputLinkChanges();
		m_Agent.Commit();
		
		// Do not allow a move if we rotated.
		if (m_LastMove.rotate) {
			if (m_LastMove.move) {
				m_Logger.log("Tried to move with a rotation, rotating only.");
				assert moveId != null;
				moveId.AddStatusError();
				moveId = null;
				m_LastMove.move = false;
			}
		}
	}
	
	private void handleShields() {
		// Handle shields.
		boolean desiredShieldStatus = m_LastMove.shields ? m_LastMove.shieldsSetting : m_ShieldStatus;
		boolean enoughPowerForShields = m_Energy >= kSheildEnergyUsage;
		// If the agent wants to turn the shield on
		if (!m_ShieldStatus && desiredShieldStatus) {
			// Turn the shield on if there is enough power.
			if (enoughPowerForShields) {
				m_ShieldStatus = true;
			}
		}
		// If the agent wants the shield off or there isn't enough power
		if ((m_ShieldStatus && !desiredShieldStatus) || !enoughPowerForShields) {
			// Turn the shield off.
			m_ShieldStatus = false;
		}
	}
	
	public void updateSensors(TankSoarWorld world) {		
		TankSoarCell cell = world.getCell(getLocation());
		
		// Chargers
		if (m_Health > 0) {
			if (cell.isEnergyRecharger()) {
				m_Energy += 250;
				m_Energy = m_Energy > kMaximumEnergy ? kMaximumEnergy : m_Energy;
			}
			if (cell.isHealthRecharger()) {
				m_Health += 250;
				m_Health = m_Health > kMaximumHealth ? kMaximumHealth : m_Health;
			}
		}
		
		// Consume shield energy.
		if (m_ShieldStatus) {
			boolean enoughPowerForShields = m_Energy >= kSheildEnergyUsage;
			if (enoughPowerForShields) {
				m_Energy -= kSheildEnergyUsage;
			} else {
				m_ShieldStatus = false;
			}
		}
		
		// Handle radar.
		// Figure out desired radar power.
		int desiredRadarPower = m_LastMove.radarPower ? m_LastMove.radarPowerSetting : m_RadarPower;
		// Never exceed max power
		desiredRadarPower = desiredRadarPower > Tank.kRadarHeight ? Tank.kRadarHeight : desiredRadarPower;
		// Never exceed current energy usage
		desiredRadarPower = desiredRadarPower > m_Energy ? m_Energy : desiredRadarPower;
		// If the power setting changed, set it.
		if (m_RadarPower != desiredRadarPower) {
			m_RadarPower = desiredRadarPower;
		}
		
		// Figure out desired radar status
		boolean desiredRadarStatus = m_LastMove.radar ? m_LastMove.radarSwitch : m_RadarSwitch;
		// If the agent wants to turn the radar on
		if (!m_RadarSwitch && desiredRadarStatus) {
			// Turn the radar on.
			if (m_RadarPower > 0) {
				m_RadarSwitch = true;
			}
		}
		// If the agent wants to turn the radar off or there isn't enough power
		if ((m_RadarSwitch && !desiredRadarStatus) || (m_RadarPower == 0)) {
			// Turn the radar off.
			m_RadarSwitch = false;
		}
		// Consume radar energy.
		if (m_RadarSwitch) {
			m_Energy -= m_RadarPower;
			m_RadarDistance = scan(m_RadarPower);
		} else {
			clearRadar();
			m_RadarDistance = 0;
		}
		
		// Missiles
		if (m_LastMove.fire) {
			m_Missiles -= 1;
		}
	
	}
	
	private void clearRadar() {
		for (int i = 0; i < kRadarWidth; ++i) {
			for (int j = 0; j < kRadarHeight; ++j) {
				radarCells[i][j] = null;
			}
		}
	}
	
	private int scan(int setting) {
		MapPoint location = new MapPoint(getLocation());
		
		int distance = 0;
		for (int i = 0; i <= setting; ++i) {
			distance = i;
			if (scanCells(i, location) == true) {
				// Blocked
				break;
			}

			// Update for next distance
			location.travel(forward());
		}
		clearCells(distance, location);
		return distance;
	}
	
	static final int kRadarLeft = 0;
	static final int kRadarCenter = 1;
	static final int kRadarRight = 2;
	
	private boolean scanCells(int distance, MapPoint location) {
		for (int i = 0; i < Tank.kRadarWidth; ++i) {
			// Scan center then left then right
			int position = 0;
			int relativeDirection = 0;
			
			switch (i) {
			case 0:
				position = kRadarCenter;
				relativeDirection = 0;
				break;
			default:
			case 1:
				position = kRadarLeft;
				relativeDirection = left();
				break;
			case 2:
				position = kRadarRight;
				relativeDirection = right();
				break;
			}
			
			radarCells[position][distance] = m_World.getCell(location, relativeDirection);
			if (!(position == 1 && distance == 0)) {
				if (radarCells[position][distance].containsTank()) {
					radarCells[position][distance].getTank().setRWaves(backward());
				}
			}
			
			if ((position == kRadarCenter) && radarCells[position][distance].isBlocked()) {
				if ((position != kRadarCenter) || (distance != 0)) {
					return true;
				}
			}
			radarCells[position][distance].setRadarTouch();
		}
		return false;
	}
	
	private void clearCells(int initialDistance, MapPoint location) {
		for (int j = initialDistance; j < Tank.kRadarHeight; ++j) {
			for (int i = 0; i < Tank.kRadarWidth; ++i) {
				if ((i == 1) && (j == initialDistance)) {
					// skip first center
					continue;
				}
				radarCells[i][j] = null;
			}
		}
	}
	
	public void writeInputLink() {
		if (m_Agent != null) {
			m_ILM.write();
		}
	}
	
	public int forward() {
		return m_RD.forward;
	}
	
	public int backward() {
		return m_RD.backward;
	}
	
	public int left() {
		return m_RD.left;
	}
	
	public int right() {
		return m_RD.right;
	}
	
	public void addMissiles(int numberToAdd) {
		m_Missiles += numberToAdd;
	}
	
	public int getMissiles() {
		return m_Missiles;
	}
	
	public int getHealth() {
		return m_Health;
	}
	
	public void hitBy(Tank attacker, TankSoarCell currentCell) {
		// Missile damage
		if (m_ShieldStatus) {
			m_Energy -= kMissileEnergyDamage;
			if (m_Energy < 0) {
				m_Energy = 0;
			}
		} else {
			m_Health -= kMissileHealthDamage;
			if (m_Health < 0) {
				m_Health = 0;
			}
		}

		// Chargers
		if (m_Health > 0) {
			if (currentCell.isEnergyRecharger()) {
				// Instant death
				m_Health = 0;
			}
			if (currentCell.isHealthRecharger()) {
				// Instant death
				m_Health = 0;
			}
		}
		
		if (m_Health <= 0) {
			adjustPoints(TankSoarWorld.kKillPenalty);
			// TODO: multiple missiles!
			attacker.adjustPoints(TankSoarWorld.kKillAward);
		}
	}
	
	public void collide() {
		// Collision damage
		m_Health -= kCollisionHealthDamage;
		if (m_Health < 0) {
			m_Health = 0;
		}
		m_LastMove.move = false;
	}
	public int getEnergy() {
		return m_Energy;
	}
	
	public boolean getShieldStatus() {
		return m_ShieldStatus;
	}
	
	public boolean recentlyMovedOrRotated() {
		return m_LastMove.move || m_LastMove.rotate;
	}
	
	public boolean recentlyMoved() {
		return m_LastMove.move;
	}
	
	public boolean recentlyRotated() {
		return m_LastMove.rotate;
	}
	
	public int lastMoveDirection() {
		return m_LastMove.moveDirection;
	}
	
	private void rotate(String direction) {		
		int facing = 0;
		
		if (direction.equalsIgnoreCase(kLeftID)) {
			facing = m_RD.left;
		} else if (direction.equalsIgnoreCase(kRightID)) {
			facing = m_RD.right;
		}
		
		switch (facing) {
		case kNorthInt:
			m_Facing = kNorth;
			break;
		case kSouthInt:
			m_Facing = kSouth;
			break;
		case kEastInt:
			m_Facing = kEast;
			break;
		case kWestInt:
			m_Facing = kWest;
			break;
		}
		
		setFacingInt();
		m_RD.calculate(getFacingInt());
	}
	
	public boolean firedMissile() {
		return m_LastMove.fire;
	}
	
	public int getRWaves() {
		return m_RWaves;
	}
	
	void setRWaves(int fromDirection) {
		m_RWaves |= fromDirection;
	}
	
}
