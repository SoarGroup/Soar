package splintersoar;

import java.util.Arrays;

public class SplinterState {
	
	// command input
	public double [] throttle = { 0, 0 };
	public double targetYaw = 0;
	public double targetYawTolerance = 0;
	public boolean targetYawEnabled = false;
	
	// state output
	public long utime = 0;
	
	public int [] motorPosition = { 0, 0 };
	
	public double [] pos = { 0, 0, 0 };
	public double yaw = 0;
	
	public RangerData [] ranger;
	public long rangerutime = 0; // last time the ranger was updated
	
	// geometry, configuration
	public final double baselineMeters = 0.42545;
	public final double tickMeters = 0.0000429250;
	public final double length = 0.64;
	public final double width = 0.42;
	public final int rangerSlices = 5;
	
	public SplinterState()
	{}
	
	public SplinterState( SplinterState other )
	{
		System.arraycopy( other.throttle, 0, this.throttle, 0, other.throttle.length );
		this.targetYaw = other.targetYaw;
		this.targetYawTolerance = other.targetYaw;
		this.targetYawEnabled = other.targetYawEnabled;

		this.utime = other.utime;

		System.arraycopy( other.motorPosition, 0, this.motorPosition, 0, other.motorPosition.length );

		if ( other.ranger == null )
		{
			this.ranger = null;
		}
		else
		{
			this.ranger = Arrays.copyOf( other.ranger, other.ranger.length );
		}
		this.rangerutime = other.rangerutime;
		
		if ( other.pos == null )
		{
			other.pos = new double[3];
		}
		this.pos = Arrays.copyOf( other.pos, other.pos.length );
		this.yaw = other.yaw;
	}
	
}
