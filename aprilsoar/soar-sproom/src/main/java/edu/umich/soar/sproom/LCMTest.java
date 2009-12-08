package edu.umich.soar.sproom;

import java.io.IOException;

import lcm.lcm.LCM;
import lcm.lcm.LCMDataInputStream;
import lcm.lcm.LCMSubscriber;
import lcmtypes.pose_t;

public class LCMTest {

	private final static String TEST_CHANNEL = "TEST_CHANNEL";
	private final static int SLEEP_MSEC = 1000;
	
	public static void main(String [] args) {
		final LCM lcm = LCM.getSingleton();

		final LCMSubscriber sub = new LCMSubscriber() {
			@Override
			public void messageReceived(LCM lcm, String channel, LCMDataInputStream ins) {
				assert channel.equals(TEST_CHANNEL);
				try {
					pose_t pose = new pose_t(ins);
					System.out.println(String.format("Received %d%n", pose.utime));
				} catch (IOException e) {
					System.out.println("IOException");
					e.printStackTrace();
				}
			}
		};
		
		lcm.subscribe(TEST_CHANNEL, sub);

		try {
			for(long count = 0; true; ++count) {
				pose_t pose = new pose_t();
				pose.utime = count;
				lcm.publish(TEST_CHANNEL, pose);
				Thread.sleep(SLEEP_MSEC);
			}
		} catch (InterruptedException e) {
			System.out.println("Interrupted");
			e.printStackTrace();
		}
		lcm.unsubscribe(TEST_CHANNEL, sub);
		System.out.println("Exiting");
	}
}
