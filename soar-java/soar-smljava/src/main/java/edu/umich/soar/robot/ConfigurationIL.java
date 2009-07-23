package edu.umich.soar.robot;

import sml.FloatElement;
import sml.Identifier;
import sml.StringElement;

public final class ConfigurationIL implements ConfigureInterface {

	private final Identifier configuration;
	private final StringElement yawFormatwme;
	private final FloatElement offsetxwme;
	private final FloatElement offsetywme;
	private final OffsetPose opose;
	boolean floatYawWmes = true;

	public ConfigurationIL(Identifier configuration, OffsetPose opose) {
		this.configuration = configuration;
		this.opose = opose;

		yawFormatwme = configuration.CreateStringWME("yaw-format", "float");
		offsetxwme = configuration.CreateFloatWME("offset-x", opose.getOffset()[0]);
		offsetywme = configuration.CreateFloatWME("offset-y", opose.getOffset()[1]);
	}

	public void destroy() {
		configuration.DestroyWME();
	}

	public void update() {
		yawFormatwme.Update(floatYawWmes ? "float" : "int");
		offsetxwme.Update(opose.getOffset()[0]);
		offsetywme.Update(opose.getOffset()[1]);
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

