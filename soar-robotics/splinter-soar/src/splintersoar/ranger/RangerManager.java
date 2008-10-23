package splintersoar.ranger;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.concurrent.locks.Lock;
import java.util.logging.Level;
import java.util.logging.Logger;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;
import splintersoar.LCMInfo;
import splintersoar.LogFactory;

public class RangerManager implements LCMSubscriber, RangerStateProducer 
{
	private LCM lcm;
	private laser_t laserDataCurrent;
	Lock lock;
	private Logger logger;
	
	public RangerManager()
	{
		logger = LogFactory.createSimpleLogger( "RangerManager", Level.INFO ); 
		
		lcm = LCM.getSingleton();
		lcm.subscribe( LCMInfo.LASER_FRONT_CHANNEL, this );
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
		if ( channel.equals( LCMInfo.LASER_FRONT_CHANNEL ) )
		{
			if ( lock.tryLock() == true )
			{
				try 
				{
					laserDataCurrent = new laser_t( ins );
				}
				catch ( IOException ex ) 
				{
					logger.warning( "Error decoding LASER_FRONT message: " + ex );
				}
				finally
				{
					lock.unlock();
				}
			}
		}
	}
}
