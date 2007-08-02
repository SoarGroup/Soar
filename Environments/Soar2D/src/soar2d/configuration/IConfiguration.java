package soar2d.configuration;

import java.io.File;

import org.jdom.Element;

import soar2d.configuration.LoadError;

public interface IConfiguration {

	String getMapPath();

	String getAgentPath();

	String getMapExt();

	void setMap(File absoluteFile);

	File getMap();

	String getTitle();
	
	void rules(Element rules) throws LoadError;
	void rulesSave(Element rules);

	void copy(IConfiguration config);

	void setDefaultTerminals(Configuration configuration);

	boolean getRunTilOutput();

	BaseConfiguration getModule();

	int getCycleTimeSlice();
}
