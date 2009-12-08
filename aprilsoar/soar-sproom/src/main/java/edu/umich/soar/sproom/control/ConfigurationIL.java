package edu.umich.soar.sproom.control;

import sml.FloatElement;
import sml.Identifier;
import sml.StringElement;

public final class ConfigurationIL implements ConfigureInterface {

	private final Identifier configuration;
	private final StringElement yawFormatwme;
	private final FloatElement offsetxwme;
	private final FloatElement offsetywme;
	private final FloatElement offsetzwme;
	private final OffsetPose opose;
	boolean floatYawWmes = true;

	public ConfigurationIL(Identifier configuration, OffsetPose opose) {
		this.configuration = configuration;
		this.opose = opose;

		yawFormatwme = configuration.CreateStringWME("yaw-format", "float");
		offsetxwme = configuration.CreateFloatWME("offset-x", opose.getOffset().pos[0]);
		offsetywme = configuration.CreateFloatWME("offset-y", opose.getOffset().pos[1]);
		offsetzwme = configuration.CreateFloatWME("offset-z", opose.getOffset().pos[2]);
	}

	public void destroy() {
		configuration.DestroyWME();
	}

	public void update() {
		yawFormatwme.Update(floatYawWmes ? "float" : "int");
		offsetxwme.Update(opose.getOffset().pos[0]);
		offsetywme.Update(opose.getOffset().pos[1]);
		offsetzwme.Update(opose.getOffset().pos[2]);
	}

	@Override
	public boolean isFloatYawWmes() {
		return floatYawWmes;
	}

	@Override
	public void setFloatYawWmes(boolean setting) {
		floatYawWmes = setting;
	}
}

