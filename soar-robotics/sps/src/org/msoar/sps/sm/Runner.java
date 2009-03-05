package org.msoar.sps.sm;

import java.io.BufferedReader;
import java.io.IOException;
import java.util.List;
import java.util.Map;

interface Runner {
	String getComponentName();
	void setOutput(BufferedReader output);
	void configure(List<String> command, String config, Map<String, String> environment) throws IOException;
	void start() throws IOException;
	boolean isAlive()throws IOException;
	void stop() throws IOException;
	void quit();
}
