package org.msoar.sps.sm;

import java.io.IOException;

interface Runner {
	String getComponentName();
	void stop() throws IOException;
}
