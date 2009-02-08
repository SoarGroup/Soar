package org.msoar.sps.sm;

import java.io.IOException;
import java.util.ArrayList;

interface Runner {
	String getComponentName();
	void configure(ArrayList<String> command, String config) throws IOException;
	void start() throws IOException;
	boolean isAlive()throws IOException;
	void stop() throws IOException;
	void quit();
}
