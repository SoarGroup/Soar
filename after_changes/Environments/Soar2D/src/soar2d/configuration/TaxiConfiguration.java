package soar2d.configuration;

import java.io.File;
import java.util.Iterator;
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
		children = (List<Element>)child.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			child = iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagDisableFuel)) {

			} else if (child.getName().equalsIgnoreCase(kTagFuelStartingMinimum)) {
				try {
					this.setFuelStartingMinimum(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagFuelStartingMinimum);
				}

			} else if (child.getName().equalsIgnoreCase(kTagFuelStartingMaximum)) {
				try {
					this.setFuelStartingMaximum(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagFuelStartingMaximum);
				}

			} else if (child.getName().equalsIgnoreCase(kTagFuelMaximum)) {
				try {
					this.setFuelMaximum(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagFuelMaximum);
				}

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
	}

	public void rulesSave(Element rules) {
		Element taxi = new Element(getTitle().toLowerCase());
		taxi.addContent(new Element(kTagDisableFuel));
		taxi.addContent(new Element(kTagFuelStartingMinimum).setText(Integer.toString(this.getFuelStartingMinimum())));
		taxi.addContent(new Element(kTagFuelStartingMaximum).setText(Integer.toString(this.getFuelStartingMaximum())));
		taxi.addContent(new Element(kTagFuelMaximum).setText(Integer.toString(this.getFuelMaximum())));
		rules.addContent(taxi);
	}
	
	private static final String kTagDisableFuel = "disable-fuel";
	private boolean disableFuel = false;
	public void setDisableFuel(boolean setting) {
		this.disableFuel = setting;
	}
	public boolean getDisableFuel() {
		return this.disableFuel;
	}
	
	private static final String kTagFuelStartingMinimum = "fuel-starting-minimum";
	private int fuelStartingMinimum = 5;
	public void setFuelStartingMinimum(int setting) {
		this.fuelStartingMinimum = setting;
	}
	public int getFuelStartingMinimum() {
		return this.fuelStartingMinimum;
	}
	
	private static final String kTagFuelStartingMaximum = "fuel-starting-maximum";
	private int fuelStartingMaximum = 12;
	public void setFuelStartingMaximum(int setting) {
		this.fuelStartingMaximum = setting;
	}
	public int getFuelStartingMaximum() {
		return this.fuelStartingMaximum;
	}
	
	private static final String kTagFuelMaximum = "fuel-maximum";
	private int fuelMaximum = 14;
	public void setFuelMaximum(int setting) {
		this.fuelMaximum = setting;
	}
	public int getFuelMaximum() {
		return this.fuelMaximum;
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
