package splintersoar;

public class SplinterState {
	
	// command input
	public double left = 0;
	public double right = 0;
	
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
	
	// geometry, configuration
	public final double baselineMeters = 0.42545;
	public final double tickMeters = 0.0000429250;
	
	public SplinterState()
	{}
	
	public SplinterState( SplinterState other )
	{
		this.left = other.left;
		this.right = other.right;

		this.utime = other.utime;

		this.leftCurrent = other.leftCurrent;
		this.leftPosition = other.leftPosition;
		this.leftVelocity = other.leftVelocity;

		this.rightCurrent = other.rightCurrent;
		this.rightPosition = other.rightPosition;
		this.rightVelocity = other.rightVelocity;
		
		this.x = other.x;
		this.y = other.y;
		this.yaw = other.yaw;
	}
	
}
