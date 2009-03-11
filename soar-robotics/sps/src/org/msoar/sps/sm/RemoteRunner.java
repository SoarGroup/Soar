package org.msoar.sps.sm;

import java.util.List;
import java.util.Map;

import org.apache.log4j.Logger;

final class RemoteRunner implements Runner {
	private static final Logger logger = Logger.getLogger(RemoteRunner.class);
	
	static RemoteRunner newInstance(RemoteConnection rc, List<String> command, String config, Map<String, String> environment) {
		return new RemoteRunner(rc, command, config, environment);
	}
	
	private final RemoteConnection rc;
	
	private RemoteRunner(RemoteConnection rc, List<String> command, String config, Map<String, String> environment) {
		if (rc == null) {
			throw new NullPointerException();
		}
		this.rc = rc;

		logger.trace("Starting new remote runner");
		rc.command(command);
		if (config != null) {
			rc.config(config);
		}
		if (environment != null) {
			rc.environment(environment);
		}
		rc.start();
	}

	public String getComponentName() {
		return rc.getComponentName();
	}
	
	public void stop() {
		rc.stop();
	}
}
