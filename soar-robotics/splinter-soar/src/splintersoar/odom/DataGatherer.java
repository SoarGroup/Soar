package splintersoar.odom;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Arrays;
import java.util.Timer;
import java.util.TimerTask;

import jmat.LinAlg;

import orc.Motor;
import orc.Orc;
import orc.QuadratureEncoder;
import orc.util.GamePad;

/**
 * @author voigtjr
 * Simple tool used to collect odometry data for calibration.
 */
public class DataGatherer {

	public static void main(String[] args) {
		if (args.length != 1)
			System.err.println("Usage: DataGatherer output-filename");
		new DataGatherer(args[0]);
	}

	public DataGatherer(String outputFilename) {
		Orc orc = Orc.makeOrc();
		System.out.println("orc up");

		motor[0] = new Motor(orc, 1, true);
		motor[1] = new Motor(orc, 0, false);
		quadenc[0] = new QuadratureEncoder(orc, 1, true);
		quadenc[1] = new QuadratureEncoder(orc, 0, false);

		gamePad = new GamePad();

		File datafile = new File(outputFilename);
		try {
			datawriter = new FileWriter(datafile);
		} catch (IOException e) {
			e.printStackTrace();
			System.exit(1);
		}
		System.out.println("file opened");

		System.out.println("running");
		Timer timer = new Timer();
		timer.schedule(new UpdateTask(), 0, 1000 / UPDATE_HZ);

		try {
			while (true) {
				updateGamepad();
				Thread.sleep(50);
			}
		} catch (InterruptedException ignored) {
		}
	}

	boolean markButton = false;
	double[] previousCommand = new double[2];

	private void updateGamepad() {
		boolean currentMarkButton = gamePad.getButton(0);
		// change on leading edge
		if (!markButton && currentMarkButton) {
			try {
				synchronized (datawriter) {
					datawriter.append("\n");
					datawriter.flush();
				}
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				System.exit(1);
			}
			System.out.println("mark\n");
		}
		markButton = currentMarkButton;

		double[] newThrottle = new double[2];

		if (false) {
			newThrottle[0] = gamePad.getAxis(1) * -1;
			newThrottle[1] = gamePad.getAxis(3) * -1;
		} else {
			double fwd = -1 * gamePad.getAxis(3); // +1 = forward, -1 = back
			double lr = -1 * gamePad.getAxis(2); // +1 = left, -1 = right

			newThrottle[0] = fwd - lr;
			newThrottle[1] = fwd + lr;

			double max = Math.max(Math.abs(newThrottle[0]), Math.abs(newThrottle[1]));
			if (max > 1) {
				newThrottle[0] /= max;
				newThrottle[1] /= max;
			}
		}

		if (!Arrays.equals(command, newThrottle))
			commandMotors(newThrottle);
	}

	int UPDATE_HZ = 30;
	Motor[] motor = new Motor[2];
	QuadratureEncoder[] quadenc = new QuadratureEncoder[2];
	FileWriter datawriter;
	GamePad gamePad;

	public class UpdateTask extends TimerTask {
		@Override
		public void run() {
			try {
				synchronized (datawriter) {
					datawriter.append(Integer.toString(quadenc[0].getPosition()));
					datawriter.append(",");
					datawriter.append(Integer.toString(quadenc[1].getPosition()));
					datawriter.append("\n");
					datawriter.flush();
				}
			} catch (Throwable e) {
				e.printStackTrace();
				System.exit(1);
			}
		}

	}

	public static double maxThrottleAccelleration = 1.0;
	double[] command = new double[2];
	private long lastutime = 0;

	private void commandMotors(double[] throttle) {
		assert throttle[0] <= 1;
		assert throttle[0] >= -1;
		assert throttle[1] <= 1;
		assert throttle[1] >= -1;

		if (lastutime == 0) {
			lastutime = System.nanoTime();
			return;
		}
		long elapsed = System.nanoTime() - lastutime;
		double elapsedsec = elapsed / 1000000000.0;

		double[] delta = LinAlg.subtract(throttle, command);

		boolean[] capped = { false, false };
		if (delta[0] > 0) {
			double newDelta = Math.min(delta[0], elapsedsec * maxThrottleAccelleration);
			if (delta[0] != newDelta) {
				capped[0] = true;
				delta[0] = newDelta;
			}
			// System.out.format( "delta1: %10.3f %10.3f%n", delta[0], delta[1]
			// );
		} else if (delta[0] < 0) {
			double newDelta = Math.max(delta[0], -1 * elapsedsec * maxThrottleAccelleration);
			if (delta[0] != newDelta) {
				capped[0] = true;
				delta[0] = newDelta;
			}
			// System.out.format( "delta2: %10.3f %10.3f%n", delta[0], delta[1]
			// );
		}
		if (delta[1] > 0) {
			double newDelta = Math.min(delta[1], elapsedsec * maxThrottleAccelleration);
			if (delta[1] != newDelta) {
				capped[1] = true;
				delta[1] = newDelta;
			}
			// System.out.format( "delta3: %10.3f %10.3f%n", delta[0], delta[1]
			// );
		} else if (delta[1] < 0) {
			double newDelta = Math.max(delta[1], -1 * elapsedsec * maxThrottleAccelleration);
			if (delta[1] != newDelta) {
				capped[1] = true;
				delta[1] = newDelta;
			}
			// System.out.format( "delta4: %10.3f %10.3f%n", delta[0], delta[1]
			// );
		}

		command[0] += delta[0];
		command[1] += delta[1];

		// System.out.format( "0: %10.3f %10.3f %10.3f %10.3f %s%n",
		// throttle[0], delta[0], delta[0] / elapsedsec, command[0], capped[0] ?
		// "capped" : "" );
		// System.out.format( "1: %10.3f %10.3f %10.3f %10.3f %s%n",
		// throttle[1], delta[1], delta[1] / elapsedsec, command[1], capped[1] ?
		// "capped" : "" );

		lastutime += elapsed;

		motor[0].setPWM(command[0]);
		motor[1].setPWM(command[1]);
	}

}
