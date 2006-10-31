package simulation;

public class Cell {
	protected int m_Type;
	protected boolean m_Redraw = true;
	private boolean m_Collision = false;
	
	public void setCollision(boolean setting) {
		m_Collision = setting;
		m_Redraw = true;
	}
	
	public boolean checkCollision() {
		return m_Collision;
	}
	
	public void clearRedraw() {
		m_Redraw = false;
	}
	
	public boolean needsRedraw() {
		return m_Redraw;
	}
	
	public String toString() {
		return Integer.toString(m_Type);
	}	
}
