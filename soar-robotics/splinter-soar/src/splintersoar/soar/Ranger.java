package splintersoar.soar;

import sml.*;
import splintersoar.ranger.RangerState;

public class Ranger {
	
	Agent agent;
	Identifier ranges;
	long lastutime = 0;
	Range[] slices;
	
	class Range
	{
		int idNumber;
		
		Identifier range;
		FloatElement start;
		FloatElement end;
		FloatElement distance;

		Range( int id )
		{
			idNumber = id;
		}
		
		void createWmes()
		{
			range = agent.CreateIdWME( ranges, "range" );
			agent.CreateIntWME( range, "id", idNumber );
			
			start = agent.CreateFloatWME( range, "start", 0 );
			end = agent.CreateFloatWME( range, "end", 0 );
			distance = agent.CreateFloatWME( range, "distance", 0 );
		}
		
		void update( RangerState.RangerData data )
		{
			if ( range == null )
			{
				createWmes();
			}
			
			agent.Update( start, Math.toDegrees( data.start ) );
			agent.Update( end, Math.toDegrees( data.end ) );
			agent.Update( distance, data.distance );
		}
	}
	
	public Ranger( Agent agent, Identifier ranges, RangerState rangerState )
	{
		this.agent = agent;
		this.ranges = ranges;
		
		if ( rangerState != null )
		{
			createSlices( rangerState );
			updateSlices( rangerState );
		}
	}
	
	private void createSlices( RangerState rangerState )
	{
		assert rangerState != null;
		assert slices == null;

		slices = new Range[ rangerState.ranger.length ];
		for ( int index = 0; index < rangerState.ranger.length; ++index )
		{
			slices[ index ] = new Range( index - ( rangerState.ranger.length / 2 ) );
		}
	}
	
	public void update( RangerState rangerState )
	{
		if ( rangerState == null )
		{
			return;
		}
		
		if ( rangerState.utime == lastutime )
		{
			return;
		}
		lastutime = rangerState.utime;
		
		if ( slices == null )
		{
			createSlices( rangerState );
		}
		else
		{
			assert rangerState.ranger.length == slices.length;
		}
		
		updateSlices( rangerState );
	}
	
	private void updateSlices( RangerState rangerState )
	{
		for ( int index = 0; index < rangerState.ranger.length; ++index )
		{
			slices[ index ].update( rangerState.ranger[ index ] );
		}
	}
}
