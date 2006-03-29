package tanksoar;

import utilities.*;

public class Missile {
	private MapPoint m_CurrentLocation;
	private int m_FlightPhase; // 0, 1 == affects current location, 2 == affects current location + 1
	private int m_Direction;
	private Tank m_Owner;
	
	public Missile(MapPoint location, int direction, Tank owner) {
		m_CurrentLocation = location;
		m_Direction = direction;
		m_FlightPhase = 0;
		m_Owner = owner;
	}
	
	public Tank getOwner() {
		return m_Owner;
	}
	
	public MapPoint getCurrentLocation() {
		return m_CurrentLocation;
	}
	
	public int getDirection() {
		return m_Direction;
	}
	
	void incrementFlightPhase() {
		++m_FlightPhase;
		m_FlightPhase %= 3;
	}
}

