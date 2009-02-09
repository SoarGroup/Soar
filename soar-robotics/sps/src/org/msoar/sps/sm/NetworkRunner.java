package org.msoar.sps.sm;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.util.ArrayList;

class NetworkRunner {
	// TODO: how do you check a cast?
	@SuppressWarnings("unchecked")
	public static ArrayList<String> readCommand(ObjectInputStream oin) throws IOException {
		try {
			return (ArrayList<String>)oin.readObject();
		} catch (ClassNotFoundException e) {
			throw new IOException(e);
		}
	}

	public static String readString(ObjectInputStream oin) throws IOException {
		try {
			return (String)oin.readObject();
		} catch (ClassNotFoundException e) {
			throw new IOException(e);
		}
	}

	public static Boolean readBoolean(ObjectInputStream oin) throws IOException {
		try {
			return (Boolean)oin.readObject();
		} catch (ClassNotFoundException e) {
			throw new IOException(e);
		}
	}


}
