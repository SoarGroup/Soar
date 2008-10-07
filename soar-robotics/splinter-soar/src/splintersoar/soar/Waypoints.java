package splintersoar.soar;

import sml.*;

public class Waypoints {
	
	Agent agent;
	Identifier waypoints;

	Waypoints( Agent agent )
	{
		this.agent = agent;
	}
	
	void setRootIdentifier( Identifier waypoints )
	{
		this.waypoints = waypoints;
	}

	public void add(double x, double y, String name) {
		// TODO Auto-generated method stub
		
	}

	public boolean remove(String name) {
		// TODO Auto-generated method stub
		return false;
	}

	public boolean enable(String name) {
		// TODO Auto-generated method stub
		return false;
	}

	public boolean disable(String name) {
		// TODO Auto-generated method stub
		return false;
	}
}
