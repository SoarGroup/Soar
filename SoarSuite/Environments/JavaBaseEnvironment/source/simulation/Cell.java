package simulation;

public class Cell {
	protected int m_Type;
	protected boolean m_Modified = true;
	private boolean m_Collision = false;
	
	public void setCollision(boolean setting) {
		m_Collision = setting;
		m_Modified = true;
	}
	
	public boolean checkCollision() {
		return m_Collision;
	}
	
	public void clearModified() {
		m_Modified = false;
	}
	
	public boolean isModified() {
		return m_Modified;
	}
	
	public String toString() {
		return Integer.toString(m_Type);
	}	
}
