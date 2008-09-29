package	soaruorc;

import orc.*;
import orc.util.*;

public class uOrcThread extends Thread
{
	private Orc orc;
	private Motor leftMotor;
	private Motor rightMotor;
	private QuadratureEncoder leftOdom;
	private QuadratureEncoder rightOdom;

	public class BotState
	{
		// command input
		public double left = 0;
		public double right = 0;
		
		// state output
		public double leftCurrent = 0;
		public int leftPosition = 0;
		public double leftVelocity = 0;
		
		public double rightCurrent = 0;
		public int rightPosition = 0;
		public double rightVelocity = 0;
	}
	private BotState state = new BotState();
	
	private boolean running = true;

	public uOrcThread()
	{
		orc = Orc.makeOrc();
		
		leftMotor = new Motor( orc, 1, true );
		leftOdom = new QuadratureEncoder( orc, 1, true);
		
		rightMotor = new Motor( orc, 0, false );
		rightOdom = new QuadratureEncoder( orc, 0, false);
	}
	
	public void run()
	{
		while ( running ) {
			double left, right;

			double leftCurrent = leftMotor.getCurrent();
			int leftPosition = leftOdom.getPosition();
			double leftVelocity = leftOdom.getVelocity();

			double rightCurrent = rightMotor.getCurrent();
			int rightPosition = rightOdom.getPosition();
			double rightVelocity = rightOdom.getVelocity();
			
			synchronized ( state )
			{
				left = state.left;
				right = state.right;
				
				state.leftCurrent = leftCurrent;
				state.leftPosition = leftPosition;
				state.leftVelocity = leftVelocity;
				
				state.rightCurrent = rightCurrent;
				state.rightPosition = rightPosition;
				state.rightVelocity = rightVelocity;
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
	
	public BotState getBotState()
	{
		return state;
	}
	
	public void stopThread()
	{
		running = false;
	}
}
