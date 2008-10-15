package splintersoar.soar;

import java.util.Arrays;

import sml.*;
import splintersoar.*;

public class OutputLinkManager {
	
	Agent agent;
	Waypoints waypoints;
	
	SplinterState state;
	
	public OutputLinkManager( Agent agent, Waypoints waypoints, SplinterState state )
	{
		this.agent = agent;
		this.waypoints = waypoints;
		this.state = state;
	}
	
	boolean overrideEnabled = false;
	public void setOverride( boolean enabled )
	{
		this.overrideEnabled = enabled;
	}

	public void update()
	{
		boolean motorsCommanded = false;
				
		// process output
		for ( int i = 0; i < agent.GetNumberCommands(); ++i ) 
		{
			Identifier commandId = agent.GetCommand( i );
			String commandName = commandId.GetAttribute();
			
			if ( commandName.equals( "motor" ) )
			{
				if ( motorsCommanded )
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
				if ( !overrideEnabled )
				{
					synchronized ( state )
					{
						System.arraycopy( motorThrottle, 0, state.throttle, 0, motorThrottle.length );
						state.targetYawEnabled = false;
					}
				}
				
				motorsCommanded = true;
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "move" ) )
			{
				if ( motorsCommanded )
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
					if ( !overrideEnabled )
					{
						synchronized ( state )
						{
							state.throttle = new double [] { throttle * -1, throttle * -1 };
							state.targetYawEnabled = false;
						}
					}					
				}
				else if ( direction.equals( "forward" ) )
				{
					if ( !overrideEnabled )
					{
						synchronized ( state )
						{
							state.throttle = new double [] { throttle, throttle };
							state.targetYawEnabled = false;
						}
					}					
				}
				else if ( direction.equals( "stop" ) )
				{
					if ( !overrideEnabled )
					{
						synchronized ( state )
						{
							state.throttle = new double [] { 0, 0 };
							state.targetYawEnabled = false;
						}
					}					
				}
				else
				{
					System.err.println( "Unknown direction on move command: " + commandId.GetParameterValue( "direction" ) );
					commandId.AddStatusError();
					continue;
				}
				
				motorsCommanded = true;
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "rotate" ) )
			{
				if ( motorsCommanded )
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
					if ( !overrideEnabled )
					{
						synchronized ( state )
						{
							state.throttle = new double [] { throttle * -1, throttle };
							state.targetYawEnabled = false;
						}
					}					
				}
				else if ( direction.equals( "right" ) )
				{
					if ( !overrideEnabled )
					{
						synchronized ( state )
						{
							state.throttle = new double [] { throttle, throttle * -1 };
							state.targetYawEnabled = false;
						}
					}					
				}
				else if ( direction.equals( "stop" ) )
				{
					if ( !overrideEnabled )
					{
						synchronized ( state )
						{
							state.throttle = new double [] { 0, 0 };
							state.targetYawEnabled = false;
						}
					}					
				}
				else
				{
					System.err.println( "Unknown direction on rotate command: " + commandId.GetParameterValue( "direction" ) );
					commandId.AddStatusError();
					continue;
				}
				
				motorsCommanded = true;
				commandId.AddStatusComplete();
				continue;

			}
			else if ( commandName.equals( "rotate-to" ) )
			{
				if ( motorsCommanded )
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
				
				if ( !overrideEnabled )
				{
					synchronized ( state )
					{
						state.targetYaw = yaw;
						state.targetYawTolerance = tolerance;
						state.targetYawEnabled = true;
						state.throttle = new double [] { throttle, throttle };
					}
				}
				
				motorsCommanded = true;
				commandId.AddStatusComplete();
				continue;

			}
			else if ( commandName.equals( "stop" ) )
			{
				if ( motorsCommanded )
				{
					// This is a warning
					System.err.println( "Stop command received, possibly overriding previous orders" );
				}

				System.out.format( "stop:%n" );
				
				if ( !overrideEnabled )
				{
					synchronized ( state )
					{
						state.throttle = new double [] { 0, 0 };
						state.targetYawEnabled = false;
					}
				}
				
				motorsCommanded = true;
				commandId.AddStatusComplete();
				continue;
				
			}
			else if ( commandName.equals( "add-waypoint" ) )
			{
				SplinterState stateCopy;
				synchronized ( state )
				{
					stateCopy = new SplinterState( state );
				}

				String id = commandId.GetParameterValue( "id" );
				if ( id == null )
				{
					System.err.println( "No id on add-waypoint command" );
					commandId.AddStatusError();
					continue;
				}
				
				double [] pos = Arrays.copyOf( stateCopy.pos, stateCopy.pos.length );
				try 
				{
					pos[0] = Double.parseDouble( commandId.GetParameterValue( "x" ) );
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
					pos[1] = Double.parseDouble( commandId.GetParameterValue( "y" ) );
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
				System.out.format( "              %16s %10.3f %10.3f%n", id, pos[0], pos[1] );
				
				waypoints.add( pos, id );
				
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
	}
}
