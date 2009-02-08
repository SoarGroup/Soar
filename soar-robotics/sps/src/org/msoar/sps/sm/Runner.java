package org.msoar.sps.sm;

import java.io.IOException;
import java.util.ArrayList;

import org.msoar.sps.config.Config;

interface Runner {
	String getComponentName();
	void configure(ArrayList<String> command, Config config) throws IOException;
	void start() throws IOException;
	boolean isAlive()throws IOException;
	void destroy() throws IOException;
}
