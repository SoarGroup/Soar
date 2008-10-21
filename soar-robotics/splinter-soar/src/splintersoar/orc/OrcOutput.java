package splintersoar.orc;

public class OrcOutput {
	// geometry, configuration
	public final double BASELINE_METERS = 0.383;
	public final double TICK_METERS = 0.000043225;
	public final double LENGTH_METERS = 0.64;
	public final double WIDTH_METERS = 0.42;

	public long utime = 0;
	public int [] motorPosition = { 0, 0 };
	public double [] xyt = { 0, 0, 0 };
	
	public OrcOutput( long utime ) {
		this.utime = utime;
	}
	
	public OrcOutput copy() 
	{
		OrcOutput other = new OrcOutput( utime );

		System.arraycopy( motorPosition, 0, other.motorPosition, 0, motorPosition.length );
		System.arraycopy( xyt, 0, other.xyt, 0, xyt.length );
		
		return other;
	}

}
