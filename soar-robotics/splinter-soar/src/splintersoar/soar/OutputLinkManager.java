package splintersoar.soar;

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

	public void update()
	{
		boolean motorsCommanded = false;
		
		//System.out.print( "." );
		
		// process output
		for ( int i = 0; i < agent.GetNumberCommands(); ++i ) 
		{
			Identifier commandId = agent.GetCommand( i );
			String commandName = commandId.GetAttribute();
			
			if ( commandName.equals( "motor" ) )
			{
				System.out.println( "motor" );
				if ( motorsCommanded )
				{
					// This is a warning
					System.out.println( "Motor command received possibly overriding previous orders" );
				}
				
				double leftCommand = 0;
				double rightCommand = 0;
		
				//System.out.print( "motor: " );
				try 
				{
					leftCommand = Double.parseDouble( commandId.GetParameterValue( "left" ) );
				} 
				catch ( NullPointerException ex )
				{
					System.out.println( "No left on motor command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.out.println( "Unable to parse left: " + commandId.GetParameterValue( "left" ) );
					commandId.AddStatusError();
					continue;
				}
				//System.out.print( leftCommand + " " );
				
				try 
				{
					rightCommand = Double.parseDouble( commandId.GetParameterValue( "right" ) );
				} 
				catch ( NullPointerException ex )
				{
					System.out.println( "No right on motor command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.out.println( "Unable to parse right: " + commandId.GetParameterValue( "right" ) );
					commandId.AddStatusError();
					continue;
				}
				//System.out.print( rightCommand + " " );
				
				leftCommand = Math.max( leftCommand, -1.0 );
				leftCommand = Math.min( leftCommand, 1.0 );
				
				rightCommand = Math.max( rightCommand, -1.0 );
				rightCommand = Math.min( rightCommand, 1.0 );
				
				//System.out.print( leftCommand + " " );
				//System.out.print( rightCommand + "\n" );
				
				//System.out.print( leftCommand + "                   \r" );
				synchronized ( state )
				{
					state.left = leftCommand;
					//System.out.println( "W: " + state.left );
					state.right = rightCommand;
					state.targetYawEnabled = false;
				}
				
				motorsCommanded = true;
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "move" ) )
			{
				if ( motorsCommanded )
				{
					System.out.println( "Move command received but motors already have orders" );
					commandId.AddStatusError();
					continue;
				}
				
				String direction = commandId.GetParameterValue( "direction" );		
				if ( direction == null )
				{
					System.out.println( "No direction on move command" );
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
					System.out.println( "No throttle on move command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.out.println( "Unable to parse throttle: " + commandId.GetParameterValue( "throttle" ) );
					commandId.AddStatusError();
					continue;
				}
				
				throttle = Math.max( throttle, 0 );
				throttle = Math.min( throttle, 1.0 );
				
				if ( direction.equals( "backward" ) )
				{
					synchronized ( state )
					{
						state.left = throttle * -1;
						state.right = throttle * -1;
						state.targetYawEnabled = false;
					}
					
				}
				else if ( direction.equals( "forward" ) )
				{
					System.out.println( "move forward" );
					synchronized ( state )
					{
						state.left = throttle;
						state.right = throttle;
						state.targetYawEnabled = false;
					}
					
				}
				else if ( direction.equals( "stop" ) )
				{
					synchronized ( state )
					{
						state.left = 0;
						state.right = 0;
						state.targetYawEnabled = false;
					}
					
				}
				else
				{
					System.out.println( "Unknown direction on move command: " + commandId.GetParameterValue( "direction" ) );
					commandId.AddStatusError();
					continue;
				}
				
				motorsCommanded = true;
				System.out.println( "move status complete" );
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "rotate" ) )
			{
				if ( motorsCommanded )
				{
					System.out.println( "Rotate command received but motors already have orders" );
					commandId.AddStatusError();
					continue;
				}
				
				String direction = commandId.GetParameterValue( "direction" );		
				if ( direction == null )
				{
					System.out.println( "No direction on rotate command" );
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
					System.out.println( "No throttle on rotate command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.out.println( "Unable to parse throttle: " + commandId.GetParameterValue( "throttle" ) );
					commandId.AddStatusError();
					continue;
				}
				
				throttle = Math.max( throttle, 0 );
				throttle = Math.min( throttle, 1.0 );
				
				if ( direction.equals( "left" ) )
				{
					System.out.println( "rotate left" );
					synchronized ( state )
					{
						state.left = throttle * -1;
						state.right = throttle;
						state.targetYawEnabled = false;
					}
					
				}
				else if ( direction.equals( "right" ) )
				{
					System.out.println( "rotate right" );
					synchronized ( state )
					{
						state.left = throttle;
						state.right = throttle * -1;
						state.targetYawEnabled = false;
					}
					
				}
				else if ( direction.equals( "stop" ) )
				{
					synchronized ( state )
					{
						state.left = 0;
						state.right = 0;
						state.targetYawEnabled = false;
					}
					
				}
				else
				{
					System.out.println( "Unknown direction on rotate command: " + commandId.GetParameterValue( "direction" ) );
					commandId.AddStatusError();
					continue;
				}
				
				motorsCommanded = true;
				System.out.println( "rotate status complete" );
				commandId.AddStatusComplete();
				continue;

			}
			else if ( commandName.equals( "rotate-to" ) )
			{
				if ( motorsCommanded )
				{
					System.out.println( "Rotate-to command received but motors already have orders" );
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
					System.out.println( "No yaw on rotate-to command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.out.println( "Unable to parse yaw: " + commandId.GetParameterValue( "yaw" ) );
					commandId.AddStatusError();
					continue;
				}
				
				double tolerance = 0;
				try 
				{
					tolerance = Double.parseDouble( commandId.GetParameterValue( "tolerance" ) );
				} 
				catch ( NullPointerException ex )
				{
					System.out.println( "No tolerance on rotate-to command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.out.println( "Unable to parse tolerance: " + commandId.GetParameterValue( "tolerance" ) );
					commandId.AddStatusError();
					continue;
				}

				tolerance = Math.max( tolerance, 0 );
				tolerance = Math.min( tolerance, 180 );
				
				double throttle = 0;
				try 
				{
					throttle = Double.parseDouble( commandId.GetParameterValue( "throttle" ) );
				} 
				catch ( NullPointerException ex )
				{
					System.out.println( "No throttle on rotate-to command" );
					commandId.AddStatusError();
					continue;
				}
				catch ( NumberFormatException e ) 
				{
					System.out.println( "Unable to parse throttle: " + commandId.GetParameterValue( "throttle" ) );
					commandId.AddStatusError();
					continue;
				}
				
				throttle = Math.max( throttle, 0 );
				throttle = Math.min( throttle, 1.0 );
				
				synchronized ( state )
				{
					state.targetYaw = yaw;
					state.targetYawTolerance = tolerance;
					state.targetYawEnabled = true;
					state.left = throttle;
					state.right = throttle;
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
					System.out.println( "Stop command received, possibly overriding previous orders" );
				}
				
				synchronized ( state )
				{
					state.left = 0;
					state.right = 0;
					state.targetYawEnabled = false;
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

				String name = commandId.GetParameterValue( "name" );
				if ( name == null )
				{
					System.out.println( "No name on add-waypoint command" );
					commandId.AddStatusError();
					continue;
				}
				
				double x = stateCopy.x;
				try 
				{
					x = Double.parseDouble( commandId.GetParameterValue( "x" ) );
				} 
				catch ( NullPointerException ignored )
				{
					// no x param is ok, use current
				}
				catch ( NumberFormatException e ) 
				{
					System.out.println( "Unable to parse x: " + commandId.GetParameterValue( "x" ) );
					commandId.AddStatusError();
					continue;
				}

				double y = stateCopy.y;
				try 
				{
					y = Double.parseDouble( commandId.GetParameterValue( "y" ) );
				} 
				catch ( NullPointerException ignored )
				{
					// no y param is ok, use current
				}
				catch ( NumberFormatException e ) 
				{
					System.out.println( "Unable to parse y: " + commandId.GetParameterValue( "y" ) );
					commandId.AddStatusError();
					continue;
				}

				waypoints.add( x, y, name );
				
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "remove-waypoint" ) )
			{
				String name = commandId.GetParameterValue( "name" );
				if ( name == null )
				{
					System.out.println( "No name on remove-waypoint command" );
					commandId.AddStatusError();
					continue;
				}
				
				if ( waypoints.remove( name ) == false )
				{
					System.out.println( "Unable to remove waypoint " + name + ", no such waypoint" );
					commandId.AddStatusError();
					continue;
				}
				
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "enable-waypoint" ) )
			{
				String name = commandId.GetParameterValue( "name" );
				if ( name == null )
				{
					System.out.println( "No name on enable-waypoint command" );
					commandId.AddStatusError();
					continue;
				}
				
				if ( waypoints.enable( name ) == false )
				{
					System.out.println( "Unable to enable waypoint " + name + ", no such waypoint" );
					commandId.AddStatusError();
					continue;
				}
				
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "disable-waypoint" ) )
			{
				String name = commandId.GetParameterValue( "name" );
				if ( name == null )
				{
					System.out.println( "No name on disable-waypoint command" );
					commandId.AddStatusError();
					continue;
				}
				
				if ( waypoints.disable( name ) == false )
				{
					System.out.println( "Unable to disable waypoint " + name + ", no such waypoint" );
					commandId.AddStatusError();
					continue;
				}
				
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "broadcast-message" ) )
			{
				System.out.println( "broadcast-message command not implemented, ignoring" );
				continue;
			}
			
			System.out.println( "Unknown command: " + commandName );
			commandId.AddStatusError();
		}
	}
}
