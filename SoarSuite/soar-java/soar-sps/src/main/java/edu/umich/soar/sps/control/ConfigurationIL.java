package edu.umich.soar.sps.control;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.StringElement;

final class ConfigurationIL {

	private final StringElement yawFormatwme;
	private final FloatElement offsetxwme;
	private final FloatElement offsetywme;
	private final Agent agent;
	private final SplinterState splinter;

	ConfigurationIL(Agent agent, Identifier configuration, SplinterState splinter) {
		this.agent = agent;
		this.splinter = splinter;

		yawFormatwme = agent.CreateStringWME(configuration, "yaw-format", "float");
		offsetxwme = agent.CreateFloatWME(configuration, "offset-x", 0);
		offsetywme = agent.CreateFloatWME(configuration, "offset-y", 0);
	}

	void update(boolean useFloatYawWmes) {
		agent.Update(yawFormatwme, useFloatYawWmes ? "float" : "int");
		agent.Update(offsetxwme, splinter.getOffset()[0]);
		agent.Update(offsetywme, splinter.getOffset()[1]);
	}
}

