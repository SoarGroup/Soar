/* File: SoarJavaParser.java
 * Sep 15, 2004
 */
package edu.umich.JavaBaseEnvironment;

import java.util.ArrayList;

import javax.swing.JOptionPane;

/**
 * Implements parsing of the strings passed as arguments to a Soar simulation in
 * Java.
 * 
 * @author John Duchi
 */
public class SoarJavaParser {

	/**
	 * Provides the information necessary to set up the UI for Eaters.
	 */
	public static class argumentContainer {
		/**
		 * <code>String</code> names of .soar files for soar agents.
		 */
		public String[] soarFiles;

		/**
		 * String name of a map file to load when simulation begins.
		 */
		public String mapFile;

		/**
		 * True if the VisMap for an Eaters simulation should be open.
		 */
		public boolean visualMapOn;

		/**
		 * True if, on creation of an agent in the simulation, an agent view
		 * should open for the agent.
		 */
		public boolean agentWindowsOn;

		/**
		 * True if the control panel should be visible
		 */
		public boolean controlPanelOn;

		/**
		 * True if no windows should be shown in the simulation.
		 */
		public boolean allWindowsOff;

		/**
		 * True if help should be printed out for how to run the simulation.
		 */
		public boolean needHelp;

		/**
		 * True if we should begin running immediately after agents connected
		 */
		public boolean autoRun;
		
		/**
		 * True if we should update the world once per cycle instead of the perscribed 15 cycles (tanksoar only)
		 */
		public boolean andyMode = false;

		/**
		 * Path to a log file to append run information (not debug stuff)
		 */
		public String logPath = null;

		/**
		 * Log verbosity: ranges from 0 to 5 (inclusive) see Eaters Notes.txt
		 * for details
		 */
		public int logVerbosity = 0;

		/**
		 * Seater error level: 0 = none, 1 = warn, 2 = quit (see Eaters
		 * Notes.txt for details)
		 */
		public int seaterErrorLvl = 0;
		
		/**
		 * Whether or not to clear log before writing
		 */
		public boolean clearLog = false;
	}

	/**
	 * Parses all the command-line arguments into the program.
	 * 
	 * @param args
	 *            The arguments to be parsed for a Soar simulation. Usually the
	 *            arguments passed into the program at command line.
	 * @return An <code>argumentContainer</code> that contains values for all
	 *         the variables that determine how the simulation will work.
	 */
	public static argumentContainer parseAll(String[] args) {
		argumentContainer ac = new argumentContainer();
		ac.visualMapOn = true;
		ac.controlPanelOn = true;
		ac.agentWindowsOn = true;
		ac.seaterErrorLvl = 0;
		ac.needHelp = false;
		ac.autoRun = false;
		if (args == null)
			return (ac);

		ArrayList soarFiles = new ArrayList();

		for (int i = 0; i < args.length; i++) {
			if (args[i].equalsIgnoreCase("-m")
					|| args[i].equalsIgnoreCase("--map")) {
				if (args.length > (i + 1)) {
					++i;
					ac.mapFile = args[i];
				}
			} else if (args[i].equalsIgnoreCase("--warn-seater") || args[i].equalsIgnoreCase("--warn-stank"))
				ac.seaterErrorLvl = 1;
			else if (args[i].equalsIgnoreCase("--error-seater") || args[i].equalsIgnoreCase("--error-stank"))
				ac.seaterErrorLvl = 2;
			else if (args[i].equalsIgnoreCase("-c") || args[i].equalsIgnoreCase("--clear-log"))
				ac.clearLog = true;
			else if (args[i].equalsIgnoreCase("-v")
					|| args[i].equalsIgnoreCase("--verbosity")) {
				i++;
				ac.logVerbosity = new Integer(args[i]).intValue();
				if (ac.logVerbosity < 0 || ac.logVerbosity > 8) {
					JOptionPane
							.showMessageDialog(
									null,
									"Fatal error during command line parse: Verbosity level must be 0-8 inclusive.",
									"Fatal Error", JOptionPane.ERROR_MESSAGE);
					System.exit(-1);
				}
			} else if (args[i].equalsIgnoreCase("-r")
					|| args[i].equalsIgnoreCase("--run"))
				ac.autoRun = true;
			else if (args[i].equalsIgnoreCase("-l")
					|| args[i].equalsIgnoreCase("--log")) {
				i++;
				ac.logPath = args[i];
			} else if (args[i].toLowerCase().endsWith(".soar")
					|| args[i].toLowerCase().endsWith(".seater") 
					|| args[i].toLowerCase().endsWith(".stank")) {
				soarFiles.add(args[i]);
			} else if (args[i].equalsIgnoreCase("-a")
					|| args[i].equalsIgnoreCase("--agent")) {
				if (args.length > (i + 1)) {
					++i;
					if (args[i].equalsIgnoreCase("off")) {
						ac.agentWindowsOn = false;
					}
				}
			} /*
				 * else if (args[i].equalsIgnoreCase("-c") ||
				 * args[i].equalsIgnoreCase("--controls")) { if (args.length >
				 * (i + 1)) { ++i; if (args[i].equalsIgnoreCase("off")) {
				 * ac.controlPanelOn = false; } } }
				 */else if (args[i].equalsIgnoreCase("-b")
					|| args[i].equalsIgnoreCase("--board")) {
				if (args.length > (i + 1)) {
					++i;
					if (args[i].equalsIgnoreCase("off")) {
						ac.visualMapOn = false;
					}
				}
			} else if (args[i].equalsIgnoreCase("--no-time-to-think")
					|| args[i].equalsIgnoreCase("--andy-mode")
					|| args[i].equalsIgnoreCase("-e")) {		
				ac.andyMode = true;
			} else if (args[i].equalsIgnoreCase("-?")
					|| args[i].equalsIgnoreCase("-help")) {
				ac.needHelp = true;
			} else if (args[i].equalsIgnoreCase("-w")
					|| args[i].equalsIgnoreCase("--windows")) {
				if (args.length > (i + 1)) {
					++i;
					if (args[i].equalsIgnoreCase("off")) {
						ac.allWindowsOff = true;
					}
				}
			}
		}
		if (ac.allWindowsOff) {
			ac.agentWindowsOn = false;
			ac.visualMapOn = false;
		}
		if (soarFiles.size() > 0) {
			ac.soarFiles = (String[]) soarFiles.toArray(new String[0]);
		}
		return (ac);
	}

	public static void printHelp() {
		System.out.println("Java-Soar Help");
		System.out.println("Command line arguments:");
		System.out
				.println("\t-b [on/off] Sets the visible board to be on or off.");
		System.out
				.println("\t-a [on/off] Sets agent windows to be on or off (viewing one specific agent).");
		System.out
				.println("\t-w [on/off] Sets all the windows to be off if off."
						+ "\n\t\tIf on, allows windows to be open (In this case, overridden by –b, –c, and –a)");
		System.out
				.println("\t-m [path] Sets map file specified by path to be loaded when run.");
		System.out
				.println("\t*.soar Any argument ending in .soar or .seater the simulation will attempt to load as an agent");
		System.out.println("\t-? Prints help");
		System.out.println("\t-help Prints help");
	}
}
