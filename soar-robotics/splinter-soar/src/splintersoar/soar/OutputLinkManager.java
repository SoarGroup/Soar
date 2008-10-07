package splintersoar.soar;

import sml.*;
import splintersoar.*;

public class OutputLinkManager {
	
	Agent agent;
	
	SplinterState state;
	
	public OutputLinkManager( Agent agent, SplinterState state )
	{
		this.agent = agent;
		this.state = state;
	}

	public void update()
	{
		// process output
		for ( int i = 0; i < agent.GetNumberCommands(); ++i ) 
		{
			Identifier commandId = agent.GetCommand( i );
			String commandName = commandId.GetAttribute();
			
			if ( commandName.equals( "motor" ) )
			{
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
				
				synchronized ( state )
				{
					state.left = leftCommand;
					state.right = rightCommand;
				}
				commandId.AddStatusComplete();
				continue;
			}
			else if ( commandName.equals( "move" ) )
			{
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
					}
					
				}
				else if ( direction.equals( "forward" ) )
				{
					synchronized ( state )
					{
						state.left = throttle;
						state.right = throttle;
					}
					
				}
				else if ( direction.equals( "stop" ) )
				{
					synchronized ( state )
					{
						state.left = 0;
						state.right = 0;
					}
					
				}
				else
				{
					System.out.println( "Unknown direction on move command: " + commandId.GetParameterValue( "direction" ) );
					commandId.AddStatusError();
					continue;
				}
				
			}
			else if ( commandName.equals( "rotate" ) )
			{
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
					synchronized ( state )
					{
						state.left = throttle * -1;
						state.right = throttle;
					}
					
				}
				else if ( direction.equals( "right" ) )
				{
					synchronized ( state )
					{
						state.left = throttle;
						state.right = throttle * -1;
					}
					
				}
				else if ( direction.equals( "stop" ) )
				{
					synchronized ( state )
					{
						state.left = 0;
						state.right = 0;
					}
					
				}
				else
				{
					System.out.println( "Unknown direction on rotate command: " + commandId.GetParameterValue( "direction" ) );
					commandId.AddStatusError();
					continue;
				}

			}
			else if ( commandName.equals( "stop" ) )
			{
				synchronized ( state )
				{
					state.left = 0;
					state.right = 0;
				}
				
			}
			
			System.out.println( "Unknown command: " + commandName );
			commandId.AddStatusError();
		}
	}
}
