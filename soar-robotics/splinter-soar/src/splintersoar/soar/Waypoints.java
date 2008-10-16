package splintersoar.soar;

import java.util.HashMap;
import java.util.Iterator;

import erp.geom.*;
import sml.*;

public class Waypoints {
	
	Agent agent;
	Identifier waypoints;
	double [] robotPosition = new double[3];
	double robotYaw;
	
	HashMap< String, Waypoint > waypointList = new HashMap< String, Waypoint >();
	
	class Waypoint
	{
		double [] pos = new double[3];
		String name;
		
		Identifier waypoint;
		FloatElement absRelativeBearing;
		FloatElement distance;
		FloatElement relativeBearing;
		FloatElement yaw;
		
		public Waypoint( double [] waypointPosition, String name )
		{
			System.arraycopy( waypointPosition, 0, this.pos, 0, waypointPosition.length );
			this.name = new String( name );
			
			createWmes();
			updateWmes();
		}
		
		String getName()
		{
			return name;
		}
		
		boolean equals( String other )
		{
			return other.equals( name );
		}
		
		void createWmes()
		{
			waypoint = agent.CreateIdWME( waypoints, "waypoint" );
			agent.CreateStringWME( waypoint, "id", name );
			agent.CreateFloatWME( waypoint, "x", pos[0] );
			agent.CreateFloatWME( waypoint, "y", pos[1] );
			
			distance = agent.CreateFloatWME( waypoint, "distance", 0 );
			yaw = agent.CreateFloatWME( waypoint, "yaw", 0 );
			relativeBearing = agent.CreateFloatWME( waypoint, "relative-bearing", 0 );
			absRelativeBearing = agent.CreateFloatWME( waypoint, "abs-relative-bearing", 0 );
		}
		
		void updateWmes()
		{
			double distanceValue = Geometry.distance( robotPosition, pos, 2 );
			agent.Update( distance, distanceValue );
			
			double [] delta = Geometry.subtract( pos, robotPosition );
			double yawValue = Math.atan2( delta[1], delta[0] );
			agent.Update( yaw, Math.toDegrees( yawValue ) );
			double relativeBearingValue = yawValue - robotYaw;
			
			if ( relativeBearingValue > Math.PI )
			{
				relativeBearingValue -= 2 * Math.PI;
			} else if ( relativeBearingValue < Math.PI * -1 )
			{
				relativeBearingValue += 2 * Math.PI;
			}
			
			agent.Update( relativeBearing, Math.toDegrees( relativeBearingValue ) );
			agent.Update( absRelativeBearing, Math.abs( Math.toDegrees( relativeBearingValue ) ) );
			
			System.out.format( "%16s %10.3f %10.3f %10.3f %10.3f %10.3f%n", name, pos[0], pos[1], distanceValue, Math.toDegrees( yawValue ), Math.toDegrees( relativeBearingValue ) );
		}
		
		void enable()
		{
			if ( waypoint != null )
			{
				return;
			}
			
			createWmes();
			updateWmes();
		}
		
		void disable()
		{
			if ( waypoint == null )
			{
				return;
			}
			
			agent.DestroyWME( waypoint );
			
			waypoint = null;
			absRelativeBearing = null;
			distance = null;
			relativeBearing = null;
			yaw = null;
		}
	}

	Waypoints( Agent agent )
	{
		this.agent = agent;
	}
	
	void setRootIdentifier( Identifier waypoints )
	{
		this.waypoints = waypoints;
	}

	public void add( double [] waypointPosition, String name ) 
	{
		Waypoint waypoint = waypointList.remove( name );
		if ( waypoint != null )
		{
			waypoint.disable();
		}

		waypointList.put( name, new Waypoint( waypointPosition, name ) );
	}

	public boolean remove( String name ) 
	{
		Waypoint waypoint = waypointList.remove( name );
		if ( waypoint == null )
		{
			return false;
		}
		waypoint.disable();
		return true;
	}

	public boolean enable( String name ) 
	{
		Waypoint waypoint = waypointList.get( name );
		if ( name == null )
		{
			return false;
		}
		
		waypoint.enable();
		return true;
	}

	public boolean disable( String name ) 
	{
		Waypoint waypoint = waypointList.get( name );
		if ( name == null )
		{
			return false;
		}
		
		waypoint.disable();
		return true;
	}
	
	public void setNewRobotPose( double [] robotPosition, double robotYaw )
	{
		System.arraycopy( robotPosition, 0, this.robotPosition, 0, robotPosition.length );
		this.robotYaw = robotYaw;
	}
	
	public void update()
	{
		if ( waypointList.size() == 0 )
		{
			return;
		}
		
		//System.out.format( "%16s %10s %10s %10s %10s %10s%n", "name", "x", "y", "distance", "yaw", "bearing" );
		Iterator< Waypoint > iter = waypointList.values().iterator();
		while ( iter.hasNext() )
		{
			Waypoint waypoint = iter.next();
			waypoint.updateWmes();
		}
	}
}
