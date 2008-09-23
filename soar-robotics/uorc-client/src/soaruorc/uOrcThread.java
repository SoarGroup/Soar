package	soaruorc;

import orc.*;
import orc.util.*;

public class uOrcThread extends Thread
{
	private Orc orc;
	private Motor leftMotor;
	private Motor rightMotor;

	private double left;
	private double right;
	
	private running = true;

	public uOrcThread()
	{
		orc = Orc.makeOrc();
		leftMotor = new Motor( orc, 1, true );
		rightMotor = new Motor( orc, 0, false );
	}
	
	public void run()
	{
		//System.out.printf( "%15s %15s %15s %15s\n", "left", "right", "left current", "right current" );
		while ( running ) {
			//System.out.printf( "%15f %15f %15f %15f\r", left, right, leftMotor.getCurrent(), rightMotor.getCurrent() );
			
			// write new commands
			leftMotor.setPWM( left );
			rightMotor.setPWM(right );
	
			try {
				Thread.sleep( 30 );
			} catch (InterruptedException ex) 
			{}
		}
	}
	
	public void setPower( double left, double right )
	{
		synchronized
		{
			this.left = left;
			this.right = right;
		}
	}
	
	public void stop()
	{
		running = false;
	}
}
