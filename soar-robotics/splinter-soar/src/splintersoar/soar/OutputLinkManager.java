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
			
			if ( commandName.equals( "move" ) )
			{
				double leftCommand = 0;
				double rightCommand = 0;
		
				//System.out.print( "move: " );
				try 
				{
					leftCommand = Double.parseDouble( commandId.GetParameterValue( "left" ) );
				} 
				catch ( NullPointerException ex )
				{
					System.out.println( "No left on move command" );
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
					System.out.println( "No right on move command" );
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
			
			System.out.println( "Unknown command: " + commandName );
			commandId.AddStatusError();
		}
	}
}
