package	soaruorc;

import orc.*;
import orc.util.*;

public class uOrcThread extends Thread
{
	private Orc orc;
	private Motor leftMotor;
	private Motor rightMotor;

	private class BotState
	{
		public double left = 0;
		public double right = 0;
	}
	private BotState state = new BotState();
	
	private boolean running = true;

	public uOrcThread()
	{
		orc = Orc.makeOrc();
		leftMotor = new Motor( orc, 1, true );
		rightMotor = new Motor( orc, 0, false );
	}
	
	public void run()
	{
		System.out.printf( "%15s %15s %15s %15s\n", "left", "right", "left current", "right current" );
		while ( running ) {
			double left, right;
			synchronized ( state )
			{
				left = state.left;
				right = state.right;
			}
			System.out.printf( "%15f %15f %15f %15f\r", left, right, leftMotor.getCurrent(), rightMotor.getCurrent() );
			
			// write new commands
			leftMotor.setPWM( left );
			rightMotor.setPWM( right );
			
			try {
				Thread.sleep( 30 );
			} catch ( InterruptedException ex ) 
			{}
		}
	}
	
	public void setPower( double left, double right )
	{
		synchronized ( state )
		{
			state.left = left;
			state.right = right;
		}
	}
	
	public void stopThread()
	{
		running = false;
	}
}
