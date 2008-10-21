package splintersoar.ranger;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.concurrent.locks.Lock;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;
import splintersoar.SplinterSoar;

public class RangerManager implements LCMSubscriber, RangerStateProducer 
{
	public static final String LASER_FRONT = "LASER_FRONT";
	private LCM lcm;
	private laser_t laserDataCurrent;
	Lock lock;
	
	public RangerManager()
	{
		lcm = LCM.getSingleton();
		lcm.subscribe( LASER_FRONT, this );
	}
	
	@Override
	public RangerState getRangerState() {
		if ( laserDataCurrent == null )
		{
			return null;
		}
		
		laser_t laserDataCopy;
		lock.lock();
		try
		{
			laserDataCopy = laserDataCurrent.copy();
		}
		finally
		{
			lock.unlock();
		}
		
		return new RangerState( laserDataCopy );
	}

	@Override
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if ( channel.equals( LASER_FRONT ) )
		{
			if ( lock.tryLock() == true )
			{
				try 
				{
					laserDataCurrent = new laser_t( ins );
				}
				catch ( IOException ex ) 
				{
					SplinterSoar.logger.warning( "Error decoding LASER_FRONT message: " + ex );
				}
				finally
				{
					lock.unlock();
				}
			}
		}
	}
}
