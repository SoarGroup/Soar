package org.msoar.sps.sm;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.util.Arrays;
import java.util.List;

final class NetworkRunner {
	static List<String> readCommand(ObjectInputStream oin) throws IOException {
		String[] command = null;
		try {
			command = (String[])oin.readObject();

		} catch (ClassNotFoundException e) {
			throw new IOException(e);
		}
		return Arrays.asList(command);
	}

	static String readString(ObjectInputStream oin) throws IOException {
		try {
			return (String)oin.readObject();
		} catch (ClassNotFoundException e) {
			throw new IOException(e);
		}
	}

	static Boolean readBoolean(ObjectInputStream oin) throws IOException {
		try {
			return (Boolean)oin.readObject();
		} catch (ClassNotFoundException e) {
			throw new IOException(e);
		}
	}

	private NetworkRunner() {
		assert false;
	}
}
