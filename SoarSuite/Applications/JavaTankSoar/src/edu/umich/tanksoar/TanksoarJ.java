package edu.umich.tanksoar;

/* File: TanksoarJ.java
 * Aug 11, 2004
 */

import java.io.File;

import edu.umich.JavaBaseEnvironment.Logger;
import edu.umich.JavaBaseEnvironment.SoarJavaParser;
import edu.umich.JavaBaseEnvironment.SoarJavaParser.argumentContainer;

/**
 * Runs the TankSoar simulation, and allows command line commands, too. For an
 * explanation of the commands, run this class with the command line flag -? or
 * -help.
 * 
 * @author John Duchi
 */
public class TanksoarJ {

	public static void main(String[] args) {
		TankSoarLogger.log("---------------------------------------");
		TankSoarLogger.log("Beginning TS....");
		SoarJavaParser.argumentContainer ac = SoarJavaParser.parseAll(args);
		TankSoarJControl tc;

		if (ac.needHelp) {
			SoarJavaParser.printHelp();
			return;
		}
		if (ac.mapFile != null) {
			tc = new TankSoarJControl(new File(ac.mapFile));
		} else {
			tc = new TankSoarJControl();
		}
		if (ac.logPath != null) {
			tc.logger = new Logger(ac.logPath,ac.clearLog,ac.logVerbosity);
		} else {
			tc.logger = new Logger();
		}
		tc.commandLine = ac;
		logCommandLine(ac,tc.logger);
		
		if (ac.soarFiles != null) {
			for (int i = 0; i < ac.soarFiles.length; i++) {
				TankSoarLogger
						.log("attempting to load a soar agent, from main(), with the file name "
								+ ac.soarFiles[i]);

				tc.loadAgent(new File(ac.soarFiles[i]), null);
			}
		}
		//		else
		//		  TankSoarLogger.log("\tNo soar agent files were specified for loading,
		// so nothing done from main()");

		//Have to update all tanks here, because a tool might cause the Soar
		// kernel
		//to start running at any moment. The tanks must already be set to run
		tc.updateAllTanks();

		if (!ac.allWindowsOff) {
			TankSWindowManager wm = new TankSWindowManager(tc,
					ac.agentWindowsOn, ac.visualMapOn);
		} else {
			if (tc.getAllAgents().length != 0) {
				tc.runSimulation();
				tc.printMap();
				System.out.println("World count: " + tc.getWorldCount());
			} else {
				System.out.println("No agents specified. Quitting.");
				//return;
			}
			tc.simQuit();
		}
		TankSoarLogger.close();
	}
	
	private static String BoolAsOnOff(boolean b)
	{
		if (b)
			return "ON";
		else
			return "OFF";
	}
	
	private static String BoolAsWillNot(boolean b)
	{
		if (b)
			return "WILL";
		else
			return "WILL NOT";
	}
	
	private static String SelfOrNotSpecified(String s)
	{
		if (s != null && s != "")
			return '\"' + s + '"';
		else
			return "NOT SPECIFIED";
	}
	
	private static void logCommandLine(argumentContainer ac,Logger logger)
	{
		logger.log("---Command-line Options---");
		
		logger.log("\nAgent windows are " + BoolAsOnOff(ac.agentWindowsOn));
		logger.log("\nAll windows are " + BoolAsOnOff(!ac.allWindowsOff));
		logger.log("\nSimulation " + BoolAsWillNot(ac.autoRun) + " run automatically.");
		logger.log("\nControl panel is " + BoolAsOnOff(!ac.allWindowsOff));
		logger.log("\nLog path is " + SelfOrNotSpecified(ac.logPath));
		logger.log("\nLog verbosity is set to " + ac.logVerbosity);
		logger.log("\nMap file is " + SelfOrNotSpecified(ac.mapFile));
		
		if (ac.seaterErrorLvl == 0)
			logger.log("\nStank errors will be silently worked around.");
		else if (ac.seaterErrorLvl == 1)
			logger.log("\nStank errors will be worked around with a warning.");
		else
			logger.log("\nStank errors will cause a fatal error.");
		
		logger.log("\nVisual map is " + BoolAsOnOff(ac.visualMapOn) + '\n');
		
		if (ac.soarFiles != null && ac.soarFiles.length != 0)
		{
			logger.log("Soar files to source are: \n");
			
			for (int i = 0;i < ac.soarFiles.length;i++)
			{
				logger.log('\t' + ac.soarFiles[i] + '\n');
			}
		}
		else
			logger.log("No Soar files to source were specified.\n");
		
		if (ac.andyMode)
			logger.log("Andy mode enabled, tanks will only get one Soar cycle per turn.\n");
		else
			logger.log("Tanks will get fifteen Soar cycles per world turn.\n");
		
		logger.log("\n\n---TankSoar Simulation---\n");
	}
}