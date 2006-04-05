package tanksoar;

import simulation.*;
import utilities.*;

public class TankSoarCell extends Cell {
	// background
	private static final int kWallInt = 0;
	private static final int kOpenInt = 1;
	private static final int kEnergyInt = 2;
	private static final int kHealthInt = 3;
	
	// contents
	private static final int kNothingInt = 0;
	private static final int kTankInt = 1;
	private static final int kMissilePackInt = 2;

	private Tank m_Tank;
	private int m_Contents = 0;
	
	// used in path searching
	private boolean m_Explored = false;
	private MapPoint m_Parent = null;
	
	private boolean m_Explosion = false;
	protected boolean m_RadarTouch = false;

	public TankSoarCell(String name) throws Exception {
		if (name.equalsIgnoreCase(TankSoarWorld.kTypeWall)) {
			m_Type = kWallInt;
			return;
		} else if (name.equalsIgnoreCase(TankSoarWorld.kTypeEmpty)) {
			m_Type = kOpenInt;			
			return;
		} else if (name.equalsIgnoreCase(TankSoarWorld.kTypeEnergyRecharger)) {
			m_Type = kEnergyInt;			
			return;
		} else if (name.equalsIgnoreCase(TankSoarWorld.kTypeHealthRecharger)) {
			m_Type = kHealthInt;			
			return;
		} else {	
			throw new Exception("Invalid type name: " + name);
		}
	}

	boolean isExplored() {
		return m_Explored;
	}
	
	void setExplored(boolean setting) {
		m_Explored = setting;
	}
	
	MapPoint getParent() {
		return m_Parent;
	}
	
	void setParent(MapPoint parent) {
		m_Parent = parent;
	}
	
	public boolean isBlocked() {
		return (m_Type == kWallInt) || (m_Contents == kTankInt);
	}
	
	public boolean isWall() {
		return m_Type == kWallInt;
	}
	
	public boolean isOpen() {
		return m_Type == kOpenInt;
	}
	
	public boolean isEnergyRecharger() {
		return m_Type == kEnergyInt;
	}
	
	public boolean isHealthRecharger() {
		return m_Type == kHealthInt;
	}
	
	public boolean containsTank() {
		return m_Contents == kTankInt;
	}
	
	public boolean hasContents() {
		return m_Contents != kNothingInt;
	}
	
	public void setTank(Tank tank) {
		m_Modified = true;
		m_Contents = kTankInt;
		m_Tank = tank;
	}
	
	void setModified() {
		m_Modified = true;
	}
	
	void setRadarTouch() {
		m_RadarTouch = true;
		m_Modified = true;
	}
	
	public Tank getTank() {
		return m_Tank;
	}
	
	public boolean removeTank() {
		if (m_Contents != kTankInt) {
			return false;
		}
		m_Modified = true;
		m_Contents = kNothingInt;
		m_Tank = null;
		return true;
	}
	
	public boolean containsMissilePack() {
		return m_Contents == kMissilePackInt;
	}
	
	void setHealth() {
		m_Modified = true;
		m_Type = kHealthInt;
	}
	
	void setEnergy() {
		m_Modified = true;
		m_Type = kEnergyInt;
	}
	
	void setMissilePack() {
		m_Modified = true;
		m_Contents = kMissilePackInt;
	}
	
	void setExplosion() {
		m_Modified = true;
		m_Explosion = true;
	}
	
	public boolean isExplosion() {
		return m_Explosion;
	}

	public void clearModified() {
		m_Explosion = false;
		if (m_RadarTouch) {
			setModified();
			m_RadarTouch = false;
		} else {
			super.clearModified();
		}
	}
}

