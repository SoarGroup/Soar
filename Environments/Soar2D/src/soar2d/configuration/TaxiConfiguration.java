package soar2d.configuration;

import java.io.File;
import java.util.List;

import org.jdom.Element;

public class TaxiConfiguration extends BaseConfiguration implements IConfiguration {

	TaxiConfiguration() {
		super();
		
		this.mapPath += "taxi" + System.getProperty("file.separator");
		this.agentPath += "taxi" + System.getProperty("file.separator");
		this.map = new File(this.mapPath + "default.xml");
	}

	public String getMapExt() {
		return "xml";
	}

	public String getTitle() {
		return "Taxi";
	}
	
	public void rules(Element rules) throws LoadError {
		List<Element> children = (List<Element>)rules.getChildren();
		if (children.size() < 1) {
			return;
		}
		if (children.size() > 1) {
			throw new LoadError("Only one rules set is allowed");
		}
		Element child = children.get(0);
		if (child.getName().equalsIgnoreCase(getTitle()) == false) {
			throw new LoadError("Expected " + getTitle() + " rules, got " + child.getName());
		}
	}

	public void rulesSave(Element rules) {
		assert false;
	}
	
	public void copy(IConfiguration config) {
		TaxiConfiguration xConfig = (TaxiConfiguration)config;
		assert false;
	}
	
	public void setDefaultTerminals(Configuration configuration) {
		configuration.setTerminalPassengerDelivered(true);
		configuration.setTerminalFuelRemaining(true);
	}

	public boolean getRunTilOutput() {
		return false;
	}
	
	public BaseConfiguration getModule() {
		return this;
	}

	public int getCycleTimeSlice() {
		return 50;
	}

}
