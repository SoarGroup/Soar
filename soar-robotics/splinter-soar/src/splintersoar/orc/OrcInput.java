package splintersoar.orc;

import java.util.Arrays;

public class OrcInput {
	public double [] throttle = { 0, 0 };
	public double targetYaw = 0;
	public double targetYawTolerance = 0;
	public boolean targetYawEnabled = false;
	
	public enum Direction { left, right };

	public OrcInput() {}

	public OrcInput( double throttle ) {
		Arrays.fill( this.throttle, throttle );
	}

	public OrcInput( Direction dir, double throttle ) {
		this.throttle[0] = throttle * ( dir == Direction.left ? -1 : 1 );
		this.throttle[1] = throttle * ( dir == Direction.right ? -1 : 1 );
	}

	public OrcInput( double [] throttle ) {
		System.arraycopy( throttle, 0, this.throttle, 0, throttle.length );
	}

	public OrcInput( double yaw, double tolerance, double throttle ) {
		this.targetYaw = yaw;
		this.targetYawTolerance = tolerance;
		this.targetYawEnabled = true;
		Arrays.fill( this.throttle, throttle );
	}

	public OrcInput copy()
	{
		OrcInput other = new OrcInput();
		
		System.arraycopy( throttle, 0, other.throttle, 0, throttle.length );
		other.targetYaw = targetYaw;
		other.targetYawTolerance = targetYaw;
		other.targetYawEnabled = targetYawEnabled;
		
		return other;
	}
}
