/* File: Eaters.java
 * Jul 12, 2004
 */

package edu.umich.eaters;

import edu.umich.JavaBaseEnvironment.Logger;
import edu.umich.JavaBaseEnvironment.SoarJavaParser;
import edu.umich.JavaBaseEnvironment.SoarJavaParser.argumentContainer;

import java.io.File;

/**
 * The main class to run Eaters.
 * @author John Duchi
 */
public class Eaters {

	public static void main(String[] args)
	{
		SoarJavaParser.argumentContainer ac = SoarJavaParser.parseAll(args);
		EaterControl ec;
		if(ac.needHelp){
			SoarJavaParser.printHelp();
			return;
		}
		if(ac.mapFile != null){
			ec = new EaterControl(new File(ac.mapFile));
		} else {
			ec = new EaterControl();
		}
		if (ac.logPath != null)
			ec.logger = new Logger(ac.logPath,ac.clearLog,ac.logVerbosity);
		else
			ec.logger = new Logger();
		
		ec.commandLine = ac;
		logCommandLine(ac,ec.logger);
		
		if(ac.soarFiles != null){
			for(int i = 0; i < ac.soarFiles.length; i++){
				ec.loadAgent(new File(ac.soarFiles[i]));
			}
		}
		if (ac.autoRun)
			ec.run();

		if(!ac.allWindowsOff){
			EaterSWindowManager wm = new EaterSWindowManager(ec, ac.agentWindowsOn, ac.visualMapOn, ac.controlPanelOn,ac.autoRun);
		}
		
		System.exit(0);//Oddity in SML requires this to ensure no memory leaked
	}
	
	private static String OhBool(boolean b)
	{
		if (b)
			return "ON";
		else
			return "OFF";
	}
	
	private static String OhBool2(boolean b)
	{
		if (b)
			return "WILL";
		else
			return "WILL NOT";
	}
	
	private static String OhNull(String s)
	{
		if (s != null && s != "")
			return '\"' + s + '\"';
		else
			return "NOT SPECIFIED";
	}
	
	private static void logCommandLine(argumentContainer ac,Logger logger)
	{
		logger.log("---Command-line Options---");
		
		logger.log("\nAgent windows are " + OhBool(ac.agentWindowsOn));
		logger.log("\nAll windows are " + OhBool(!ac.allWindowsOff));
		logger.log("\nSimulation " + OhBool2(ac.autoRun) + " run automatically.");
		logger.log("\nControl panel is " + OhBool(!ac.allWindowsOff));
		logger.log("\nLog path is " + OhNull(ac.logPath));
		logger.log("\nLog verbosity is set to " + ac.logVerbosity);
		logger.log("\nMap file is " + OhNull(ac.mapFile));
		
		if (ac.seaterErrorLvl == 0)
			logger.log("\nSeater errors will be silently worked around.");
		else if (ac.seaterErrorLvl == 1)
			logger.log("\nSeater errors will be worked around with a warning.");
		else
			logger.log("\nSeater errors will cause a fatal error.");
		
		logger.log("\nVisual map is " + OhBool(ac.visualMapOn) + '\n');
		
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
			logger.log("Warning: no-time-to-think (Andy mode) has no effect in Eaters.\n");
		
		logger.log("\n\n---Eaters Simulation---\n");
	}
}
