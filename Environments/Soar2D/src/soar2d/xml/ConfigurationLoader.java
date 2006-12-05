package soar2d.xml;

import java.util.Stack;
import java.util.logging.*;
import java.io.*;

import sml.ElementXML;
import soar2d.*;
import soar2d.player.*;

public class ConfigurationLoader {
	private Stack<String> xmlPath;

	static final int kVersion = 1;
	int version = kVersion;
	Configuration c;
	
	private void throwSyntax(String message) throws SyntaxException {
		throw new SyntaxException(xmlPath, message);
	}
	
	private void throwSML(String message) throws SMLException {
		throw new SMLException(xmlPath, message);
	}
	
	public Configuration getConfig() {
		return c;
	}
	
	public boolean load(String configFile) {
		
		c = new Configuration();
		
		// Generate paths
		c.basePath = System.getProperty("user.dir") + System.getProperty("file.separator");
		c.mapPath = c.basePath + c.kMapDir + System.getProperty("file.separator");
		c.agentPath = c.basePath + c.kAgentDir + System.getProperty("file.separator");
		
		xmlPath = new Stack<String>();
		
		ElementXML rootTag = ElementXML.ParseXMLFromFile(configFile);
		if (rootTag == null) {
			Soar2D.control.severeError("Error parsing file: " + ElementXML.GetLastParseErrorDescription());
			return false;
		}
		
		try {
			if (rootTag.IsTag(Names.kTagSoar2D)) {
				xmlPath.push(Names.kTagSoar2D);
				soar2d(rootTag);
				xmlPath.pop();
			} else {
				throwSyntax("unrecognized tag " + rootTag.GetTagName());
			}
		} catch (SyntaxException p) {
			this.c = null;
			Soar2D.control.severeError("Error parsing file: " + p.getMessage());
			return false;
			
		} catch (SMLException s) {
			this.c = null;
			Soar2D.control.severeError("SML error during parsing: " + s.getMessage());
			return false;
			
		} finally {
			// BADBAD: call garbage collector because something is lame and leaks
			// and this is the only thing that fixes it
			assert rootTag.GetRefCount() == 1;
			rootTag.ReleaseRefOnHandle();
			rootTag = null;
			System.gc();
		}
		
		return c.eaters || c.tanksoar;
	}
	
	private void soar2d(ElementXML soar2dTag) throws SyntaxException, SMLException {
		
		String attribute = null;
		
		attribute = soar2dTag.GetAttribute(Names.kParamVersion);
		if (attribute == null) {
			System.out.println("No configuration file version specified, assuming " + kVersion);
		} else {
			version = Integer.parseInt(attribute);
			System.out.println("Config file version: " + version);
		}
		
		if (version > kVersion) {
			throwSyntax("Unsupported configuration version " + version); 
		}
		
		ElementXML subTag = null;
		for (int soar2dTagIndex = 0 ; soar2dTagIndex < soar2dTag.GetNumberChildren() ; ++soar2dTagIndex) {

			subTag = new ElementXML();
			if (subTag == null) throwSML("couldn't create subTag");

			try {
				soar2dTag.GetChild(subTag, soar2dTagIndex);
				if (subTag == null) throwSML("failed to get tag by index");
				
				if (subTag.IsTag(Names.kTagLogger)) {
					xmlPath.push(Names.kTagLogger);
					logger(subTag);
					xmlPath.pop();
					
				} else if (subTag.IsTag(Names.kTagEaters)) {
					if (c.tanksoar) {
						throwSyntax("tanksoar and eaters tags cannot co-exist");
					}
					c.eaters = true;
					xmlPath.push(Names.kTagEaters);
					game(subTag);
					xmlPath.pop();
					
				} else if (subTag.IsTag(Names.kTagTankSoar) && !c.eaters) {
					if (c.eaters) {
						throwSyntax("tanksoar and eaters tags cannot co-exist");
					}
					c.tanksoar = true;
					xmlPath.push(Names.kTagTankSoar);
					game(subTag);
					xmlPath.pop();
					
				} else if (subTag.IsTag(Names.kTagDisplay)) {
					xmlPath.push(Names.kTagDisplay);
					display(subTag);
					xmlPath.pop();
					
				} else if (subTag.IsTag(Names.kTagSimulation)) {
					xmlPath.push(Names.kTagSimulation);
					simulation(subTag);
					xmlPath.pop();
					
				} else if (subTag.IsTag(Names.kTagClient)) {
					xmlPath.push(Names.kTagClient);
					client(subTag);
					xmlPath.pop();
					
				} else {
					throwSyntax("unrecognized tag " + subTag.GetTagName());
				}
			} finally {
				subTag.delete();
				subTag = null;
			}
		}
	}
	
	private void logger(ElementXML loggerTag) throws SyntaxException {
		
		String attribute;
		
		attribute = loggerTag.GetAttribute(Names.kParamLevel);
		if (attribute != null) {
			attribute = attribute.toLowerCase();
			if (attribute.equals(Names.kLevelSevere)) {
				c.logLevel = Level.SEVERE;
			} else if (attribute.equals(Names.kLevelWarning)) {
				c.logLevel = Level.WARNING;
			} else if (attribute.equals(Names.kLevelInfo)) {
				c.logLevel = Level.INFO;
			} else if (attribute.equals(Names.kLevelFine)) {
				c.logLevel = Level.FINE;
			} else if (attribute.equals(Names.kLevelFiner)) {
				c.logLevel = Level.FINER;
			} else if (attribute.equals(Names.kLevelFinest)) {
				c.logLevel = Level.FINEST;
			} else {
				throwSyntax("Unrecognized log level: " + attribute);
			}
		} else {
			throwSyntax("No log level attribute");
		}

		attribute = loggerTag.GetAttribute(Names.kParamConsole);
		if (attribute != null) {
			c.logConsole = Boolean.parseBoolean(attribute);
		}
		
		attribute = loggerTag.GetAttribute(Names.kParamFile);
		if (attribute != null) {
			c.logFile = Boolean.parseBoolean(attribute);
		}
		
		attribute = loggerTag.GetAttribute(Names.kParamFileName);
		if (c.logFileName == null) {
			c.logFileName = c.kDefaultLogFilename;
		} else {
			if (attribute.length() <= 0) {
				throwSyntax("zero length log filename specified");
			}
			c.logFileName = attribute;
		}
	}
	
	private void game(ElementXML gameTag) throws SyntaxException, SMLException {
		String attribute;
		
		attribute = gameTag.GetAttribute(Names.kParamMap);
		if ((attribute == null) || (attribute.length() <= 0)) {
			throwSyntax("map not specified, is required");
		}
		File mapFile = new File(attribute);
		if (!mapFile.exists()) {
			mapFile = new File(c.mapPath + attribute);
			if (!mapFile.exists()) {
				throwSyntax("Error finding map " + attribute);
			}
		}
		c.map = mapFile;

		ElementXML subTag = null;
		for (int soar2dTagIndex = 0 ; soar2dTagIndex < gameTag.GetNumberChildren() ; ++soar2dTagIndex) {

			subTag = new ElementXML();
			if (subTag == null) throwSML("couldn't create subTag");

			try {
				gameTag.GetChild(subTag, soar2dTagIndex);
				if (subTag == null) throwSML("failed to get tag by index");
				
				if (subTag.IsTag(Names.kTagAgent)) {
					xmlPath.push(Names.kTagAgent);
					agent(subTag);
					xmlPath.pop();

				} else {
					throwSyntax("unrecognized tag " + subTag.GetTagName());
				}
			} finally {
				subTag.delete();
				subTag = null;
			}
		}
	}

	private void agent(ElementXML tag) {
		PlayerConfig agentConfig = new PlayerConfig();
		agentConfig.setName(tag.GetAttribute(Names.kParamName));
		
		String productions = tag.GetAttribute(Names.kParamProductions);
		if (productions != null) {
			File productionsFile = new File(productions);
			if (!productionsFile.exists()) {
				productionsFile = new File(c.agentPath + productions);
				if (!productionsFile.exists()) {
					Soar2D.logger.warning("Error finding prodcutions " + productions);
				}
			}
			agentConfig.setProductions(productionsFile);
		}
		agentConfig.setColor(tag.GetAttribute(Names.kParamColor));
		
		String attribute;
		attribute = tag.GetAttribute(Names.kParamX);
		int x = -1;
		int y = -1;
		if (attribute != null) {
			x = Integer.parseInt(attribute);
		}
		
		attribute = tag.GetAttribute(Names.kParamY);
		if (attribute != null) {
			y = Integer.parseInt(attribute);
		}
		
		if ((x >= 0) && (y >= 0)) {
			agentConfig.setInitialLocation(new java.awt.Point(x, y));
		}
		
		if (c.tanksoar) {
			int facingInt = Integer.parseInt(tag.GetAttribute(Names.kParamFacing));
			if ((facingInt > 0) && (facingInt < 5)) {
				agentConfig.setFacing(facingInt);
			}

			attribute = tag.GetAttribute(Names.kParamEnergy);
			if (attribute != null) {
				agentConfig.setEnergy(Integer.parseInt(attribute));
			}

			attribute = tag.GetAttribute(Names.kParamHealth);
			if (attribute != null) {
				agentConfig.setHealth(Integer.parseInt(attribute));
			}

			attribute = tag.GetAttribute(Names.kParamMissiles);
			if (attribute != null) {
				agentConfig.setMissiles(Integer.parseInt(attribute));
			}
		}
		c.players.add(agentConfig);
	}
	
	private void display(ElementXML tag) {
		String attribute = tag.GetAttribute(Names.kParamGraphical);
		if (attribute != null) {
			c.graphical = Boolean.parseBoolean(attribute);
		}
	}
	
	private void simulation(ElementXML tag) {
		String attribute;
		
		attribute = tag.GetAttribute(Names.kParamDebuggers);
		if (attribute != null) {
			c.debuggers = Boolean.parseBoolean(attribute);
		}
		
		attribute = tag.GetAttribute(Names.kParamMaxUpdates);
		if (attribute != null) {
			c.maxUpdates = Integer.parseInt(attribute);
		}
		
		attribute = tag.GetAttribute(Names.kParamWinningScore);
		if (attribute != null) {
			c.winningScore = Integer.parseInt(attribute);
		}
		
		attribute = tag.GetAttribute(Names.kParamRemote);
		if (attribute != null) {
			c.remote = Boolean.parseBoolean(attribute);
		}
		
		attribute = tag.GetAttribute(Names.kParamRandom);
		if (attribute != null) {
			c.random = Boolean.parseBoolean(attribute);
		}
	}
	
	private void client(ElementXML tag) {
		ClientConfig client = new ClientConfig();
		
		client.name = tag.GetAttribute(Names.kParamName);
		
		client.command = tag.GetAttribute(Names.kParamCommand);
		
		String attribute;
		
		attribute = tag.GetAttribute(Names.kParamTimeOut);
		if (attribute != null) {
			client.timeout = Integer.parseInt(attribute);
		}
		
		attribute = tag.GetAttribute(Names.kParamAfter);
		if (attribute != null) {
			client.after = Boolean.parseBoolean(attribute);
		}
		
		c.clients.add(client);
	}
}
