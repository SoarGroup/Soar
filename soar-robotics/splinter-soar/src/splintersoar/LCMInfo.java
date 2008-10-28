package splintersoar;

import java.io.DataInputStream;
import java.io.IOException;

import splintersoar.lcmtypes.differential_drive_command_t;
import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;

/**
 * @author voigtjr
 * LCM constants.
 */
public class LCMInfo implements LCMSubscriber {
	public static final String DRIVE_COMMANDS_CHANNEL = "DRIVE_COMMANDS";
	public static final String LASER_LOC_CHANNEL = "LASER_LOC";
	public static final String COORDS_CHANNEL = "COORDS";
	public static final String SPLINTER_STATE_CHANNEL = "SPLINTER_STATE";
	public static final String LASER_FRONT_CHANNEL = "LASER_FRONT";
	public static final String WAYPOINTS_CHANNEL = "WAYPOINTS";
	public static final String PARTICLES_CHANNEL = "PARTICLES";

    static 
    {
        System.setProperty("java.net.preferIPv4Stack", "true");
        System.out.println("LC: Disabling IPV6 support");
    }

	public static final String TEST_CHANNEL_A = "TEST_CHANNEL";
	public static void main(String[] args) {
		LCM lcm = LCM.getSingleton();

		if (args.length < 1) {
			while (true) {
				differential_drive_command_t dc = new differential_drive_command_t();
				dc.utime = System.nanoTime() / 1000;
				lcm.publish(TEST_CHANNEL_A, dc);
				System.out.print(".");
				try {
					Thread.sleep(100);
				} catch (InterruptedException ignored) {
				}
			}
		} else {
			lcm.subscribeAll(new LCMInfo());
			//lcm.subscribe(TEST_CHANNEL_A, new LCMInfo());
			while (true) {
				try {
					Thread.sleep(1000);
				} catch (InterruptedException ignored) {
				}
			}
		}
	}
	
	@Override
	public void messageReceived(LCM lcm, String channel, DataInputStream ins) {
		if (channel.equals(TEST_CHANNEL_A)) {
			try {
				new differential_drive_command_t(ins);
				System.out.print("o");
			} catch (IOException ex) {
				System.out.println("Error decoding differential_drive_command_t message: " + ex);
			} catch (Throwable t) {
				System.out.println("Unhandled exception " + t);
			}
		}
	}
}
