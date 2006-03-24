package simulation;

import org.eclipse.swt.graphics.*;

import utilities.*;

public class World {
	protected Logger m_Logger = Logger.logger;
	protected int m_WorldWidth;
	protected int m_WorldHeight;

	public int getWidth() {
		return this.m_WorldWidth;
	}
	
	public int getHeight() {
		return this.m_WorldHeight;
	}
	
	protected boolean isInBounds(MapPoint location) {
		return isInBounds(location.x, location.y);
	}

	protected boolean isInBounds(int x, int y) {
		return (x >= 0) && (y >= 0) && (x < m_WorldWidth) && (y < m_WorldWidth);
	}
}
