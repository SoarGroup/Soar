package splintersoar;

import java.io.DataInputStream;
import java.io.IOException;

import lcmtypes.differential_drive_command_t;
import lcm.lcm.LCM;
import lcm.lcm.LCMSubscriber;

/**
 * @author voigtjr
 * LCM constants.
 */
public class LCMInfo implements LCMSubscriber {
	public static final String L1_NETWORK = "udpm://239.255.1.3:7667?ttl=0";
	public static final String H1_NETWORK = "udpm://239.255.1.1:7667?ttl=0";
	public static final String H2_NETWORK = "udpm://239.255.1.2:7667?ttl=0";
	public static final String GG_NETWORK = "udpm://239.255.1.4:7667?ttl=1";
	
	public static final String DRIVE_COMMANDS_CHANNEL = "DRIVE_COMMANDS";
	public static final String LASER_LOC_CHANNEL = "LASER_LOC";
	public static final String COORDS_CHANNEL = "COORDS";
	public static final String SPLINTER_STATE_CHANNEL = "SPLINTER_STATE";
	public static final String LASER_FRONT_CHANNEL = "LASER_FRONT";
	public static final String WAYPOINTS_CHANNEL = "WAYPOINTS";
	public static final String PARTICLES_CHANNEL = "PARTICLES";
	public static final String RANGER_CHANNEL = "RANGER";

	public static final String TEST_CHANNEL_A = "TEST_CHANNEL";
	public static void main(String[] args) {
		LCM lcm = null;
		try {
			lcm = new LCM("udpm://239.255.76.67:7667?ttl=0");
		} catch (IOException e) {
			e.printStackTrace();
			System.exit(1);
		}

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
			while (true) {
				try {
					synchronized(lcm) {
						lcm.wait();
					}
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
