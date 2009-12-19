package edu.umich.soar.sproom.command;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.HzChecker;
import edu.umich.soar.sproom.command.HttpControllerEvent.DDCChanged;
import edu.umich.soar.sproom.command.HttpControllerEvent.GainsChanged;
import edu.umich.soar.sproom.command.HttpControllerEvent.MessageChanged;
import edu.umich.soar.sproom.command.HttpControllerEvent.SoarChanged;

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
	private final SoarInterface soar = new SoarInterface();
	private final HttpController httpController = new HttpController();
	private DifferentialDriveCommand ddc = DifferentialDriveCommand.newEStopCommand();
	
	public Command() {
		httpController.addEventHandler(new HttpControllerEventHandler() {
			@Override
			public void handleEvent(HttpControllerEvent event) {
				if (event instanceof SoarChanged) {
					soarChanged();
				} else if (event instanceof DDCChanged) {
					synchronized(this) {
						DDCChanged ddcevent = (DDCChanged)event;
						logger.trace("http: " + ddcevent.getDdc());
						ddc = ddcevent.getDdc();
					}
				} else if (event instanceof MessageChanged) {
					//MessageChanged mc = (MessageChanged)event;
					//soar.newMessage("http", mc.getTokens());
				} else if (event instanceof GainsChanged) {
					drive3.updateGains();
				}
			}
		});

		schexec.scheduleAtFixedRate(new UpdateThread(), 0, 33, TimeUnit.MILLISECONDS); // 30 Hz
	}
	
	private void soarChanged() {
		soar.changeRunningState();
		ddc = DifferentialDriveCommand.newMotorCommand(0, 0);
	}
	
	private class UpdateThread implements Runnable {
		@Override
		public void run() {
			if (logger.isDebugEnabled()) {
				hzChecker.tick();
			}
			
			DifferentialDriveCommand soarddc = soar.getDDCommand();
			if (soarddc == null) {
				if (ddc == null) {
					ddc = DifferentialDriveCommand.newEStopCommand();
				}
			} else {
				ddc = soarddc;
			}
			drive3.update(ddc, pose.getPose());
		}
	}
}
