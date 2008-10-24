package splintersoar.ranger;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;

import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;
import lcmtypes.laser_t;
import splintersoar.LCMInfo;
import splintersoar.LogFactory;

/**
 * @author voigtjr
 * Soar wants coarse ranger data, this class manages its production.
 */
public class RangerManager implements LCMSubscriber, RangerStateProducer {
	private LCM lcm;
	private laser_t laserDataCurrent;
	private Logger logger;

	public RangerManager() {
		logger = LogFactory.createSimpleLogger("RangerManager", Level.INFO);

		lcm = LCM.getSingleton();
		lcm.subscribe(LCMInfo.LASER_FRONT_CHANNEL, this);
	}

	@Override
	public RangerState getRangerState() {
		if (laserDataCurrent == null) {
			return null;
		}

		RangerState state = new RangerState(laserDataCurrent);
		if (logger.isLoggable(Level.FINEST)) {
			logger.finest(String.format("New ranger state: %5.2f %5.2f %5.2f %5.2f %5.2f", state.ranger[0].distance, state.ranger[1].distance,
					state.ranger[2].distance, state.ranger[3].distance, state.ranger[4].distance));
		}
		return state;
	}

	@Override
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(LCMInfo.LASER_FRONT_CHANNEL)) {
			try {
				laserDataCurrent = new laser_t(ins);
			} catch (IOException ex) {
				logger.warning("Error decoding LASER_FRONT message: " + ex);
			}
		}
	}
}
