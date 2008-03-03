package soar2d.configuration;

import java.io.File;
import java.util.Iterator;
import java.util.List;

import org.jdom.Element;

public class TankSoarConfiguration extends BaseConfiguration implements IConfiguration {

	TankSoarConfiguration() {
		super();
		
		this.mapPath += "tanksoar" + System.getProperty("file.separator");
		this.agentPath += "tanksoar" + System.getProperty("file.separator");
		this.map = new File(this.mapPath + "default.tmap");
	}
	
	public String getMapExt() {
		return "tmap";
	}

	public String getTitle() {
		return "TankSoar";
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
			
			if (child.getName().equalsIgnoreCase(kTagDefaultMissiles)) {
				try {
					this.setDefaultMissiles(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagDefaultMissiles);
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagDefaultEnergy)) {
				try {
					this.setDefaultEnergy(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagDefaultEnergy);
				}

			} else if (child.getName().equalsIgnoreCase(kTagDefaultHealth)) {
				try {
					this.setDefaultHealth(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagDefaultHealth);
				}

			} else if (child.getName().equalsIgnoreCase(kTagCollisionPenalty)) {
				try {
					this.setCollisionPenalty(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagCollisionPenalty);
				}

			} else if (child.getName().equalsIgnoreCase(kTagMaxMissilePacks)) {
				try {
					this.setMaxMissilePacks(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagMaxMissilePacks);
				}

			} else if (child.getName().equalsIgnoreCase(kTagMissilePackRespawnChance)) {
				try {
					this.setMissilePackRespawnChance(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagMissilePackRespawnChance);
				}

			} else if (child.getName().equalsIgnoreCase(kTagShieldEnergyUsage)) {
				try {
					this.setShieldEnergyUsage(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagShieldEnergyUsage);
				}

			} else if (child.getName().equalsIgnoreCase(kTagMissileHitAward)) {
				try {
					this.setMissileHitAward(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagMissileHitAward);
				}

			} else if (child.getName().equalsIgnoreCase(kTagMissileHitPenalty)) {
				try {
					this.setMissileHitPenalty(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagMissileHitPenalty);
				}

			} else if (child.getName().equalsIgnoreCase(kTagKillAward)) {
				try {
					this.setKillAward(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagKillAward);
				}

			} else if (child.getName().equalsIgnoreCase(kTagKillPenalty)) {
				try {
					this.setKillPenalty(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagKillPenalty);
				}

			} else if (child.getName().equalsIgnoreCase(kTagRadarWidth)) {
				try {
					this.setRadarWidth(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagRadarWidth);
				}

			} else if (child.getName().equalsIgnoreCase(kTagRadarHeight)) {
				try {
					this.setRadarHeight(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagRadarHeight);
				}

			} else if (child.getName().equalsIgnoreCase(kTagMaxSmellDistance)) {
				try {
					this.setMaxSmellDistance(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagMaxSmellDistance);
				}

			} else if (child.getName().equalsIgnoreCase(kTagMissileResetThreshold)) {
				try {
					this.setMissileResetThreshold(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagMissileResetThreshold);
				}

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
	}

	public void rulesSave(Element rules) {
		Element tanksoar = new Element(getTitle().toLowerCase());
		tanksoar.addContent(new Element(kTagDefaultMissiles).setText(Integer.toString(this.getDefaultMissiles())));
		tanksoar.addContent(new Element(kTagDefaultEnergy).setText(Integer.toString(this.getDefaultEnergy())));
		tanksoar.addContent(new Element(kTagDefaultHealth).setText(Integer.toString(this.getDefaultHealth())));
		tanksoar.addContent(new Element(kTagCollisionPenalty).setText(Integer.toString(this.getCollisionPenalty())));
		tanksoar.addContent(new Element(kTagMaxMissilePacks).setText(Integer.toString(this.getMaxMissilePacks())));
		tanksoar.addContent(new Element(kTagMissilePackRespawnChance).setText(Integer.toString(this.getMissilePackRespawnChance())));
		tanksoar.addContent(new Element(kTagShieldEnergyUsage).setText(Integer.toString(this.getShieldEnergyUsage())));
		tanksoar.addContent(new Element(kTagMissileHitAward).setText(Integer.toString(this.getMissileHitAward())));
		tanksoar.addContent(new Element(kTagMissileHitPenalty).setText(Integer.toString(this.getMissileHitPenalty())));
		tanksoar.addContent(new Element(kTagKillAward).setText(Integer.toString(this.getKillAward())));
		tanksoar.addContent(new Element(kTagKillPenalty).setText(Integer.toString(this.getKillPenalty())));
		tanksoar.addContent(new Element(kTagRadarWidth).setText(Integer.toString(this.getRadarWidth())));
		tanksoar.addContent(new Element(kTagRadarHeight).setText(Integer.toString(this.getRadarHeight())));
		tanksoar.addContent(new Element(kTagMaxSmellDistance).setText(Integer.toString(this.getMaxSmellDistance())));
		tanksoar.addContent(new Element(kTagMissileResetThreshold).setText(Integer.toString(this.getMissileResetThreshold())));
		rules.addContent(tanksoar);
	}
	
	private static final String kTagDefaultMissiles = "default-missiles";
	private int defaultMissiles = 15;
	public void setDefaultMissiles(int setting) {
		this.defaultMissiles = setting;
	}
	public int getDefaultMissiles() {
		return this.defaultMissiles;
	}
	
	private static final String kTagDefaultEnergy = "default-energy";
	private int defaultEnergy = 1000;
	public void setDefaultEnergy(int setting) {
		this.defaultEnergy = setting;
	}
	public int getDefaultEnergy() {
		return this.defaultEnergy;
	}
	
	private static final String kTagDefaultHealth = "default-health";
	private int defaultHealth = 1000;
	public void setDefaultHealth(int setting) {
		this.defaultHealth = setting;
	}
	public int getDefaultHealth() {
		return this.defaultHealth;
	}
	
	private static final String kTagCollisionPenalty = "collision-penalty";
	private int tankCollisionPenalty = -100;	// tanksoar
	public void setCollisionPenalty(int setting) {
		this.tankCollisionPenalty = setting;
	}
	public int getCollisionPenalty() {
		return this.tankCollisionPenalty;
	}
	
	private static final String kTagMaxMissilePacks = "max-missile-packs";
	private int maxMissilePacks = 3;
	public void setMaxMissilePacks(int setting) {
		this.maxMissilePacks = setting;
	}
	public int getMaxMissilePacks() {
		return this.maxMissilePacks;
	}
	
	private static final String kTagMissilePackRespawnChance = "missile-pack-respawn-chance";
	private int missilePackRespawnChance = 5;	// percent chance per update that a missile pack will respawn
	public void setMissilePackRespawnChance(int setting) {
		this.missilePackRespawnChance = setting;
	}
	public int getMissilePackRespawnChance() {
		return this.missilePackRespawnChance;
	}
	
	private static final String kTagShieldEnergyUsage = "shield-energy-usage";
	private int sheildEnergyUsage = -20;
	public void setShieldEnergyUsage(int setting) {
		this.sheildEnergyUsage = setting;
	}
	public int getShieldEnergyUsage() {
		return this.sheildEnergyUsage;
	}
	
	private static final String kTagMissileHitAward = "missile-hit-award";
	private int missileHitAward = 2;
	public void setMissileHitAward(int setting) {
		this.missileHitAward = setting;
	}
	public int getMissileHitAward() {
		return this.missileHitAward;
	}
	
	private static final String kTagMissileHitPenalty = "missile-hit-penalty";
	private int missileHitPenalty = -1;
	public void setMissileHitPenalty(int setting) {
		this.missileHitPenalty = setting;
	}
	public int getMissileHitPenalty() {
		return this.missileHitPenalty;
	}
	
	private static final String kTagKillAward = "kill-award";
	private int killAward = 3;
	public void setKillAward(int setting) {
		this.killAward = setting;
	}
	public int getKillAward() {
		return this.killAward;
	}
	
	private static final String kTagKillPenalty = "kill-penalty";
	private int killPenalty = -2;
	public void setKillPenalty(int setting) {
		this.killPenalty = setting;
	}
	public int getKillPenalty() {
		return this.killPenalty;
	}
	
	private static final String kTagRadarWidth = "radar-width";
	private int radarWidth = 3;
	public void setRadarWidth(int setting) {
		this.radarWidth = setting;
	}
	public int getRadarWidth() {
		return this.radarWidth;
	}
	
	private static final String kTagRadarHeight = "radar-height";
	private int radarHeight = 15;
	public void setRadarHeight(int setting) {
		this.radarHeight = setting;
	}
	public int getRadarHeight() {
		return this.radarHeight;
	}
	
	private static final String kTagMaxSmellDistance = "max-smell-distance";
	private int maxSmellDistance = 7;
	public void setMaxSmellDistance(int setting) {
		this.maxSmellDistance = setting;
	}
	public int getMaxSmellDistance() {
		return this.maxSmellDistance;
	}
	
	private static final String kTagMissileResetThreshold = "missile-reset-threshold";
	private int missileResetThreshold = 100;
	public void setMissileResetThreshold(int setting) {
		this.missileResetThreshold = setting;
	}
	public int getMissileResetThreshold() {
		return this.missileResetThreshold;
	}
	
	public void copy(IConfiguration config) {
		TankSoarConfiguration tConfig = (TankSoarConfiguration)config;
		this.defaultMissiles = tConfig.defaultMissiles;
		this.defaultEnergy = tConfig.defaultEnergy;
		this.defaultHealth = tConfig.defaultHealth;
		this.tankCollisionPenalty = tConfig.tankCollisionPenalty;
		this.maxMissilePacks = tConfig.maxMissilePacks;
		this.missilePackRespawnChance = tConfig.missilePackRespawnChance;
		this.sheildEnergyUsage = tConfig.sheildEnergyUsage;
		this.missileHitAward = tConfig.missileHitAward;
		this.missileHitPenalty = tConfig.missileHitPenalty;
		this.killAward = tConfig.killAward;
		this.killPenalty = tConfig.killPenalty;
		this.radarWidth = tConfig.radarWidth;
		this.radarHeight = tConfig.radarHeight;
		this.maxSmellDistance = tConfig.maxSmellDistance;
		this.missileResetThreshold = tConfig.missileResetThreshold;
	}

	public void setDefaultTerminals(Configuration configuration) {
		configuration.setTerminalWinningScore(50);
	}

	public boolean getRunTilOutput() {
		return true;
	}
	
	public BaseConfiguration getModule() {
		return this;
	}

	public int getCycleTimeSlice() {
		return 50;
	}

}
