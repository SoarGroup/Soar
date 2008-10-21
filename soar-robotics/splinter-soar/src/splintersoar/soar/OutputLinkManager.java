package splintersoar.soar;

import java.util.Arrays;

import sml.*;
import splintersoar.orc.OrcInput;
import splintersoar.orc.OrcOutput;

public class OutputLinkManager {
	
	Agent agent;
	Waypoints waypoints;
	
	OrcInput splinterInput = new OrcInput();
	
	public OutputLinkManager( Agent agent, Waypoints waypoints )
	{
		this.agent = agent;
		this.waypoints = waypoints;
	}
	
	public void update( OrcOutput splinterOutput )
	{
		OrcInput newSplinterInput = null;
				
		// process output
		for ( int i = 0; i < agent.GetNumberCommands(); ++i ) 
		{
			Identifier commandId = agent.GetCommand( i );
			String commandName = commandId.GetAttribute();
			
			if ( commandName.equals( "motor" ) )
			{
				if ( newSplinterInput != null )
				{
					// This is a warning
					System.err.println( "Motor command received possibly overriding previous orders" );
				}
				
				double [] motorThrottle = { 0, 0 };
		
				try 
				{
					motorThrottle[0] = Double.parseDouble( commandId.GetParameterValue( "left" ) );
				} 
				catch ( NullPointerException ex )
				{
					System.err.println( "No left on motor command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.err.println( "Unable to parse left: " + commandId.GetParameterValue( "left" ) );
					commandId.AddStatusError();
					continue;
				}
				
				try 
				{
					motorThrottle[1] = Double.parseDouble( commandId.GetParameterValue( "right" ) );
				} 
				catch ( NullPointerException ex )
				{
					System.err.println( "No right on motor command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.err.println( "Unable to parse right: " + commandId.GetParameterValue( "right" ) );
					commandId.AddStatusError();
					continue;
				}
				
				motorThrottle[0] = Math.max( motorThrottle[0], -1.0 );
				motorThrottle[0] = Math.min( motorThrottle[0], 1.0 );
				
				motorThrottle[1] = Math.max( motorThrottle[1], -1.0 );
				motorThrottle[1] = Math.min( motorThrottle[1], 1.0 );
				
				System.out.format( "motor: %10s %10s%n", "left", "right" );
				System.out.format( "       %10.3f %10.3f%n", motorThrottle[0], motorThrottle[1] );

				newSplinterInput = new OrcInput( motorThrottle );
				
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "move" ) )
			{
				if ( newSplinterInput != null )
				{
					System.err.println( "Move command received but motors already have orders" );
					commandId.AddStatusError();
					continue;
				}
				
				String direction = commandId.GetParameterValue( "direction" );		
				if ( direction == null )
				{
					System.err.println( "No direction on move command" );
					commandId.AddStatusError();
					continue;
				}
				
				double throttle = 0;
				try 
				{
					throttle = Double.parseDouble( commandId.GetParameterValue( "throttle" ) );
				} 
				catch ( NullPointerException ex )
				{
					System.err.println( "No throttle on move command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.err.println( "Unable to parse throttle: " + commandId.GetParameterValue( "throttle" ) );
					commandId.AddStatusError();
					continue;
				}
				
				throttle = Math.max( throttle, 0 );
				throttle = Math.min( throttle, 1.0 );
				
				System.out.format( "move: %10s %10s%n", "direction", "throttle" );
				System.out.format( "      %10s %10.3f%n", direction, throttle );

				if ( direction.equals( "backward" ) )
				{
					newSplinterInput = new OrcInput( throttle * -1 );
				}
				else if ( direction.equals( "forward" ) )
				{
					newSplinterInput = new OrcInput( throttle );
				}
				else if ( direction.equals( "stop" ) )
				{
					newSplinterInput = new OrcInput( 0 );
				}
				else
				{
					System.err.println( "Unknown direction on move command: " + commandId.GetParameterValue( "direction" ) );
					commandId.AddStatusError();
					continue;
				}
				
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "rotate" ) )
			{
				if ( newSplinterInput != null )
				{
					System.err.println( "Rotate command received but motors already have orders" );
					commandId.AddStatusError();
					continue;
				}
				
				String direction = commandId.GetParameterValue( "direction" );		
				if ( direction == null )
				{
					System.err.println( "No direction on rotate command" );
					commandId.AddStatusError();
					continue;
				}
				
				double throttle = 0;
				try 
				{
					throttle = Double.parseDouble( commandId.GetParameterValue( "throttle" ) );
				} 
				catch ( NullPointerException ex )
				{
					System.err.println( "No throttle on rotate command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.err.println( "Unable to parse throttle: " + commandId.GetParameterValue( "throttle" ) );
					commandId.AddStatusError();
					continue;
				}
				
				throttle = Math.max( throttle, 0 );
				throttle = Math.min( throttle, 1.0 );
				
				System.out.format( "rotate: %10s %10s%n", "direction", "throttle" );
				System.out.format( "        %10s %10.3f%n", direction, throttle );
				
				if ( direction.equals( "left" ) )
				{
					newSplinterInput = new OrcInput( OrcInput.Direction.left, throttle );
				}
				else if ( direction.equals( "right" ) )
				{
					newSplinterInput = new OrcInput( OrcInput.Direction.right, throttle );
				}
				else if ( direction.equals( "stop" ) )
				{
					newSplinterInput = new OrcInput( 0 );
				}
				else
				{
					System.err.println( "Unknown direction on rotate command: " + commandId.GetParameterValue( "direction" ) );
					commandId.AddStatusError();
					continue;
				}
				
				commandId.AddStatusComplete();
				continue;

			}
			else if ( commandName.equals( "rotate-to" ) )
			{
				if ( newSplinterInput != null )
				{
					System.err.println( "Rotate-to command received but motors already have orders" );
					commandId.AddStatusError();
					continue;
				}
				
				double yaw = 0;
				try 
				{
					yaw = Double.parseDouble( commandId.GetParameterValue( "yaw" ) );
				} 
				catch ( NullPointerException ex )
				{
					System.err.println( "No yaw on rotate-to command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.err.println( "Unable to parse yaw: " + commandId.GetParameterValue( "yaw" ) );
					commandId.AddStatusError();
					continue;
				}
				yaw = Math.toRadians( yaw );
				
				double tolerance = 0;
				try 
				{
					tolerance = Double.parseDouble( commandId.GetParameterValue( "tolerance" ) );
				} 
				catch ( NullPointerException ex )
				{
					System.err.println( "No tolerance on rotate-to command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.err.println( "Unable to parse tolerance: " + commandId.GetParameterValue( "tolerance" ) );
					commandId.AddStatusError();
					continue;
				}
				
				tolerance = Math.toRadians( tolerance );
				tolerance = Math.max( tolerance, 0 );
				tolerance = Math.min( tolerance, Math.PI );

				double throttle = 0;
				try 
				{
					throttle = Double.parseDouble( commandId.GetParameterValue( "throttle" ) );
				} 
				catch ( NullPointerException ex )
				{
					System.err.println( "No throttle on rotate-to command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.err.println( "Unable to parse throttle: " + commandId.GetParameterValue( "throttle" ) );
					commandId.AddStatusError();
					continue;
				}
				
				throttle = Math.max( throttle, 0 );
				throttle = Math.min( throttle, 1.0 );

				System.out.format( "rotate-to: %10s %10s %10s%n", "yaw", "tolerance", "throttle" );
				System.out.format( "           %10.3f %10.3f %10.3f%n", yaw, tolerance, throttle );
				
				newSplinterInput = new OrcInput( yaw, tolerance, throttle );
				
				commandId.AddStatusComplete();
				continue;

			}
			else if ( commandName.equals( "stop" ) )
			{
				if ( newSplinterInput != null )
				{
					// This is a warning
					System.err.println( "Stop command received, possibly overriding previous orders" );
				}

				System.out.format( "stop:%n" );
				
				newSplinterInput = new OrcInput( 0 );
				
				commandId.AddStatusComplete();
				continue;
				
			}
			else if ( commandName.equals( "add-waypoint" ) )
			{
				String id = commandId.GetParameterValue( "id" );
				if ( id == null )
				{
					System.err.println( "No id on add-waypoint command" );
					commandId.AddStatusError();
					continue;
				}
				
				double [] xyt = Arrays.copyOf( splinterOutput.xyt, splinterOutput.xyt.length );
				try 
				{
					xyt[0] = Double.parseDouble( commandId.GetParameterValue( "x" ) );
				} 
				catch ( NullPointerException ignored )
				{
					// no x param is ok, use current
				}
				catch ( NumberFormatException e ) 
				{
					System.err.println( "Unable to parse x: " + commandId.GetParameterValue( "x" ) );
					commandId.AddStatusError();
					continue;
				}

				try 
				{
					xyt[1] = Double.parseDouble( commandId.GetParameterValue( "y" ) );
				} 
				catch ( NullPointerException ignored )
				{
					// no y param is ok, use current
				}
				catch ( NumberFormatException e ) 
				{
					System.err.println( "Unable to parse y: " + commandId.GetParameterValue( "y" ) );
					commandId.AddStatusError();
					continue;
				}

				System.out.format( "add-waypoint: %16s %10s %10s%n", "id", "x", "y" );
				System.out.format( "              %16s %10.3f %10.3f%n", id, xyt[0], xyt[1] );
				
				waypoints.add( xyt, id );
				
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "remove-waypoint" ) )
			{
				String id = commandId.GetParameterValue( "id" );
				if ( id == null )
				{
					System.err.println( "No id on remove-waypoint command" );
					commandId.AddStatusError();
					continue;
				}
				
				System.out.format( "remove-waypoint: %16s%n", "id" );
				System.out.format( "                 %16s%n", id );

				if ( waypoints.remove( id ) == false )
				{
					System.err.println( "Unable to remove waypoint " + id + ", no such waypoint" );
					commandId.AddStatusError();
					continue;
				}
				
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "enable-waypoint" ) )
			{
				String id = commandId.GetParameterValue( "id" );
				if ( id == null )
				{
					System.err.println( "No id on enable-waypoint command" );
					commandId.AddStatusError();
					continue;
				}
				
				System.out.format( "enable-waypoint: %16s%n", "id" );
				System.out.format( "                 %16s%n", id );

				if ( waypoints.enable( id ) == false )
				{
					System.err.println( "Unable to enable waypoint " + id + ", no such waypoint" );
					commandId.AddStatusError();
					continue;
				}
				
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "disable-waypoint" ) )
			{
				String id = commandId.GetParameterValue( "id" );
				if ( id == null )
				{
					System.err.println( "No id on disable-waypoint command" );
					commandId.AddStatusError();
					continue;
				}
				
				System.out.format( "disable-waypoint: %16s%n", "id" );
				System.out.format( "                  %16s%n", id );

				if ( waypoints.disable( id ) == false )
				{
					System.err.println( "Unable to disable waypoint " + id + ", no such waypoint" );
					commandId.AddStatusError();
					continue;
				}
				
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "broadcast-message" ) )
			{
				System.err.println( "broadcast-message command not implemented, ignoring" );
				continue;
			}
			
			System.err.println( "Unknown command: " + commandName );
			commandId.AddStatusError();
		}
		
		if ( newSplinterInput != null )
		{
			synchronized( this )
			{
				splinterInput = newSplinterInput;
			}
		}
	}

	public OrcInput getSplinterInput() {
		synchronized( this )
		{
			return splinterInput.copy();
		}
	}
}
