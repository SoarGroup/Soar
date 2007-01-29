package soar2d.xml;

import java.util.Iterator;
import java.util.Stack;
import java.util.logging.*;
import java.io.*;

import sml.*;
import soar2d.*;
import soar2d.Configuration.SimType;
import soar2d.player.*;

public class ConfigurationLoader {
	private Stack<String> xmlPath;

	static final int kVersion = 1;
	int version = kVersion;
	Configuration c;
	boolean hasType = false;
	
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
			assert rootTag.GetRefCount() == 1;
			rootTag.ReleaseRefOnHandle();
			rootTag.delete();
			rootTag = null;
		}
		
		return hasType;
	}
	
	public void generateXMLString_NO_CRASH() {
		ElementXML soar2dTag = new ElementXML();
		ElementXML loggerTag = new ElementXML();
		soar2dTag.AddChild(loggerTag);
		soar2dTag = null;
		
		System.gc();
		try { Thread.sleep(5); } catch (Exception ignored) {}
	}
	
	public void generateXMLString_CRASH_MINIMAL() {
		ElementXML soar2dTag = new ElementXML();
		crashCall(soar2dTag);
		soar2dTag = null;
		
		// BUGBUG: calling the garbage collector forces the crash to happen right away
		System.gc();
		try { Thread.sleep(5); } catch (Exception ignored) {}
	}
	
	public void crashCall(ElementXML soar2dTag) {
		ElementXML loggerTag = new ElementXML();
		soar2dTag.AddChild(loggerTag);
		loggerTag.AddRefOnHandle();
	}

	public String generateXMLString(Configuration config) {
		c = config;
		
		ElementXML soar2dTag = new ElementXML();
		soar2dTag.SetTagName(Names.kTagSoar2D);
		soar2dSave(soar2dTag);

		return soar2dTag.GenerateXMLString(true, true);
	}
	
	private void soar2dSave(ElementXML soar2dTag) {
		
		soar2dTag.AddAttributeConstConst(Names.kParamVersion, Integer.toString(version));
		
		ElementXML loggerTag = new ElementXML();
		loggerTag.SetTagName(Names.kTagLogger);
		loggerSave(loggerTag);
		soar2dTag.AddChild(loggerTag);
		
		ElementXML gameTag = new ElementXML();
		switch (c.getType()) {
		case kTankSoar:
			gameTag.SetTagName(Names.kTagTankSoar);
			break;

		case kEaters:
			gameTag.SetTagName(Names.kTagEaters);
			break;
		}
		gameSave(gameTag);
		soar2dTag.AddChild(gameTag);
		
		ElementXML displayTag = new ElementXML();
		displayTag.SetTagName(Names.kTagDisplay);
		displaySave(displayTag);
		soar2dTag.AddChild(displayTag);
		
		ElementXML simTag = new ElementXML();
		simTag.SetTagName(Names.kTagSimulation);
		simulationSave(simTag);
		soar2dTag.AddChild(simTag);
		
		Iterator<ClientConfig> iter = c.clients.iterator();
		while (iter.hasNext()) {
			ElementXML clientTag = new ElementXML();
			clientTag.SetTagName(Names.kTagClient);
			clientSave(clientTag, iter.next());
			soar2dTag.AddChild(clientTag);
		}
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
					if (hasType) {
						throwSyntax("tanksoar and eaters tags cannot co-exist");
					}
					hasType = true;
					c.setType(SimType.kEaters);
					xmlPath.push(Names.kTagEaters);
					game(subTag);
					xmlPath.pop();
					
				} else if (subTag.IsTag(Names.kTagTankSoar)) {
					if (hasType) {
						throwSyntax("tanksoar and eaters tags cannot co-exist");
					}
					hasType = true;
					c.setType(SimType.kTankSoar);
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
	
	private void loggerSave(ElementXML loggerTag) {
		if (c.logLevel.equals(Level.SEVERE)) {
			loggerTag.AddAttributeConstConst(Names.kParamLevel, Names.kLevelSevere);
			
		} else if (c.logLevel.equals(Level.WARNING)) {
			loggerTag.AddAttributeConstConst(Names.kParamLevel, Names.kLevelWarning);
			
		} else if (c.logLevel.equals(Level.INFO)) {
			loggerTag.AddAttributeConstConst(Names.kParamLevel, Names.kLevelInfo);
			
		} else if (c.logLevel.equals(Level.FINE)) {
			loggerTag.AddAttributeConstConst(Names.kParamLevel, Names.kLevelFine);
			
		} else if (c.logLevel.equals(Level.FINER)) {
			loggerTag.AddAttributeConstConst(Names.kParamLevel, Names.kLevelFiner);
			
		} else if (c.logLevel.equals(Level.FINEST)) {
			loggerTag.AddAttributeConstConst(Names.kParamLevel, Names.kLevelFinest);
			
		} else {
			assert false;
		}
		
		loggerTag.AddAttributeConstConst(Names.kParamConsole, c.logConsole ? Names.kTrue : Names.kFalse);
		loggerTag.AddAttributeConstConst(Names.kParamFile, c.logToFile ? Names.kTrue : Names.kFalse);
		loggerTag.AddAttributeConstConst(Names.kParamFileName, c.logFile.getAbsolutePath());
		loggerTag.AddAttributeConstConst(Names.kParamLogTime, c.logTime ? Names.kTrue : Names.kFalse);
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
			c.logToFile = Boolean.parseBoolean(attribute);
		}
		
		attribute = loggerTag.GetAttribute(Names.kParamFileName);
		if (attribute != null) {
			if (attribute.length() <= 0) {
				throwSyntax("zero length log filename specified");
			}
			c.logFile = new File(attribute);
		}
		
		attribute = loggerTag.GetAttribute(Names.kParamLogTime);
		if (attribute != null) {
			c.logTime = Boolean.parseBoolean(attribute);
		}
	}
	
	private void gameSave(ElementXML gameTag) {
		gameTag.AddAttributeConstConst(Names.kParamMap, c.map.getAbsolutePath());
		
		Iterator<PlayerConfig> iter = c.players.iterator();
		while (iter.hasNext()) {
			PlayerConfig player = iter.next();
			ElementXML agentTag = new ElementXML();
			agentTag.SetTagName(Names.kTagAgent);
			agentSave(agentTag, player);
			gameTag.AddChild(agentTag);
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
			mapFile = new File(c.getMapPath() + attribute);
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

	private void agentSave(ElementXML agentTag, PlayerConfig player) {
		if (player.hasName()) {
			agentTag.AddAttributeConstConst(Names.kParamName, player.getName());
		}
		
		if (player.hasProductions()) {
			agentTag.AddAttributeConstConst(Names.kParamProductions, player.getProductions().getAbsolutePath());
		}
		
		if (player.hasColor()) {
			agentTag.AddAttributeConstConst(Names.kParamColor, player.getColor());
		}
		
		if (player.hasInitialLocation()) {
			agentTag.AddAttributeConstConst(Names.kParamX, Integer.toString(player.getInitialLocation().x));
			agentTag.AddAttributeConstConst(Names.kParamY, Integer.toString(player.getInitialLocation().y));
		}
		
		if (player.hasPoints()) {
			agentTag.AddAttributeConstConst(Names.kParamPoints, Integer.toString(player.getPoints()));
		}
		
		if (player.hasFacing()) {
			String facing = Direction.stringOf[player.getFacing()];
			agentTag.AddAttributeConstConst(Names.kParamFacing, facing);
		}
		
		if (player.hasEnergy()) {
			agentTag.AddAttributeConstConst(Names.kParamEnergy, Integer.toString(player.getEnergy()));
		}
		
		if (player.hasHealth()) {
			agentTag.AddAttributeConstConst(Names.kParamHealth, Integer.toString(player.getHealth()));
		}
		
		if (player.hasMissiles()) {
			agentTag.AddAttributeConstConst(Names.kParamMissiles, Integer.toString(player.getMissiles()));
		}
		
		Iterator<String> iter = player.getShutdownCommands().iterator();
		while (iter.hasNext()) {
			String command = iter.next();
			ElementXML shutdownTag = new ElementXML();
			shutdownTag.SetTagName(Names.kTagShutdownCommand);
			shutdownCommandSave(shutdownTag, command);
			agentTag.AddChild(shutdownTag);
		}
	}
	
	private void agent(ElementXML tag) throws SMLException, SyntaxException {
		PlayerConfig agentConfig = new PlayerConfig();
		agentConfig.setName(tag.GetAttribute(Names.kParamName));
		
		String attribute = tag.GetAttribute(Names.kParamProductions);
		if (attribute != null) {
			File productionsFile = new File(attribute);
			if (!productionsFile.exists()) {
				productionsFile = new File(c.getAgentPath() + attribute);
				if (!productionsFile.exists()) {
					Soar2D.logger.warning("Error finding prodcutions " + attribute);
				}
			}
			agentConfig.setProductions(productionsFile);
		}
		agentConfig.setColor(tag.GetAttribute(Names.kParamColor));
		
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
		
		attribute = tag.GetAttribute(Names.kParamPoints);
		if (attribute != null) {
			agentConfig.setPoints(Integer.parseInt(attribute));
			agentConfig.setPoints(true);
		}
		
		attribute = tag.GetAttribute(Names.kParamFacing);
		if ((attribute != null) && (attribute.length() > 0)) {
			agentConfig.setFacing(Direction.getInt(attribute));
		}
		
		if (c.getType() == SimType.kTankSoar) {
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

		ElementXML subTag = null;
		for (int soar2dTagIndex = 0 ; soar2dTagIndex < tag.GetNumberChildren() ; ++soar2dTagIndex) {

			subTag = new ElementXML();
			if (subTag == null) throwSML("couldn't create subTag");

			try {
				tag.GetChild(subTag, soar2dTagIndex);
				if (subTag == null) throwSML("failed to get tag by index");
				
				if (subTag.IsTag(Names.kTagShutdownCommand)) {
					xmlPath.push(Names.kTagShutdownCommand);
					shutdownCommand(agentConfig, subTag);
					xmlPath.pop();

				} else {
					throwSyntax("unrecognized tag " + subTag.GetTagName());
				}
			} finally {
				subTag.delete();
				subTag = null;
			}
		}
		
		c.players.add(agentConfig);
	}
	
	private void shutdownCommandSave(ElementXML shutdownTag, String command) {
		shutdownTag.AddAttributeConstConst(Names.kParamCommand, command);
	}
	
	private void shutdownCommand(PlayerConfig playerConfig, ElementXML tag) throws SyntaxException {
		String attribute = tag.GetAttribute(Names.kParamCommand);
		if (attribute == null || attribute.length() <= 0) {
			throwSyntax("command parameter must be present in shutdown-command");
		}
		
		playerConfig.addShutdownCommand(attribute);
	}
	
	private void displaySave(ElementXML displayTag) {
		displayTag.AddAttributeConstConst(Names.kParamGraphical, c.graphical ? Names.kTrue : Names.kFalse);
	}
	
	private void display(ElementXML tag) {
		String attribute = tag.GetAttribute(Names.kParamGraphical);
		if (attribute != null) {
			c.graphical = Boolean.parseBoolean(attribute);
		}
	}

	private void simulationSave(ElementXML simulationTag) {
		simulationTag.AddAttributeConstConst(Names.kParamDebuggers, c.debuggers ? Names.kTrue : Names.kFalse);
		simulationTag.AddAttributeConstConst(Names.kParamRemote, c.remote ? Names.kTrue : Names.kFalse);
		simulationTag.AddAttributeConstConst(Names.kParamRandom, c.random ? Names.kTrue : Names.kFalse);
		simulationTag.AddAttributeConstConst(Names.kParamNoWorld, c.noWorld ? Names.kTrue : Names.kFalse);
		simulationTag.AddAttributeConstConst(Names.kParamMissileResetThreshold, Integer.toString(c.missileResetThreshold));
		simulationTag.AddAttributeConstConst(Names.kParamRandomSeed, Integer.toString(c.randomSeed));

		ElementXML terminalTag;
		if (c.terminalAgentCommand) {
			terminalTag = new ElementXML();
			terminalTag.SetTagName(Names.kTagTerminal);
			terminalTag.AddAttributeConstConst(Names.kParamType, Names.kTerminalAgentCommand);
			simulationTag.AddChild(terminalTag);
		}
		
		if (c.terminalFoodRemaining) {
			terminalTag = new ElementXML();
			terminalTag.SetTagName(Names.kTagTerminal);
			terminalTag.AddAttributeConstConst(Names.kParamType, Names.kTerminalFoodRemaining);
			simulationTag.AddChild(terminalTag);
		}
		
		if (c.terminalMaxUpdates > 0) {
			terminalTag = new ElementXML();
			terminalTag.SetTagName(Names.kTagTerminal);
			terminalTag.AddAttributeConstConst(Names.kParamType, Names.kTerminalMaxUpdates);
			terminalTag.AddAttributeConstConst(Names.kParamValue, Integer.toString(c.terminalMaxUpdates));
			simulationTag.AddChild(terminalTag);
		}
		
		if (c.terminalPointsRemaining) {
			terminalTag = new ElementXML();
			terminalTag.SetTagName(Names.kTagTerminal);
			terminalTag.AddAttributeConstConst(Names.kParamType, Names.kTerminalPointsRemaining);
			simulationTag.AddChild(terminalTag);
		}
		
		if (c.terminalUnopenedBoxes) {
			terminalTag = new ElementXML();
			terminalTag.SetTagName(Names.kTagTerminal);
			terminalTag.AddAttributeConstConst(Names.kParamType, Names.kTerminalUnopenedBoxes);
			simulationTag.AddChild(terminalTag);
		}
		
		if (c.terminalWinningScore > 0) {
			terminalTag = new ElementXML();
			terminalTag.SetTagName(Names.kTagTerminal);
			terminalTag.AddAttributeConstConst(Names.kParamType, Names.kTerminalWinningScore);
			terminalTag.AddAttributeConstConst(Names.kParamValue, Integer.toString(c.terminalWinningScore));
			simulationTag.AddChild(terminalTag);
		}
	}
	
	private void simulation(ElementXML simulationTag) throws SMLException, SyntaxException {
		String attribute;
		
		attribute = simulationTag.GetAttribute(Names.kParamDebuggers);
		if (attribute != null) {
			c.debuggers = Boolean.parseBoolean(attribute);
		}
		
		attribute = simulationTag.GetAttribute(Names.kParamRemote);
		if (attribute != null) {
			c.remote = Boolean.parseBoolean(attribute);
		}
		
		attribute = simulationTag.GetAttribute(Names.kParamRandom);
		if (attribute != null) {
			c.random = Boolean.parseBoolean(attribute);
		}
		
		attribute = simulationTag.GetAttribute(Names.kParamNoWorld);
		if (attribute != null) {
			c.noWorld = Boolean.parseBoolean(attribute);
		}
		
		attribute = simulationTag.GetAttribute(Names.kParamMissileResetThreshold);
		if (attribute != null) {
			c.missileResetThreshold = Integer.parseInt(attribute);
		}
		
		attribute = simulationTag.GetAttribute(Names.kParamRandomSeed);
		if (attribute != null) {
			c.randomSeed = Integer.parseInt(attribute);
		}
		
		ElementXML subTag = null;
		for (int subTagIndex = 0 ; subTagIndex < simulationTag.GetNumberChildren() ; ++subTagIndex) {

			subTag = new ElementXML();
			if (subTag == null) throwSML("couldn't create subTag");

			try {
				simulationTag.GetChild(subTag, subTagIndex);
				if (subTag == null) throwSML("failed to get tag by index");
				
				if (subTag.IsTag(Names.kTagTerminal)) {
					xmlPath.push(Names.kTagTerminal);
					terminal(subTag);
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

	private void terminal(ElementXML terminalTag) throws SyntaxException {
		String attribute;
		
		attribute = terminalTag.GetAttribute(Names.kParamType);
		if ((attribute == null) || attribute.length() <= 0) {
			throwSyntax("Terminal type is required.");
		}
		
		if (attribute.equalsIgnoreCase(Names.kTerminalMaxUpdates)) {
			attribute = terminalTag.GetAttribute(Names.kParamValue);
			if ((attribute == null) || attribute.length() <= 0) {
				throwSyntax("Value is required for terminal max-updates.");
			}
			c.terminalMaxUpdates = Integer.parseInt(attribute);
			
		} else if(attribute.equalsIgnoreCase(Names.kTerminalAgentCommand)) {
			c.terminalAgentCommand = true;
			
		} else if(attribute.equalsIgnoreCase(Names.kTerminalPointsRemaining)) {
			c.terminalPointsRemaining = true;
			
		} else if(attribute.equalsIgnoreCase(Names.kTerminalWinningScore)) {
			attribute = terminalTag.GetAttribute(Names.kParamValue);
			if ((attribute == null) || attribute.length() <= 0) {
				throwSyntax("Value is required for terminal winning-score.");
			}
			c.terminalWinningScore = Integer.parseInt(attribute);
		} else if(attribute.equalsIgnoreCase(Names.kTerminalFoodRemaining)) {
			
			c.terminalFoodRemaining = true;
			
		} else if(attribute.equalsIgnoreCase(Names.kTerminalUnopenedBoxes)) {
			c.terminalUnopenedBoxes = true;
			
		} else {
			throwSyntax("Unknown terminal type: " + attribute);
		}
	}
	
	private void clientSave(ElementXML clientTag, ClientConfig client) {

		clientTag.AddAttributeConstConst(Names.kParamName, client.name);
		
		if (client.command != null) {
			clientTag.AddAttributeConstConst(Names.kParamCommand, client.command);
		}
		
		clientTag.AddAttributeConstConst(Names.kParamTimeOut, Integer.toString(client.timeout));

		clientTag.AddAttributeConstConst(Names.kParamAfter, client.after ? Names.kTrue : Names.kFalse);
	}
	
	private void client(ElementXML tag) throws SyntaxException {
		ClientConfig client = new ClientConfig();
		
		client.name = tag.GetAttribute(Names.kParamName);
		if (client.name == null || client.name.length() < 1) {
			throwSyntax("Client name is required.");
		}
		
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
