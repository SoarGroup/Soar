package splintersoar;

import java.util.Arrays;

public class SplinterState {
	
	// command input
	public double left = 0;
	public double right = 0;
	public double targetYaw = 0;
	public double targetYawTolerance = 0;
	public boolean targetYawEnabled = false;
	
	// state output
	public long utime = 0;
	
	public double leftCurrent = 0;
	public int leftPosition = 0;
	public double leftVelocity = 0;
	
	public double rightCurrent = 0;
	public int rightPosition = 0;
	public double rightVelocity = 0;
	
	public double x = 0;
	public double y = 0;
	public double yaw = 0;
	
	public RangerData [] ranger;
	
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
		this.left = other.left;
		this.right = other.right;
		this.targetYaw = other.targetYaw;
		this.targetYawTolerance = other.targetYaw;
		this.targetYawEnabled = other.targetYawEnabled;

		this.utime = other.utime;

		this.leftCurrent = other.leftCurrent;
		this.leftPosition = other.leftPosition;
		this.leftVelocity = other.leftVelocity;

		this.rightCurrent = other.rightCurrent;
		this.rightPosition = other.rightPosition;
		this.rightVelocity = other.rightVelocity;

		if ( other.ranger != null )
		{
			this.ranger = Arrays.copyOf( other.ranger, other.ranger.length );
		}
		
		this.x = other.x;
		this.y = other.y;
		this.yaw = other.yaw;
	}
	
}
