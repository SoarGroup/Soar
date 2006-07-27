package simulation;

import java.util.logging.*;

import sml.*;

public class WorldEntity {
	private static Logger logger = Logger.getLogger("simulation");
	protected Agent m_Agent;	
	protected org.eclipse.swt.graphics.Color m_Color;
	protected int m_FacingInt;
	
	private String m_Name;
	private int m_Points = 0;
	private java.awt.Point m_Location = new java.awt.Point(-1,-1);
	private String m_ColorString;
	private String m_Productions;
	private boolean m_Colliding = false;

	public WorldEntity(Agent agent, String productions, String color, java.awt.Point location) {
		m_Agent = agent;
		
		if (location != null) {
			m_Location.x = location.x;
			m_Location.y = location.y;
		}
		m_ColorString = color;
		m_Productions = productions;

		if (m_Agent == null) {
			m_Name = m_Productions;
		} else {
			m_Name = m_Agent.GetAgentName();
		}
		logger.fine("Created agent: " + m_Name);
	}
	
	public String getProductions() {
		return m_Productions;
	}
	
	public void reloadProductions() {
		if (m_Agent == null) {
			return;
		}
		m_Agent.LoadProductions(m_Productions, true);
	}
	
	public String getName() {
		return m_Name;
	}
	
	public int getPoints() {
		return m_Points;
	}
	
	public void setPoints(int score) {
		m_Points = score;
	}
	
	public String getColor() {
		return m_ColorString;
	}
	
	public Agent getAgent() {
		return m_Agent;
	}
	
	public void initSoar() {
		if (m_Agent == null) {
			return;
		}
		m_Agent.InitSoar();
	}
	
	public java.awt.Point getLocation() {
		return m_Location;
	}
	
	public void adjustPoints(int delta, String comment) {
		int previous = m_Points;
		m_Points += delta;
		logger.info(getName() + " score: " + Integer.toString(previous) + " -> " + Integer.toString(m_Points) + " (" + comment + ")");
	}
	public void setLocation(java.awt.Point location) {
		m_Location.x = location.x;
		m_Location.y = location.y;
	}
	
	public int getFacingInt() {
		return m_FacingInt;
	}

	public boolean isColliding() {
		return m_Colliding;
	}
	
	public void setColliding(boolean colliding) {
		m_Colliding = colliding;
	}
}
