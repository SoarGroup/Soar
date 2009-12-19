package edu.umich.soar.sproom.command;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.HzChecker;

public class Command {
	private static final Log logger = LogFactory.getLog(Command.class);
	
	public static double clamp(double value, double min, double max) {
		value = Math.max(value, min);
		value = Math.min(value, max);
		return value;
	}

	private final Pose pose = new Pose();
	private final Drive3 drive3 = new Drive3(new Drive2(new Drive1()));
	private final ScheduledExecutorService schexec = Executors.newSingleThreadScheduledExecutor();
	private final HzChecker hzChecker = HzChecker.newInstance(Command.class.toString());

	public Command() {
		schexec.scheduleAtFixedRate(new UpdateThread(), 0, 33, TimeUnit.MILLISECONDS); // 30 Hz
	}
	
	private class UpdateThread implements Runnable {
		@Override
		public void run() {
			if (logger.isDebugEnabled()) {
				hzChecker.tick();
			}
			
			DifferentialDriveCommand ddc = DifferentialDriveCommand.newEStopCommand();
			drive3.update(ddc, pose.getPose());
		}
	}
}
