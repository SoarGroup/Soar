package org.msoar.sps.sm;

import java.io.IOException;
import java.io.ObjectOutputStream;
import java.util.List;
import java.util.Map;

import org.apache.log4j.Logger;
import org.msoar.sps.SharedNames;

final class RemoteRunner implements Runner {
	private static final Logger logger = Logger.getLogger(RemoteRunner.class);
	
	static RemoteRunner newInstance(RemoteConnection rc, List<String> command, String config, Map<String, String> environment) throws IOException {
		return new RemoteRunner(rc, command, config, environment);
	}
	
	private final RemoteConnection rc;
	
	private RemoteRunner(RemoteConnection rc, List<String> command, String config, Map<String, String> environment) throws IOException {
		if (rc == null) {
			throw new NullPointerException();
		}
		this.rc = rc;

		logger.trace("Starting new remote runner");
		ObjectOutputStream oout = rc.getObjectOutputStream();
		oout.writeObject(SharedNames.NET_START);
		oout.writeObject(command.toArray(new String[command.size()]));
		if (config == null) {
			oout.writeObject(SharedNames.NET_CONFIG_NO);
		} else {
			oout.writeObject(SharedNames.NET_CONFIG_YES);
			oout.writeObject(config);
		}
		
		if (environment == null) {
			oout.writeObject(SharedNames.NET_ENVIRONMENT_NO);
		} else {
			oout.writeObject(SharedNames.NET_ENVIRONMENT_YES);
			oout.writeObject(environment.keySet().toArray(new String[environment.keySet().size()]));
			oout.writeObject(environment.values().toArray(new String[environment.values().size()]));
		}
		oout.flush();
	}

	public String getComponentName() {
		return rc.getComponentName();
	}
	
	public void stop() throws IOException {
		ObjectOutputStream oout = rc.getObjectOutputStream();
		oout.writeObject(SharedNames.NET_STOP);
		oout.flush();
	}
}
