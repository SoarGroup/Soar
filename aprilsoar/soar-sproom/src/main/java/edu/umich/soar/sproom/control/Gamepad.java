package edu.umich.soar.sproom.control;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

class Gamepad {
	private final String devicePath;
	private final int axes[] = new int[16];
	private final int buttons[] = new int[16];

	Gamepad() {
		String paths[] = { "/dev/js0", "/dev/input/js0" };

		String devicePath = null;
		for (int i = 0; i < paths.length; i++) {
			String path = paths[i];
			File f = new File(path);
			if (f.exists()) {
				devicePath = path;
				break;
			}
		}
		this.devicePath = devicePath;

		if (this.devicePath == null) {
			throw new IllegalStateException("Couldn't find a joystick.");
		}

		new ReaderThread().start();
	}

	Gamepad(String path) {
		this.devicePath = path;
		new ReaderThread().start();
	}

	// returns [-1, 1]
	double getAxis(int axis) {
		if (axis >= axes.length)
			return 0;

		return axes[axis] / 32767.0;
	}

	boolean getButton(int button) {
		if (button >= buttons.length)
			return false;

		return buttons[button] > 0;
	}

	/**
	 * Returns once any button has been pressed, returning the button id. This
	 * is useful with cordless game pads as a way of ensuring that there's
	 * actually a device connected.
	 **/
	int waitForAnyButtonPress() {
		boolean buttonState[] = new boolean[16];
		for (int i = 0; i < buttonState.length; i++)
			buttonState[i] = getButton(i);

		while (true) {

			for (int i = 0; i < buttonState.length; i++)
				if (getButton(i) != buttonState[i])
					return i;

			try {
				Thread.sleep(10);
			} catch (InterruptedException ex) {
			}
		}
	}

	private class ReaderThread extends Thread {
		private ReaderThread() {
			setDaemon(true);
		}

		public void run() {
			try {
				runEx();
			} catch (IOException ex) {
				System.out.println("GamePad ex: " + ex);
			}
		}

		private void runEx() throws IOException {
			FileInputStream fins = new FileInputStream(new File(devicePath));
			byte buf[] = new byte[8];

			while (true) {
				fins.read(buf);

				// ?? time the button is held down maybe
				//int mstime = (buf[0] & 0xff) | ((buf[1] & 0xff) << 8)
				//		| ((buf[2] & 0xff) << 16) | ((buf[3] & 0xff) << 24);
				int value = (buf[4] & 0xff) | ((buf[5] & 0xff) << 8);

				if ((value & 0x8000) > 0) // sign extend
					value |= 0xffff0000;

				int type = buf[6] & 0xff;
				int number = buf[7] & 0xff;

				if ((type & 0x3) == 1) {
					if (number < buttons.length)
						buttons[number] = value;
					else
						System.out.println("GamePad: " + number + " buttons!");
				}

				if ((type & 0x3) == 2) {
					if (number < axes.length)
						axes[number] = value;
					else
						System.out.println("GamePad: " + number + " axes!");
				}
			}
		}
	}
}
