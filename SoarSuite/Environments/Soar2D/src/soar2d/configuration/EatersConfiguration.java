package soar2d.configuration;

import java.io.File;
import java.util.Iterator;
import java.util.List;

import org.jdom.Element;

public class EatersConfiguration extends BaseConfiguration implements IConfiguration {

	EatersConfiguration() {
		super();
		
		this.mapPath += "eaters" + System.getProperty("file.separator");
		this.agentPath += "eaters" + System.getProperty("file.separator");
		this.map = new File(this.mapPath + "random-walls.emap");
	}

	public String getMapExt() {
		return "emap";
	}

	public String getTitle() {
		return "Eaters";
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
			
			if (child.getName().equalsIgnoreCase(kTagVision)) {
				try {
					this.setEaterVision(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagVision);
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagWallPenalty)) {
				try {
					this.setWallPenalty(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagWallPenalty);
				}

			} else if (child.getName().equalsIgnoreCase(kTagJumpPenalty)) {
				try {
					this.setJumpPenalty(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagJumpPenalty);
				}

			} else if (child.getName().equalsIgnoreCase(kTagLowProbability)) {
				try {
					this.setLowProbability(Double.parseDouble(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagLowProbability);
				}

			} else if (child.getName().equalsIgnoreCase(kTagHighProbability)) {
				try {
					this.setHighProbability(Double.parseDouble(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagHighProbability);
				}

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
	}

	public void rulesSave(Element rules) {
		Element eaters = new Element(getTitle().toLowerCase());
		eaters.addContent(new Element(kTagVision).setText(Integer.toString(this.getEaterVision())));
		eaters.addContent(new Element(kTagWallPenalty).setText(Integer.toString(this.getWallPenalty())));
		eaters.addContent(new Element(kTagJumpPenalty).setText(Integer.toString(this.getJumpPenalty())));
		eaters.addContent(new Element(kTagLowProbability).setText(Double.toString(this.getLowProbability())));
		eaters.addContent(new Element(kTagHighProbability).setText(Double.toString(this.getHighProbability())));
		rules.addContent(eaters);
	}

	private static final String kTagVision = "vision";
	private int eaterVision = 2;	// distance that eater can see
	public void setEaterVision(int distance) {
		this.eaterVision = distance;
	}
	public int getEaterVision() {
		return this.eaterVision;
	}
	
	private static final String kTagWallPenalty = "wall-penalty";
	private int wallPenalty = -5;	// eaters scoring penalty
	public void setWallPenalty(int points) {
		this.wallPenalty = points;
	}
	public int getWallPenalty() {
		return this.wallPenalty;
	}

	private static final String kTagJumpPenalty = "jump-penalty";
	private int jumpPenalty = -5;	// eaters scoring penalty
	public void setJumpPenalty(int points) {
		this.jumpPenalty = points;
	}
	public int getJumpPenalty() {
		return this.jumpPenalty;
	}
	
	private static final String kTagLowProbability = "low-probability";
	private double lowProbability = .15;		// eaters wall generation
	public void setLowProbability(double prob) {
		this.lowProbability = prob;
	}
	public double getLowProbability() {
		return this.lowProbability;
	}

	private static final String kTagHighProbability = "high-probability";
	private double higherProbability = .65;	// eaters wall generation
	public void setHighProbability(double prob) {
		this.higherProbability = prob;
	}
	public double getHighProbability() {
		return this.higherProbability;
	}

	public void copy(IConfiguration config) {
		EatersConfiguration eConfig = (EatersConfiguration)config;
		this.eaterVision = eConfig.eaterVision;
		this.wallPenalty = eConfig.wallPenalty;
		this.jumpPenalty = eConfig.jumpPenalty;
		this.lowProbability = eConfig.lowProbability;
		this.higherProbability = eConfig.higherProbability;
	}
	
	public void setDefaultTerminals(Configuration configuration) {
		configuration.setTerminalWinningScore(0);
		configuration.setTerminalFoodRemaining(true);
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

