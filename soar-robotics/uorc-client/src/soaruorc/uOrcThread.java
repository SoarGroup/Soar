package	soaruorc;

import orc.*;
import orc.util.*;

public class uOrcThread extends Thread
{
	private Orc orc;
	private Motor leftMotor;
	private Motor rightMotor;

	public class BotState
	{
		// command input
		public double left = 0;
		public double right = 0;
		
		// state output
		public double leftCurrent = 0;
		public double rightCurrent = 0;
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
		while ( running ) {
			double left, right;
			double leftCurrent = leftMotor.getCurrent();
			double rightCurrent = rightMotor.getCurrent();
			
			synchronized ( state )
			{
				left = state.left;
				right = state.right;
				state.leftCurrent = leftCurrent;
				state.rightCurrent = rightCurrent;
			}
			
			// write new commands
			leftMotor.setPWM( left );
			rightMotor.setPWM( right );
			
			try {
				Thread.sleep( 30 );
			} catch ( InterruptedException ex ) 
			{}
		}
	}
	
	public BotState getState()
	{
		return state;
	}
	
	public void stopThread()
	{
		running = false;
	}
}
