package splintersoar.orc;

import splintersoar.*;
import java.util.*;

import orc.Motor;
import orc.Orc;
import orc.QuadratureEncoder;

public class OrcInterface implements SplinterInterface
{
	Timer timer = new Timer();
	static final int UPDATE_HZ = 30;
	
	SplinterState state = new SplinterState();
	
	private Orc orc;
	private Motor leftMotor;
	private Motor rightMotor;
	private QuadratureEncoder leftOdom;
	private QuadratureEncoder rightOdom;
	
	public OrcInterface()
	{
		orc = Orc.makeOrc();
		
		leftMotor = new Motor( orc, 1, true );
		leftOdom = new QuadratureEncoder( orc, 1, true);
		
		rightMotor = new Motor( orc, 0, false );
		rightOdom = new QuadratureEncoder( orc, 0, false);

		timer.schedule( new UpdateTask(), 0, 1000 / UPDATE_HZ );
	}
	
	@Override
	public SplinterState getState()
	{
		return state;
	}
	
	@Override
	public void shutdown()
	{
		timer.cancel();
	}
	
	class UpdateTask extends TimerTask
	{
		@Override
		public void run()
		{
			double left, right;

			// FIXME: each of the next get lines gets a new OrcStatus message. 
			// Really, we should only be getting one status message per update.
			// In the interest of not prematurely optimizing, I will leave this
			// like it is for now.
			long utime = orc.getStatus().utime;			

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
				
				state.utime = utime;

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
		}
	}
}
