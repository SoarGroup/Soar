package simulation;

import org.eclipse.swt.graphics.*;

import sml.*;
import utilities.*;

public class WorldEntity {
	public final static String kNorth = "north";
	public final static String kEast = "east";
	public final static String kSouth = "south";
	public final static String kWest = "west";
	
	public final static int kNorthInt = 1;
	public final static int kEastInt = 2;
	public final static int kSouthInt = 4;
	public final static int kWestInt = 8;
	
	protected Logger m_Logger = Logger.logger;
	protected Agent m_Agent;	
	protected Color m_Color;
	protected String m_Facing;
	protected int m_FacingInt;
	
	private String m_Name;
	private int m_Points = 0;
	private Point m_Location;
	private String m_ColorString;
	private String m_Productions;
	private boolean m_Colliding = false;
	
	public WorldEntity(Agent agent, String productions, String color, Point location) {
		m_Agent = agent;
		m_Location = location;
		m_ColorString = color;
		m_Productions = productions;

		m_Name = m_Agent.GetAgentName();
		m_Logger.log("Created agent: " + m_Name);
	}
	
	public boolean isColliding() {
		return m_Colliding;
	}
	
	public void setColliding(boolean colliding) {
		m_Colliding = colliding;
	}
	
	public String getProductions() {
		return m_Productions;
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
		m_Agent.InitSoar();
	}
	
	public Point getLocation() {
		return m_Location;
	}
	
	public void adjustPoints(int delta) {
		m_Points += delta;
	}
	public void setLocation(Point location) {
		m_Location = location;
	}
	
	public String getFacing() {
		return m_Facing;
	}
	
	protected void setFacingInt() {
		if (m_Facing == null) {
			m_FacingInt = 0;
		}
		
		if (m_Facing.equalsIgnoreCase(WorldEntity.kNorth)) {
			m_FacingInt = kNorthInt;
		} else if (m_Facing.equalsIgnoreCase(WorldEntity.kSouth)) {
			m_FacingInt = kSouthInt;
		} else if (m_Facing.equalsIgnoreCase(WorldEntity.kEast)) {
			m_FacingInt = kEastInt;
		} else if (m_Facing.equalsIgnoreCase(WorldEntity.kWest)) {
			m_FacingInt = kWestInt;
		}					
	}

	public int getFacingInt() {
		return m_FacingInt;
	}
}
