package simulation;

import utilities.*;

public class World {
	protected Logger m_Logger = Logger.logger;
	protected int m_WorldSize;

	public int getSize() {
		return this.m_WorldSize;
	}
	
	protected boolean isInBounds(MapPoint location) {
		return isInBounds(location.x, location.y);
	}

	protected boolean isInBounds(int x, int y) {
		return (x >= 0) && (y >= 0) && (x < m_WorldSize) && (y < m_WorldSize);
	}
}
