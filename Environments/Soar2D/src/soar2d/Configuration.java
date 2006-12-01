package soar2d;

import sml.*;
import java.util.*;
import java.util.logging.*;

class AnalyzeError extends Exception {
	public static final long serialVersionUID = 1;
	public AnalyzeError(String message) {
		super(message);
	}
}

public class Configuration {
	
	public final String kDefaultXMLSettingsFile = "soar2d.xml";
	public final String kDefaultLogFilename = "soar2d.log";
	public final String kMapDir = "maps";
	public final String kAgentDir = "agents";

	public final int kMaxEntities = 7;	
	public final int kDefaultVersion = 1;
	public final Level kDefaultLogLevel = Level.INFO;
	public final int kEaterVision = 2;
	public final int kWallPenalty = -5;	// eaters scoring penalty
	public final int kJumpPenalty = -5;	// eaters scoring penalty
	public final double kLowProbability = .15;		// eaters wall generation
	public final double kHigherProbability = .65;	// eaters wall generation

	public final boolean kDefaultPropertyBoolean = false;
	public final float kDefaultPropertyFloat = 0;
	public final int kDefaultPropertyInt = 0;
	
	public Level logLevel = kDefaultLogLevel;
	public boolean logFile = false;
	public String logFileName = null;
	public boolean logConsole = false;
	public boolean eaters = false;
	public boolean tanksoar = false;
	public boolean graphical = true;
	public boolean debuggers = false;
	public boolean remote = false;
	public boolean random = true;
	public int maxUpdates = 0;
	public int winningScore = 0;
	public String map = null;
	public ArrayList clients = new ArrayList();
	public ArrayList agents = new ArrayList();
	public String basePath = null;
	public String mapPath = null;
	public String agentPath = null;
	
	class Client {
		public String name = null;
		public String command = null;
		public int timeout = 0;
		public boolean after = false;
	}
	
	abstract class Agent {
		public String name;
		public String productions;
		public String color;
		public int y;
		public int x;
	}
	
	class EatersAgent extends Agent {
		
	}
	
	class TankSoarAgent extends Agent {
		public String facing;
		public int energy;
		public int health;
		public int missiles;
	}
	
	public Configuration() {
		// Generate paths
		basePath = System.getProperty("user.dir") + System.getProperty("file.separator");
		if (Soar2D.logger.isLoggable(Level.FINER)) Soar2D.logger.finer("Base path: " + basePath);
		mapPath = basePath + kMapDir + System.getProperty("file.separator");
		agentPath = basePath + kAgentDir + System.getProperty("file.separator");
	}

	public boolean analyze(String configFile) {
		ElementXML rootTag = null;
		
		try {
			rootTag = ElementXML.ParseXMLFromFile(configFile);
			if (rootTag == null) {
				throw new AnalyzeError("Error parsing configuration file: " + ElementXML.GetLastParseErrorDescription());
			}

			if (rootTag.IsTag(Names.kTagSoar2D)) {
				String versionString = rootTag.GetAttribute(Names.kParamVersion);
				int version = kDefaultVersion;
				
				if (versionString != null) {
					Integer.parseInt(versionString);
				} else {
					System.out.println("No configuration file version specified, assuming 1");
				}
				
				if (version > 1) {
					throw new AnalyzeError("Incompatable configuration file version " + versionString);
				}
				
				ElementXML tag = null;
				try {
					for (int rootTagIndex = 0 ; rootTagIndex < rootTag.GetNumberChildren() ; ++rootTagIndex) {
						tag = new ElementXML();
						rootTag.GetChild(tag, rootTagIndex);

						if (tag.IsTag(Names.kTagLogger)) {
							logger(tag);
						} else if (tag.IsTag(Names.kTagEaters) && !tanksoar) {
							eaters(tag);
							eaters = true;
						} else if (tag.IsTag(Names.kTagTankSoar) && !eaters) {
							tanksoar(tag);
							tanksoar = true;
						} else if (tag.IsTag(Names.kTagDisplay)) {
							display(tag);
						} else if (tag.IsTag(Names.kTagSimulation)) {
							simulation(tag);
						} else if (tag.IsTag(Names.kTagClient)) {
							client(tag);
						} else {
							throw new AnalyzeError("Unrecognized tag " + tag.GetTagName());
						}
					}
				} finally {
					if (tag != null) {
						tag.delete();
						tag = null;
					}
				}
			}
				
		} catch (AnalyzeError e) {
			System.err.println(e.getMessage());
		} finally {
			if (rootTag != null) {
				rootTag.ReleaseRefOnHandle();
				rootTag = null;
			}
		}

		return eaters || tanksoar;
	}
	
	private void logger(ElementXML tag) throws AnalyzeError {
		String levelString = tag.GetAttribute(Names.kParamLevel);
		if (levelString != null) {
			levelString = levelString.toLowerCase();
			if (levelString.equals(Names.kLevelSevere)) {
				logLevel = Level.SEVERE;
			} else if (levelString.equals(Names.kLevelWarning)) {
				logLevel = Level.WARNING;
			} else if (levelString.equals(Names.kLevelInfo)) {
				logLevel = Level.INFO;
			} else if (levelString.equals(Names.kLevelFine)) {
				logLevel = Level.FINE;
			} else if (levelString.equals(Names.kLevelFiner)) {
				logLevel = Level.FINER;
			} else if (levelString.equals(Names.kLevelFinest)) {
				logLevel = Level.FINEST;
			} else {
				throw new AnalyzeError("Unrecognized log level");
			}
		} else {
			throw new AnalyzeError("No log level attribute");
		}

		String logFileString = tag.GetAttribute(Names.kParamFile);
		if (logFileString != null) {
			logFile = Boolean.parseBoolean(logFileString);
		}
		
		logFileName = tag.GetAttribute(Names.kParamFileName);
		if (logFileName == null) {
			logFileName = kDefaultLogFilename;
		}
	}
	
	private void eaters(ElementXML rootTag) throws AnalyzeError {
		map = rootTag.GetAttribute(Names.kParamMap);

		ElementXML tag = null;
		try {
			for (int rootTagIndex = 0 ; rootTagIndex < rootTag.GetNumberChildren() ; ++rootTagIndex) {
				tag = new ElementXML();
				rootTag.GetChild(tag, rootTagIndex);
	
				if (tag.IsTag(Names.kTagAgent)) {
					eatersAgent(tag);
				} else {
					throw new AnalyzeError("Unrecognized tag " + tag.GetTagName());
				}
			}
		} finally {
			if (tag != null) {
				tag.delete();
				tag = null;
			}
		}
	}

	private void eatersAgent(ElementXML tag) {
		EatersAgent agent = new EatersAgent();
		agent(tag, agent);
		agents.add(agent);
	}
	
	private void agent(ElementXML tag, Agent agent) {
		agent.name = tag.GetAttribute(Names.kParamName);
		
		agent.productions = tag.GetAttribute(Names.kParamProductions);
		
		agent.color = tag.GetAttribute(Names.kParamColor);
		
		String xString = tag.GetAttribute(Names.kParamX);
		if (xString != null) {
			agent.x = Integer.parseInt(xString);
		}
		
		String yString = tag.GetAttribute(Names.kParamY);
		if (yString != null) {
			agent.y = Integer.parseInt(yString);
		}
	}
	
	private void tanksoar(ElementXML rootTag) throws AnalyzeError {
		map = rootTag.GetAttribute(Names.kParamMap);

		ElementXML tag = null;
		try {
			for (int rootTagIndex = 0 ; rootTagIndex < rootTag.GetNumberChildren() ; ++rootTagIndex) {
				tag = new ElementXML();
				rootTag.GetChild(tag, rootTagIndex);
	
				if (tag.IsTag(Names.kTagAgent)) {
					tanksoarAgent(tag);
				} else {
					throw new AnalyzeError("Unrecognized tag " + tag.GetTagName());
				}
			}
		} finally {
			if (tag != null) {
				tag.delete();
				tag = null;
			}
		}
	}
	
	private void tanksoarAgent(ElementXML tag) {
		TankSoarAgent agent = new TankSoarAgent();
		agent(tag, agent);

		agent.facing = tag.GetAttribute(Names.kParamFacing);

		String energyString = tag.GetAttribute(Names.kParamEnergy);
		if (energyString != null) {
			agent.x = Integer.parseInt(energyString);
		}

		String healthString = tag.GetAttribute(Names.kParamHealth);
		if (healthString != null) {
			agent.x = Integer.parseInt(healthString);
		}

		String missilesString = tag.GetAttribute(Names.kParamMissiles);
		if (missilesString != null) {
			agent.x = Integer.parseInt(missilesString);
		}

		agents.add(agent);
	}
	
	private void display(ElementXML tag) {
		String graphicalString = tag.GetAttribute(Names.kParamGraphical);
		if (graphicalString != null) {
			graphical = Boolean.parseBoolean(graphicalString);
		}
	}
	
	private void simulation(ElementXML tag) {
		String debuggersString = tag.GetAttribute(Names.kParamDebuggers);
		if (debuggersString != null) {
			debuggers = Boolean.parseBoolean(debuggersString);
		}
		
		String maxUpdatesString = tag.GetAttribute(Names.kParamMaxUpdates);
		if (maxUpdatesString != null) {
			maxUpdates = Integer.parseInt(maxUpdatesString);
		}
		
		String winningScoreString = tag.GetAttribute(Names.kParamWinningScore);
		if (winningScoreString != null) {
			winningScore = Integer.parseInt(winningScoreString);
		}
		
		String remoteString = tag.GetAttribute(Names.kParamRemote);
		if (remoteString != null) {
			remote = Boolean.parseBoolean(remoteString);
		}
		
		String randomString = tag.GetAttribute(Names.kParamRandom);
		if (randomString != null) {
			random = Boolean.parseBoolean(randomString);
		}
	}
	
	private void client(ElementXML tag) {
		Client client = new Client();
		
		client.name = tag.GetAttribute(Names.kParamName);
		
		client.command = tag.GetAttribute(Names.kParamCommand);
		
		String timeoutString = tag.GetAttribute(Names.kParamTimeOut);
		if (timeoutString != null) {
			client.timeout = Integer.parseInt(timeoutString);
		}
		
		String afterString = tag.GetAttribute(Names.kParamAfter);
		if (afterString != null) {
			client.after = Boolean.parseBoolean(afterString);
		}
		
		clients.add(client);
	}

	public String getMapPath() {
		// TODO Auto-generated method stub
		return null;
	}

	public String getAgentPath() {
		// TODO Auto-generated method stub
		return null;
	}

	public boolean getSpawnDebuggers() {
		// TODO Auto-generated method stub
		return false;
	}

	public String getDebuggerName() {
		// TODO Auto-generated method stub
		return null;
	}
}
