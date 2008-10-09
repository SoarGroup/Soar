package splintersoar.soar;

import sml.*;
import splintersoar.RangerData;

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
		
		void update( RangerData data )
		{
			if ( range == null )
			{
				createWmes();
			}
			
			agent.Update( start, data.start );
			agent.Update( end, data.end );
			agent.Update( distance, data.distance );
		}
	}
	
	public Ranger( Agent agent, Identifier ranges, int numSlices )
	{
		this.agent = agent;
		this.ranges = ranges;
		
		slices = new Range[ numSlices ];
		for ( int index = 0; index < numSlices; ++index )
		{
			slices[ index ] = new Range( index - ( numSlices / 2 ) );
		}
	}
	
	public void update( long utime, RangerData[] data )
	{
		if ( utime == lastutime )
		{
			return;
		}
		lastutime = utime;
		
		assert data.length == slices.length;
		
		for ( int index = 0; index < data.length; ++index )
		{
			slices[ index ].update( data[ index ] );
		}
		
		//System.out.println( "data 4 dist (soar-side): " + data[ 4 ].distance );
	}
}
