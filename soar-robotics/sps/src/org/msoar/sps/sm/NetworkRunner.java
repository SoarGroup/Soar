package org.msoar.sps.sm;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

final class NetworkRunner {
	static Map<String, String> readEnvironment(ObjectInputStream oin) throws IOException {
		String[] keys = null;
		String[] values = null;
		try {
			keys = (String[])oin.readObject();
			values = (String[])oin.readObject();
		} catch (ClassNotFoundException e) {
			throw new IOException(e);
		}
		
		if (keys.length != values.length) {
			assert false;
			return null;
		}

		Map<String, String> environment = new HashMap<String, String>();
		for (int i = 0; i < keys.length; ++i) {
			environment.put(keys[i], values[i]);
		}
		return environment;
	}

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
