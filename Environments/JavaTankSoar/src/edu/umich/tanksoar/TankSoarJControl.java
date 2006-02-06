package edu.umich.tanksoar;

/* File: TankSoarJControl.java
 * Aug 11, 2004
 */

import edu.umich.JavaBaseEnvironment.*;
import edu.umich.JavaBaseEnvironment.SoarJavaParser.argumentContainer;
import sml.Agent;
import sml.Kernel;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;

import javax.swing.JOptionPane;// TODO don't use swing, maybe. Is it bad to mix
								// swing and swt?
import java.io.*;
import java.util.*;

/**
 * The model behind a TankSoar simulation, this maintains the map and keeps
 * track of the agents in the simulation, notifying them when they need to
 * calculate moves and updating listeners as necessary.
 * 
 * @author John Duchi
 */
public class TankSoarJControl extends SimulationControl implements
		Kernel.UpdateEventInterface, Kernel.SystemEventInterface,
		SimulationControlListener, Runnable {

	/**
	 * The Soar kernel that will be used to create agents corresponding to
	 * individual tanks. This does limit the game to just using one kernel for
	 * all of the agents.
	 */
	protected static Kernel kernel = null;

	private String soarLibraryName = "SoarKernelSML";

	/**
	 * The score at which one <code>Tank</code> is decided to have won the
	 * game.
	 */
	public static final int FinalTankScore = 50;

	/** The height of the TankSoar map. */
	public static final int MapHeight = 16;

	/** The width of the TankSoar map. */
	public static final int MapWidth = 16;

	/** The random number generator used to calculate open locations. */
	private Random myRandom = null;

	/**
	 * An <code>ArrayList</code> of all the missiles that have been shot and
	 * are currently 'flying' in the simulation.
	 */
	private ArrayList flyingMissiles = new ArrayList();

	private boolean firstStep = true;

	private boolean soarIsRunning = false;

	//
	private HashMap decisions = new HashMap();

	/**
	 * Constructs a new instance of <code>TankSoarJControl</code> by calling
	 * <code>newOpenMap()</code>.
	 */
	public TankSoarJControl() {
		TankSoarLogger.log("TankSoarJControl: zero arg constructor...");
		// TankSoarLogger.log("\tTankSoarJControl::constructor creating kernel
		// inprocess");
		// kernel = Kernel.CreateKernelInCurrentThread(soarLibraryName, false,
		// 12121);
		kernel = Kernel.CreateKernelInNewThread(soarLibraryName, 12121);
		// kernel = Kernel.CreateRemoteConnection(true,"127.0.0.1");

		checkKernelStatus();
		activateDetailedKernelLogging(false);

		registerForSoarStopEvent();
		registerForSoarStartEvent();

		int callbackid = 0;
		// Register for Soar update event
		callbackid = kernel.RegisterForUpdateEvent(
				smlUpdateEventId.smlEVENT_AFTER_ALL_GENERATED_OUTPUT, this,
				null);
		if (callbackid > 0)
			TankSoarLogger.log("Registered for Soar update event.");// Registration
																	// successful
		else {
			JOptionPane
					.showMessageDialog(
							null,
							"Fatal error during kernel initialization: Failed to register for Soar's update event",
							"Fatal Error", JOptionPane.ERROR_MESSAGE);
			System.exit(-1);
		}

		// need to listen for quit event so can delete Soar kernel on exit
		addSimulationControlListener(this);
		
		// Set up map, agent paths
		String basePath = kernel.GetLibraryLocation() + System.getProperty("file.separator")
		+ ".." + System.getProperty("file.separator") 
		+ "Environments" + System.getProperty("file.separator") 
		+ "JavaTanksoar" + System.getProperty("file.separator");
		
		agentPath = basePath + "agents";
		mapPath   = basePath + "maps";

		File f = new File(mapPath, "default.tmap");
		loadMap(f);
		
		// TankSoarLogger.log(f.getPath());
		// newOpenMap();//opens an empty map
	}

	/**
	 * Cretaes a new instance of <code>TankSoarJControl</code> using the map
	 * specified at <code>infile</code> as the map to be used for the
	 * simulation.
	 * 
	 * @param infile
	 *            Expected to be a properly formatted map file for TankSoar.
	 * @see TankSoarJControl#loadMap(File)
	 */
	public TankSoarJControl(File infile) {
		TankSoarLogger.log("TankSoarJControl: inFile constructor...");
		// TankSoarLogger.log("TankSoarJControl::constructor creating kernel
		// inprocess");
		// kernel = Kernel.CreateKernelInCurrentThread(soarLibraryName, false,
		// 12121);
		kernel = Kernel.CreateKernelInNewThread(soarLibraryName, 12121);
		// kernel = Kernel.CreateRemoteConnection(true, "127.0.0.1");

		checkKernelStatus();

		activateDetailedKernelLogging(false);
		registerForSoarStopEvent();
		registerForSoarStartEvent();

		// Register for Soar update event
		int callbackid = 0;
		callbackid = kernel.RegisterForUpdateEvent(
				smlUpdateEventId.smlEVENT_AFTER_ALL_GENERATED_OUTPUT, this,
				null);
		if (callbackid > 0)
			TankSoarLogger.log("Registered for Soar update event.");// Registration
																	// successful
		else {
			JOptionPane
					.showMessageDialog(
							null,
							"Fatal error during kernel initialization: Failed to register for Soar's update event",
							"Fatal Error", JOptionPane.ERROR_MESSAGE);
			System.exit(-1);
		}

		// need to listen for quit event so can delete Soar kernel on exit
		addSimulationControlListener(this);

		// TankSoarLogger.log("\tAbout to load map: " +
		// infile.getAbsolutePath());
		loadMap(infile);
	}

	private void checkKernelStatus() {
		if (kernel == null)// this doesn't really happen with our SWIG wrapped
							// stuff. Failure will yeild a non-null reference
			TankSoarLogger
					.log("\tTankSoarJControl::constructor kernel was null after supposed creation.");
		else {
			TankSoarLogger.log("\tKernel actually created");

			if (kernel.HadError()) {
				TankSoarLogger
						.log("\tError during kernel creation, but instance isn't null");
				JOptionPane.showMessageDialog(null,
						"Fata error during kernel initialization .",
						"Fatal Error", JOptionPane.ERROR_MESSAGE);
				System.exit(-1);
			}
		}
	}

	public Agent getAgentByColor(String color) {
		SoarAgent[] es = getAllAgents();

		for (int i = 0; i < es.length; i++) {
			if (es[i].getColorName() == color) {
				return ((Tank) es[i]).agent;
			}
		}

		return null;
	}

	public SoarAgent getSoarAgentByColor(String color) {
		SoarAgent[] es = getAllAgents();

		for (int i = 0; i < es.length; i++) {
			if (es[i].getColorName() == color) {
				return es[i];
			}
		}

		return null;
	}

	private void activateDetailedKernelLogging(boolean should) {
		if (!should)
			return;
		kernel.SetTraceCommunications(true);
		if (!kernel.GetLastCommandLineResult())
			TankSoarLogger.log("Failed to turn on detailed trace logging.");
		else
			TankSoarLogger
					.log("Succeeded in turning on detailed trace logging.");
	}

	private void registerForSoarStopEvent() {
		int callbackid = kernel.RegisterForSystemEvent(
				smlSystemEventId.smlEVENT_SYSTEM_STOP, this, null);
		if (callbackid > 0)
			TankSoarLogger
					.log("Successfully registered for Soar kernel stop event.");
		else {
			TankSoarLogger
					.log("Failed to successfully register for Soar kernel stop event.");
			TankSoarLogger.log("\t" + kernel.GetLastErrorDescription());
		}
	}

	private void registerForSoarStartEvent() {
		int callbackid = kernel.RegisterForSystemEvent(
				smlSystemEventId.smlEVENT_SYSTEM_START, this, null);
		if (callbackid > 0)
			TankSoarLogger
					.log("Successfully registered for Soar kernel start event.");
		else {
			TankSoarLogger
					.log("Failed to successfully register for Soar kernel start event.");
			TankSoarLogger.log("\t" + kernel.GetLastErrorDescription());
		}
	}

	/**
	 * Character constant for an <code>TSEmpty</code> square when reading a
	 * file.
	 */
	public static final char Empty = '.';

	/** Character constant for a <code>TSWall</code> when reading a file. */
	public static final char Wall = '#';

	/**
	 * Character constant for an <code>EnergySquare</code> when reading a
	 * file.
	 */
	public static final char Energy = 'e';

	/** Character constant for a <code>HealthSquare</code> when reading a file. */
	public static final char Health = 'h';

	/**
	 * Character constant for a <code>MissileBucket</code> when reading a
	 * file.
	 */
	public static final char Missile = 'm';

	/**
	 * The proper formatting of the file is as follows.
	 * <p>
	 * The first line is a single character, either a 0 or a 1. If it is a 1,
	 * the simulation will use deterministically random numbers for missile
	 * placement, setting the random parameter in the <code>Tanks</code>'
	 * input-link, and placing energy and health squares in the simulation. If
	 * it is a 0, the simulation will seed all random numbers 'randomly', based
	 * on the current system time.
	 * </p>
	 * <p>
	 * The next <code>MapHeight</code> lines of the file must be of width
	 * <code>MapWidth</code> and contain characters indicating the type of
	 * square to be placed at the location in the map they represent, with
	 * characters at the top left representing locations in the top left of the
	 * simulation's map. The characters are as follows:
	 * </p>
	 * <ul>
	 * <li>'.' An <code>TSEmpty</code> square (think grass).
	 * <li>'#' A <code>TSWall</code> (or obstacle).
	 * <li>'e' An <code>EnergySquare</code>.
	 * <li>'h' A <code>HealthSquare</code>.
	 * <li>'m' A <code>MissileBucket</code>
	 * </ul>
	 * {@inheritDoc}
	 */
	public void loadMap(File infile) {
		stopSimulation();
		myWorldCount = 0;
		myWinner = null;
		energyCreated = healthCreated = false;
		boolean setRandoms = false;
		BufferedReader bIn = null;
		try {
			bIn = new BufferedReader(new FileReader(infile));
		} catch (FileNotFoundException f) {
			mapOpeningError(f.getMessage());
			TankSoarLogger.log("\terror in loading map..." + f.getMessage());
			// return; Duchi doesn't return when this is caught.
		}
		TankSoarLogger.log("\tMap file loaded.");
		try {
			// determine if map is a Tcl map or Java one. Tcl maps have strings
			// as first meta data chars,
			// JavaTankSoar maps have a digit as the first meta data character
			// //TODO actually do this
			TSWall wall = new TSWall();
			if (bIn != null) {
				char[] cs = new char[(int) infile.length()];
				char type;
				int x = 0, y = 0, i = 0;
				bIn.read(cs);
				try {
					while (cs[i] != '1' && cs[i] != '0') {
						++i;
					}
				} catch (ArrayIndexOutOfBoundsException e) {
					mapOpeningError("File improperly formatted.\nNo random bit");
					return;
				}// catch

				if (cs[i] == '1') {
					setRandoms = true;
				}

				for (; i < cs.length; ++i) {
					type = cs[i];
					switch (type) {
					case (Empty):
						myMap[x][y] = new TSEmpty();
						++x;
						break;
					case (Wall):
						myMap[x][y] = wall;
						++x;
						break;
					case (Energy):

					{
						myMap[x][y] = new EnergySquare();
						energyCreated = true;
						++x;
						break;
					}
					case (Health):

					{
						myMap[x][y] = new HealthSquare();
						healthCreated = true;
						++x;
						break;
					}
					case (Missile):

					{
						myMap[x][y] = new TSEmpty();
						new MissileBucket((TSEmpty) myMap[x][y], this);
						++x;
						break;
					}
					case ('\n'):
						x = 0;
						break;
					default:
						break;
					}// switch
					if (x == MapWidth) {
						x = 0;
						++y;
					}// if
				}// for loop

			}// if
		}// try
		catch (IOException ioe) {
			mapOpeningError(ioe.getMessage());
			return;
		} catch (NullPointerException n) {
			mapOpeningError(null);
			return;
		}

		try {
			if (bIn != null)
				bIn.close();

		} catch (IOException ignored) {
		}

		if (setRandoms)
			setRandomSeeds();
		else
			unsetRandomSeeds();

		if (!healthCreated)
			placeHealthSquare();

		if (!energyCreated)
			placeEnergySquare();

		placeAllTanks();
		fireNewMapNotification((setRandoms ? new String(
				"Deterministically random setting will be used to run simulation.\nRandoms seeded")
				: null));
	}

	protected void loadTclMap() {

	}

	protected void loadJavaMap() {

	}

	/**
	 * Function called when there is an error when opening a map for TankSoar.
	 * Creates a new open map, resets all the agents, and fires a new map
	 * notification for the listeners.
	 * 
	 * @param message
	 *            The message associated with the exception or error, if there
	 *            is any.
	 */
	public void mapOpeningError(String message) {
		if (message != null) {
			System.out.print(message + " ");
		}
		System.out.println("Loading open map");
		newOpenMap();
		resetAgents();
		fireNewMapNotification(null);
	}

	public int getMapHeight() {
		return (MapHeight);
	}

	public int getMapWidth() {
		return (MapWidth);
	}

	/**
	 * Creates an empty, obstacle-free map for the simulation. Will put in a
	 * recharger and a health square.
	 */
	public void newOpenMap() {
		stopSimulation();
		myWinner = null;
		myMap = new Object[MapWidth][MapHeight];
		initializeBoundaries();
		healthCreated = energyCreated = false;
		fillGrasses();
		placeMissileBucket();
		placeMissileBucket();
		placeAllTanks();
		fireNewMapNotification(null);
	}

	/**
	 * Initializes all the boundaries of the map to be obstacles so no
	 * <code>Tank</code>s can go out of bounds.
	 */
	private void initializeBoundaries() {
		TSWall obs = new TSWall();
		for (int x = 0; x < MapWidth; ++x) {
			myMap[x][0] = obs;
			myMap[x][MapHeight - 1] = obs;
		}
		for (int y = 0; y < MapHeight; ++y) {
			myMap[0][y] = obs;
			myMap[MapWidth - 1][y] = obs;
		}
	}

	/**
	 * Fills all non-null squares in the map with <code>TSEmpty</code>s, so
	 * they are enterable by <code>Tank</code>s. Also randomly places a
	 * <code>HealthSquare</code> and an <code>EnergySquare</code>.
	 */
	private void fillGrasses() {
		for (int x = 1; x < MapWidth - 1; ++x) {
			for (int y = 1; y < MapHeight - 1; ++y) {
				if (myMap[x][y] == null) {
					myMap[x][y] = new TSEmpty();
				}
			}
		}
		placeHealthSquare();
		placeEnergySquare();
	}

	/**
	 * <code>true</code> if there has been a <code>HealthSquare</code>
	 * placed in this simulation.
	 */
	private boolean healthCreated = false;

	/**
	 * <code>true</code> if there has been an <code>EnergySquare</code>
	 * placed in this simulation.
	 */
	private boolean energyCreated = false;

	/**
	 * Randomly places a <code>HealthSquare</code> in the simulation using the
	 * <code>findEmptyLocation()</code> method. If a <code>HealthSquare</code>
	 * has already been placed, no action is taken.
	 * 
	 * @see TankSoarJControl#findEmptyLocation()
	 */
	private void placeHealthSquare() {
		if (!healthCreated) {
			Location hsLoc = findEmptyLocation();
			myMap[hsLoc.getX()][hsLoc.getY()] = new HealthSquare();
			healthCreated = true;
		}
	}

	/**
	 * Randomly places an <code>EnergySquare</code> in the simulation using
	 * the <code>findEmptyLocation()</code> method. If an
	 * <code>EnergySquare</code> has already been placed, no action is taken.
	 * 
	 * @see TankSoarJControl#findEmptyLocation()
	 */
	private void placeEnergySquare() {
		if (!energyCreated) {
			Location esLoc = findEmptyLocation();
			myMap[esLoc.getX()][esLoc.getY()] = new EnergySquare();
			energyCreated = true;
		}
	}

	/**
	 * Places one <code>MissileBucket</code> at a random <code>TSEmpty</code>
	 * square in the simulation. Increments the ivar storing the number of
	 * <code>MissileBucket</code>s in the simulation.
	 * 
	 * @see edu.umich.tanksoar#TankSoarJControl#findEmptyLocation()
	 */
	private void placeMissileBucket() {
		Location loc = findEmptyLocation();
		new MissileBucket((TSEmpty) myMap[loc.getX()][loc.getY()], this);
		missileBucketCount++;
		turnsSinceMissile = 0;
		fireLocationChangedNotification(loc);
	}

	/**
	 * Notification called when a <code>MissileBucket</code> has been picked
	 * up by a <code>Tank</code>, called by the <code>MissileBucket</code>
	 * itself. Decrements the ivar storing the number of
	 * <code>MissileBucket</code>s that are in the simulation.
	 * 
	 * @param pickedUp
	 *            The <code>MissileBucket</code> that has been picked up.
	 */
	public void pickedUpMissiles(MissileBucket pickedUp) {
		--missileBucketCount;
	}

	/**
	 * <p>
	 * Lists files in maps directory and randomly loads one.
	 * {@inheritDoc}
	 */
	public void newRandomMap() {
		
		// list tmap files
		File mapDir = new File(mapPath);
		File[] files = mapDir.listFiles();

		// pick one
		Random random = new Random();
		File newMap = null;
		do {
			int index = random.nextInt(files.length);
			if (files[index].isFile()) {
				newMap = files[index];
			}
		} while (newMap == null);
		
		// load it
		loadMap(newMap);
	}

	/**
	 * Places all the <code>Tank</code>s in the simulation at random places
	 * in the map. If the random seed has been set, should work the same way
	 * every time.
	 */
	private void placeAllTanks() {
		Iterator iter = myAgents.iterator();
		while (iter.hasNext()) {
			Location loc = findEmptyLocation();
			Tank t = (Tank) iter.next();
			t.setLocation(loc);
			t.reset();
			t.setDirection((int) (getNextRandomDouble() * 4));
		}
	}

	/**
	 * Prints the the statistics for all the <code>Tank</code>s in the
	 * simulation. {@inheritDoc}
	 */
	public void printMap() {
		PrintStream so = System.out;
		for (int y = 0; y < MapHeight; y++) {
			for (int x = 0; x < MapHeight; x++) {
				if (myMap[x][y] instanceof TSEmpty) {
					if (((EnterableSquare) myMap[x][y]).containsAgent()) {
						so.print('+');
					} else if (((EnterableSquare) myMap[x][y])
							.containsMissiles()) {
						so.print('M');
					} else if (((EnterableSquare) myMap[x][y]).getOccupants().length > 0) {
						so.print('-');
					} else {
						so.print(' ');
					}
				} else if (myMap[x][y] instanceof TSWall) {
					so.print('#');
				} else if (myMap[x][y] instanceof EnergySquare) {
					so.print('E');
				} else if (myMap[x][y] instanceof HealthSquare) {
					so.print('H');
				} else {
					so.print('?');
				}
				so.print(' ');
			}
			so.print('\n');
		}
		printAgentStats();
	}

	/**
	 * A simple static nested class that contains a <code>Tank</code>, a
	 * <code>Location</code>, and an <code>int</code> specifying the
	 * <code>Tank</code>'s direction, used for recalling the original
	 * <code>Location</code>s and directions of the <code>Tank</code>s
	 * upon creation.
	 */
	private static class TankLocationTriple {
		private Tank tank;

		private Location loc;

		private int direction;

		/**
		 * Constructs a <code>TankLocationTriple</code> with the specified
		 * parameters.
		 * 
		 * @param t
		 *            The <code>Tank</code> to be stored.
		 * @param l
		 *            The <code>Tank</code>'s <code>Location</code>.
		 * @param d
		 *            The direction the <code>Tank</code> faces, as defined in
		 *            <code>Tank</code>.
		 */
		private TankLocationTriple(Tank t, Location l, int d) {
			tank = t;
			loc = l;
			direction = d;
		}
	}

	/**
	 * An <code>ArrayList</code> containing all the original locations and
	 * directions of the <code>Tank</code>s in the simulation. Made up of
	 * <code>TankLocationTriple</code>s.
	 */
	private ArrayList originalLocations = new ArrayList(8);

	/**
	 * Resets the map and all the <code>Tank</code>s in it to the positions
	 * at which they were originally located, with their correct directions.
	 * This, however, does not reset the missiles, unfortunately, so the running
	 * may be slightly different.
	 */
	public void resetMap() {
		stopSimulation();
		if (randomSeedsSet) {
			setRandomSeeds();
		} else {
			unsetRandomSeeds();
		}
		Iterator iter = originalLocations.iterator();
		while (iter.hasNext()) {
			TankLocationTriple triple = (TankLocationTriple) iter.next();
			triple.tank.setLocation(triple.loc);
			triple.tank.setDirection(triple.direction);
			triple.tank.reset();
		}
		fireNewMapNotification(null);
	}

	protected void finalize() {
		if (kernel != null) {
			ListIterator agentItr = myAgents.listIterator();
			while (agentItr.hasNext()) {
				try {
					kernel.DestroyAgent(((Tank) agentItr.next()).agent);
					TankSoarLogger.log("successfully destructed a Tank");
				} catch (ClassCastException e) {
					TankSoarLogger
							.log("TankSoarJControl::finalize: tried to destruct something that wasn't a Tank.");
				}
			}
		}
	}

	/**
	 * Notifies all <code>Tank</code>s that there has been a change in the
	 * number of agents, allowing them to recalculate their input-link sensors.
	 * {@inheritDoc}
	 */
	public SoarAgent destroyAgent(SoarAgent toDestroy) {
		TankSoarLogger.log("\t\tTankSoarJControl::destroyAgent called...");
		SoarAgent sa = super.destroyAgent(toDestroy);
		if (sa != null) {
			((Tank) sa).cleanUp();
			notifyAllTanksAgentChange();
		}
		return (sa);
	}

	/**
	 * This method clears all the original locations used for resetting the map.
	 * {@inheritDoc}
	 */
	public void destroyAllAgents() {
		TankSoarLogger.log("SimulationControl::destroyAllAgents() called....");
		super.destroyAllAgents();
		originalLocations.clear();
		for (Iterator iter = myAgents.listIterator(); iter.hasNext();) {
			((Tank) iter.next()).cleanUp();
		}
	}

	/**
	 * <code>true</code> if the random number generators for a simulation have
	 * been seeded.
	 */
	private boolean randomSeedsSet = false;

	/**
	 * Using calls to <code>seedRandomNumber()</code>, makes the running of
	 * the simulation deterministically random. Sets the random seeds for
	 * <code>MissileBucket</code>s, <code>Tank</code>s, and this
	 * <code>TankSoarJControl</code>.
	 */
	public void setRandomSeeds() {
		MissileBucket.seedRandomNumber();
		Tank.seedRandomNumber();
		seedRandomNumber();
		randomSeedsSet = true;
	}

	/**
	 * Using calls to <code>unseedRandomNumber()</code>, makes the running of
	 * the simulation non-deterministically random. Unsets the random seeds for
	 * <code>MissileBucket</code>s, <code>Tank</code>s, and this
	 * <code>TankSoarJControl</code>.
	 */
	public void unsetRandomSeeds() {
		MissileBucket.unseedRandomNumber();
		Tank.unseedRandomNumber();
		unseedRandomNumber();
		randomSeedsSet = false;
	}

	/**
	 * The number of <code>MissileBucket</code>s in the simulation. That is,
	 * the number of positions at which a <code>Tank</code> can pick up
	 * missiles.
	 */
	private int missileBucketCount = 0;

	/**
	 * The maximum number of <code>MissileBucket</code>s that can exist in
	 * the simulation at one time.
	 */
	private static final int MaxMissileBucketCount = 3;

	/**
	 * The number of steps that have elapsed since a <code>MissileBucket</code>
	 * was placed.
	 */
	private int turnsSinceMissile = 0;

	/**
	 * update each tank's perception of the world
	 * 
	 */
	public void updateAllTanks() {
		TankSoarLogger.log("TankSoarJControl::updateAllTanks called....");
		for (ListIterator iter = myAgents.listIterator(); iter.hasNext();) {
			Tank curr = ((Tank) iter.next());
			curr.getWorldState();
		}
	}

	/**
	 * Removes the specified agent from the simulation permanently, returning
	 * its color to the simulation.
	 * 
	 * @param destroyed
	 *            The <code>SoarAgent</code> to permanently remove from the
	 *            simulation.
	 */
	protected void removeAgentFromSimulation(SoarAgent destroyed) {
		myMap[destroyed.getLocation().getX()][destroyed.getLocation().getY()] = new TSEmpty();
		fireAgentDestroyedNotification(destroyed);
		myEcs.returnColor(destroyed.getColorName());
	}

	/**
	 * Method used to find the contents of the specified position in the
	 * simulation's internal map.
	 * 
	 * @param x
	 *            The x-coordinate of the location being queried.
	 * @param y
	 *            The y-coordinate of the location being queried.
	 * @return An element of the Simulation. If the x or y coordinate is outside
	 *         the indices of the map, returns a <code>TSWall</code>.
	 */
	public Object getLocationContents(int x, int y) {
		if (0 <= x && x < getMapWidth() && 0 <= y && y < getMapHeight()) {
			return (myMap[x][y]);
		}
		return new TSWall();
	}

	/**
	 * In <code>TankSoarJControl</code>, also calculates whether and where an
	 * new <code>MissileBucket</code>s should be placed, based on the number
	 * of turns since a <code>MissileBucket</code> had been placed and a
	 * random double from <code>MissileBucket</code>. Too, evaluates missiles
	 * flying and <code>Tank</code> collisions. {@inheritDoc}
	 */
	public void singleStep() {
		TankSoarLogger.log("TankSoarJControl::singleStep called....");
		singleStep = true;

		if (simShouldStop()) {
			stopSimulation();
			singleStep = false;
			running = false;
			return;
		}

		// now all decisions have been made, and should be waiting on the output
		// link
		TankSoarLogger.log("TankSoarJControl: Make decision here....");

		for (Iterator iter = myAgents.listIterator(); iter.hasNext();) {
			Tank curr = ((Tank) iter.next());
			// This will set the tank's internal state, so that it matches up
			// with the move
			// that it just decided to do
			decisions.put(curr.getName(), (TankOutputInfo) curr.makeDecision());
		}

		// evaluate all of the decisions that were just made - that is, enact
		// whatever
		// decision was made. Cause the tank to move, fire, whatever.
		TankSoarLogger.log("TankSoarJControl: Evaluate decision here...");
		for (Iterator iter = myAgents.listIterator(); iter.hasNext();) {
			try {
				SoarAgent curAgent = (SoarAgent) iter.next();
				TankOutputInfo curDecision = (TankOutputInfo) decisions
						.get(curAgent.getName());
				if (curDecision != null)
					evaluateDecision(curDecision, curAgent);
			} catch (ClassCastException e) {
				TankSoarLogger
						.log("Something that wasn't a TankOutputInfo ended up in the decisions HashMap, or something that's not a SoarAgent ended up in the list of agents....");
			}
		}

		evaluateCollisions();
		evaluateMissiles();
		resurrectTanks();

		if (missileBucketCount < MaxMissileBucketCount) {
			if ((double) turnsSinceMissile / 50.0 > MissileBucket
					.getNextRandomDouble())
				placeMissileBucket();
			else
				++turnsSinceMissile;
		}

		++myWorldCount;

		fireWorldCountChangedNotification();

		for (Iterator iter = myAgents.listIterator(); iter.hasNext();) {
			Tank subject = (Tank) iter.next();
			subject.allTanksFinished();
			subject.flagAsRun();
			// ((Tank)iter.next()).allTanksFinished();
		}

		updateAllTanks();// so that they are ready to run at any moment
		singleStep = false;
		firstStep = false;
	}

	/**
	 * Iterates over all the missiles flying in the simulation, calling their
	 * <code>fly()</code> methods and checking to see if they successfully
	 * flew. Fires notifications of the changes to the new and old
	 * <code>Location</code>s they cover.
	 */
	private void evaluateMissiles() {
		Iterator mIter = flyingMissiles.iterator();
		while (mIter.hasNext()) {
			FlyingMissile fm = (FlyingMissile) mIter.next();
			Location oldLoc = fm.getLocation();
			boolean flew = fm.fly();
			((EnterableSquare) myMap[oldLoc.getX()][oldLoc.getY()])
					.removeOccupant(fm);
			if (flew) {
				Location newLocation = fm.getLocation();
				fireLocationChangedNotification(newLocation);
			} else {
				mIter.remove();
			}
			fireLocationChangedNotification(oldLoc);
		}
	}

	/**
	 * Given a <code>Tank</code> and its decision, in the form of a
	 * <code>TankOutputInfo</code>, evaluates the decision, setting the
	 * <code>Tank</code>'s position using <code>setTankPosition(Tank,
	 * Location)</code>.
	 * Also, if the decision indicates missiles ought to be shot, checks to make
	 * sure <code>sa</code> can fire missiles, then places
	 * <code>FlyingMissile</code>s in the simulation. Calls
	 * <code>Tank.turnFinished(boolean)</code> to tell the <code>Tank</code>
	 * that it is done and whether its move was successful.
	 * 
	 * @see TankSoarJControl#setTankPosition(Tank, Location) {@inheritDoc}
	 */
	protected void evaluateDecision(Object decision, SoarAgent sa) {
		TankOutputInfo tDecision = (TankOutputInfo) decision;
		Tank t = (Tank) sa;
		boolean moved = false;
		boolean shot = false;
		int direction = t.getDirection();

		Location adjacent = t.getLocation();
		if (tDecision.fire != null) {
			shot = true;
		}
		if (tDecision.move != null) {
			if (tDecision.move.direction.startsWith("l")) {
				adjacent = adjacent.getAdjacent(Tank.getTurnDirection(
						direction, Tank.TURNLEFT));
			} else if (tDecision.move.direction.startsWith("r")) {
				adjacent = adjacent.getAdjacent(Tank.getTurnDirection(
						direction, Tank.TURNRIGHT));
			} else if (tDecision.move.direction.startsWith("f")) {
				adjacent = adjacent.getAdjacent(direction);
			} else if (tDecision.move.direction.startsWith("b")) {
				adjacent = adjacent.getAdjacent(Tank
						.getOppositeDirection(direction));
			}
		}

		moved = setTankPosition(t, adjacent);
		if (shot && t.getMissileCount() >= 0) {
			t.missileFired();
			Location loc = t.getLocation().getAdjacent(direction);
			Object o = myMap[loc.getX()][loc.getY()];
			if (o instanceof EnterableSquare) {
				FlyingMissile fm = new FlyingMissile(t, direction, loc, this);
				((EnterableSquare) o).addOccupant(fm);
				flyingMissiles.add(fm);
				TankSoarLogger
						.log("about to fire changed location notification");
				fireLocationChangedNotification(loc);
			}
		}
		t.turnFinished(moved);
	}

	/**
	 * Gives to the caller an array of all the <code>FlyingMissile</code>s
	 * currently in the "air" in the simulation.
	 * 
	 * @return An array of all the <code>FlyingMissile</code>s in existence.
	 */
	public FlyingMissile[] getFlyingMissiles() {
		return ((FlyingMissile[]) flyingMissiles.toArray(new FlyingMissile[0]));
	}

	/**
	 * Sets the given <code>Tank</code>'s position to be that specified by
	 * the <code>
	 * Location</code>. Checks to make sure the square is
	 * enterable, and, if it is calls the <code>Tank</code>'s
	 * <code>setLocation(Location)</code> method. Otherwise, crashes the
	 * <code>Tank</code> and returns <code>false</code>.
	 * 
	 * @param t
	 *            The <code>Tank</code> whose position is being set.
	 * @param l
	 *            The <code>Location</code> at which to place the
	 *            <code>Tank</code>.
	 * @return <code>true</code> if the <code>Location</code> specified
	 *         contained an <code>EnterableSquare</code> so that the
	 *         <code>Tank</code> could be positioned there, <code>false</code>
	 *         otherwise.
	 */
	private boolean setTankPosition(Tank t, Location l) {
		Location oldLoc = t.getLocation();
		if (oldLoc.equals(l)
				|| getLocationContents(l) instanceof EnterableSquare) {
			t.setLocation(l);
			return true;
		}
		t.crashed();
		return false;
	}

	/**
	 * For the specified <code>Tank</code>'s input link, represented by
	 * <code>sensors</code>, updates the <code>rwaves</code>,
	 * <code>smell</code>, and <code>sound</code> sensors (rather than
	 * simply the radar waves). Iterates over all the other <code>Tank</code>s
	 * in the simulation, checking each of their radars and filling the sensors.
	 * Infeffecient? Yes. But there are only a maximum of seven tanks anyway.
	 * 
	 * @param sensors
	 *            The <code>TankInputInfo</code> representing toFill's input
	 *            link.
	 * @param toFill
	 *            The <code>Tank</code> whose input-link sensors are being
	 *            updated.
	 */
	public void fillRwavesSensor(TankInputInfo sensors, Tank toFill) {
		Iterator iter = myAgents.iterator();
		Tank closest = null;
		int leastDistance = 10000, distance, leastSoundDistance = 10000, soundDirection = -1;
		while (iter.hasNext()) {
			Tank t = (Tank) iter.next();
			if (t != toFill) {
				if ((distance = (Math.abs(toFill.getLocation().getX()
						- t.getLocation().getX()) + Math.abs(toFill
						.getLocation().getY()
						- t.getLocation().getY()))) < leastDistance) {
					sensors.smell.color = t.getColorName();
					sensors.smell.distance = distance;
					leastDistance = distance;
					if (t.movedLastTurn() && leastDistance < Tank.MaxHear) {
						leastSoundDistance = leastDistance;
						soundDirection = toFill.getDirectionFromTank(t
								.getLocation());
					}
				}
				Location[] locs = t.getRadarLocations();
				for (int i = 0; i < locs.length; ++i) {
					if (locs[i] != null && locs[i].equals(toFill.getLocation())) {
						int direction = toFill.getDirectionFromTank(t
								.getLocation());
						switch (direction) {
						case (Tank.FORWARD): {
							sensors.rwaves.forward = TankInputInfo.yes;
							break;
						}
						case (Tank.RIGHT): {
							sensors.rwaves.right = TankInputInfo.yes;
							break;
						}
						case (Tank.LEFT): {
							sensors.rwaves.left = TankInputInfo.yes;
							break;
						}
						case (Tank.BACKWARD): {
							sensors.rwaves.backward = TankInputInfo.yes;
							break;
						}
						}
					}
				}
			}
		}
		if (soundDirection != -1) {
			soundSensorFill(soundDirection, sensors);
		}
		if (leastDistance > Tank.MaxSmell) {
			sensors.smell.color = "none";
			sensors.smell.distance = -1;
		}
	}

	/**
	 * Given that a <code>Tank</code> ought to hear a sound (another
	 * <code>Tank</code> moved and was close), sets the specified
	 * <code>TankInputInfo</code> to have <code>sensors.sound</code>
	 * parameters correctly filled with the direction from which the sound has
	 * come.
	 * 
	 * @param direction
	 *            The direction (specified in <code>Tank</code>) from which
	 *            the sound comes.
	 * @param sensors
	 *            The <code>TankInputInfo</code> whose parameters are being
	 *            updated.
	 */
	private void soundSensorFill(int direction, TankInputInfo sensors) {
		switch (direction) {
		case (Tank.FORWARD):
			sensors.sound = "forward";
			break;
		case (Tank.RIGHT):
			sensors.sound = "right";
			break;
		case (Tank.LEFT):
			sensors.sound = "left";
			break;
		case (Tank.BACKWARD):
			sensors.sound = "backward";
			break;
		}
	}

	/**
	 * Iterates over the <code>FlyingMissile</code>s in the simulation,
	 * checking to see if any are flying directly toward the specified
	 * <code>Tank</code>, filling the <code>TankInputInfo</code>
	 * specified's <code>incoming.forward, backward, left,</code> and
	 * <code>right</code> sensors if any missiles are on the same x or y
	 * coordinate the <code>Tank</code> is and flying at the <code>Tank</code>.
	 * 
	 * @param sensors
	 *            The <code>TankInputInfo</code> that will be given the
	 *            <code>Tank</code> when it must make a decision whose sensors
	 *            need to be updated.
	 * @param toFill
	 *            The <code>Tank</code> whose input sensors are represented by
	 *            <code>sensors</code>.
	 */
	public void fillIncomingSensor(TankInputInfo sensors, Tank toFill) {
		Iterator iter = flyingMissiles.iterator();
		while (iter.hasNext()) {
			FlyingMissile fm = (FlyingMissile) iter.next();
			Location loc = fm.getLocation();
			int tankX = toFill.getLocation().getX(), tankY = toFill
					.getLocation().getY();
			int fmX = loc.getX(), fmY = loc.getY();
			int direction = fm.getDirection(), directionOfAttack = -6543;
			if ((fmY == tankY && direction == Tank.WEST && fmX > tankX)
					|| (fmY == tankY && direction == Tank.EAST && fmX < tankX)
					|| (fmX == tankX && direction == Tank.NORTH && fmY > tankY)
					|| (fmX == tankX && direction == Tank.SOUTH && fmY < tankY)) {
				directionOfAttack = toFill.getDirectionFromTank(fmX, fmY);
			}
			switch (directionOfAttack) {
			case (Tank.FORWARD): {
				sensors.incoming.forward = TankInputInfo.yes;
				break;
			}
			case (Tank.BACKWARD): {
				sensors.incoming.backward = TankInputInfo.yes;
				break;
			}
			case (Tank.RIGHT): {
				sensors.incoming.right = TankInputInfo.yes;
				break;
			}
			case (Tank.LEFT): {
				sensors.incoming.left = TankInputInfo.yes;
				break;
			}
			}
		}
	}

	public boolean simShouldStop() {
		return (myWinner != null);
	}

	/**
	 * Fills the provided <code>TankInputInfo</code> with the sensory
	 * information available to the provided <code>Tank</code>--that is, its
	 * radar, sound, smell, and incoming sensors via calls to
	 * <code>fillRwavesSensor(TankInputInfo,
	 * Tank)</code> and
	 * <code>fillIncomingSensor(TankInputInfo, Tank)</code>.
	 * 
	 * @param sensors
	 *            The <code>TankInputInfo</code> representing the information
	 *            available to <code>toFill</code>.
	 * @param toFill
	 *            The <code>Tank</code> whose input-link sensors are being
	 *            filled.
	 */
	public void fillSensors(TankInputInfo sensors, Tank toFill) {
		fillRwavesSensor(sensors, toFill);
		fillIncomingSensor(sensors, toFill);
	}

	/** The <code>Tank</code> that has won this simulation, if any. */
	Tank myWinner = null;

	private boolean soarShouldStop = false;

	/**
	 * Method called by a <code>Tank</code> as soon as it knows it has won.
	 * Stops the simulation by calling <code>stopSimulation</code>.
	 * 
	 * @param winner
	 *            The <code>Tank</code> that has won the simulation.
	 */
	public void tankWon(Tank winner) {
		myWinner = winner;
		this.stopSimulation();
	}

	/**
	 * Stops the simulation that is running, notifying listeners that this is
	 * the case. If a <code>Tank</code> has won the simulation, fires that
	 * notification (that a <code>Tank</code> has won) instead of the
	 * <code>SimulationControl.fireSimEndedNotification().</code>
	 * 
	 * @see edu.umich.JavaBaseEnvironment.SimulationControl#stopSimulation()
	 */
	public void stopSimulation() {
		if (running) {
			if (commandLine.andyMode)
				soarShouldStop = true;
			else {
				running = false;
				singleStep = false;
				if (myWinner != null) {
					fireTankWonNotification(myWinner);
				} else {
					fireSimEndedNotification();
				}
			}
		}
	}

	public SoarAgent createHumanAgent(String color) {
		if (myEcs.colorAvailable(color)) {
			return (createTankColor(color, true));
		}
		return (createTank(true));
	}

	/**
	 * Creates a human-controlled <code>Tank</code> for the simulation.
	 * 
	 * @return The <code>HumanTankControl</code> created if creation was
	 *         successful, <code>null</code> otherwise.
	 */
	public HumanTankControl createHumanTank() {
		return ((HumanTankControl) createTank(true));
	}

	/**
	 * Creates a Tank at random location with random color in the simulation.
	 * 
	 * @return The <code>Tank</code> that has been created, if creation was
	 *         successful, <code>null</code> otherwise.
	 */
	public Tank createTank() {
		String[] colors = myEcs.colorsAvailable();
		if (colors.length > 0) {
			return (createTankColor(colors[0], false));
		}
		return (null);
	}

	/**
	 * This method will try to create the <code>Tank</code> with the given
	 * color. If a file ending with .soar is passed, will place the
	 * <code>Tank</code> randomly, though a .stank file can be passed as well,
	 * configuring the <code>Tank</code>'s starting position, color, and
	 * direction. {@inheritDoc}
	 */
	public SoarAgent loadAgent(File infile, String colorName) {
		TankSoarLogger.log("TankSoarJControl::loadAgent started....");

		try {
			if (infile == null)
				return null;
			String name = infile.getName();
			if (name.toLowerCase().endsWith(".soar")) {
				try {
					Tank t = createTank(colorName);
					if (t != null) {
						t.setKernel(kernel);
						t.attachSoarCode(infile);
					} else
						TankSoarLogger
								.log("\t't' was null after createTank call!");
					return (t);
				}// try
				catch (NullPointerException e) {
					TankSoarLogger
							.log("\tNull ptr while trying to load a new agent..."
									+ e.getMessage());
					return null;
				}

			} else if (name.toLowerCase().endsWith(".stank")) {
				Tank newTank = createTank(infile);
				if (newTank != null) {
					newTank.setKernel(kernel);
					newTank.attachSoarCode(infile);
				}
				return newTank;
			}
			// else
			// TankSoarLogger.log("TankSoarJControl::loadAgent did not get
			// filename ending in stank or soar, returning.");
		} catch (NullPointerException e) {
			TankSoarLogger.log("\tCaught a null ptr exception..."
					+ e.getMessage());
		}
		return (null);
	}

	/**
	 * Creates a <code>Tank</code> using the specified *.stank file, which can
	 * deterimine the <code>Tank</code>'s <code>location</code>,
	 * direction, color, and will deterimine its .soar file.
	 * <p>
	 * A .stank file must have a line specifying the Soar file whose productions
	 * will be used to control the agent. This takes the form
	 * <p>
	 * file <i>file_name</i>.soar
	 * </p>
	 * <p>
	 * It may also include lines specifying location, color, and direction, as
	 * follows:
	 * <ul>
	 * <li>location <i>x-coordinate y-coordinate</i></li>
	 * <li>direction <i>north/east/south/west</i> </li>
	 * <li>color <i>red/green/black...</i></li>
	 * </ul>
	 * 
	 * @param toLoad
	 *            The <code>File</code> to be loaded.
	 * @return The <code>Tank</code> that has been created, if creation was
	 *         successful, <code>null</code> otherwise.
	 */
	public Tank createTank(File toLoad) {
		BufferedReader bIn;
		try {
			bIn = new BufferedReader(new FileReader(toLoad));
		} catch (FileNotFoundException e) {
			return null;
		}
		String line = null, fileToLoad = null, colorName = null;
		int x = -1, y = -1, direction = -1;
		try {
			while ((line = bIn.readLine()) != null) {
				line = line.toLowerCase();
				if (line.startsWith("location")) {
					String[] locData = line.split(" ");
					if (locData.length >= 3) {
						try {
							x = Integer.valueOf(locData[1]).intValue();
							y = Integer.valueOf(locData[2]).intValue();
						} catch (NumberFormatException e) {
							x = y = -1;
						}
					}
				} else if (line.startsWith("file")) {
					try {
						fileToLoad = line.split(" ")[1];
					} catch (ArrayIndexOutOfBoundsException e) {
						return null;
					}
				} else if (line.startsWith("color")) {
					try {
						colorName = line.split(" ")[1];
					} catch (ArrayIndexOutOfBoundsException e) {
						colorName = null;
					}
				} else if (line.startsWith("direction")) {
					try {
						char dir = line.split(" ")[1].charAt(0);
						switch (dir) {
						case ('n'):
							direction = Tank.NORTH;
							break;
						case ('s'):
							direction = Tank.SOUTH;
							break;
						case ('w'):
							direction = Tank.WEST;
							break;
						case ('e'):
							direction = Tank.EAST;
							break;
						}
					} catch (ArrayIndexOutOfBoundsException e) {
						direction = -1;
					}
				}
			}
			bIn.close();
		} catch (IOException e) {
			return null;
		}
		Location createLoc = null;
		if (emptyLocation(x, y)) {
			createLoc = new Location(x, y);
		}
		Tank created = createTank(colorName, createLoc, direction);
		if (created != null) {
			created.setKernel(kernel);
			created.attachSoarCode(new File(fileToLoad));
		}
		return (created);
	}

	/**
	 * Creates a <code>Tank</code> at random location with color specified in
	 * the simulation. If color has been used, random color will be used for
	 * <code>Tank</code>.
	 * 
	 * @param colorName
	 *            The String name of the color to be used to create this
	 *            <code>Tank</code>.
	 * @return The <code>Tank</code> that has been created, if creation was
	 *         successful, <code>null</code> otherwise.
	 */
	public Tank createTank(String colorName) {
		if (myEcs.colorAvailable(colorName)) {
			return (createTankColor(colorName, false));
		}
		return (createTank());
	}

	/**
	 * Creates a <code>Tank</code> at specifed location with random color in
	 * the simulation. If the Location specifed is not empty, <code>Tank</code>
	 * will be placed randomly.
	 * 
	 * @param loc
	 *            The location desired at which the <code>Tank</code> will be
	 *            created.
	 * @return The <code>Tank</code> that has been created, if creation was
	 *         successful, <code>null</code> otherwise.
	 */
	public Tank createTank(Location loc) {
		if (emptyLocation(loc.getX(), loc.getY())) {
			return (createTankLocation(loc, false));
		}
		return (createTank());
	}

	/**
	 * Creates a <code>Tank</code> at specified location with specified color
	 * in the simulation. If the color has already been used by another agent in
	 * the simulation or the <code>
	 * Location</code> specified is not empty,
	 * they will be (respectively) random.
	 * 
	 * @param colorName
	 *            The <code>String</code> name of the color requested.
	 * @param loc
	 *            The <code>Location</code> at which it is desired to create
	 *            the <code>Tank</code>.
	 * @return The <code>Tank</code> that has been created, if creation was
	 *         successful, <code>null</code> otherwise.
	 */
	public Tank createTank(String colorName, Location loc) {
		return (createTank(colorName, loc, (int) (getNextRandomDouble() * 4)));
	}

	/**
	 * Creates a <code>Tank</code> at specified location with specified color
	 * and direction in the simulation. If the color has already been used by
	 * another agent in the simulation or the <code>Location</code> specified
	 * is not empty, they will be (respectively) random.
	 * 
	 * @param colorName
	 *            The <code>String</code> name of the color requested.
	 * @param loc
	 *            The <code>Location</code> at which it is desired to create
	 *            the <code>Tank</code>.
	 * @param direction
	 *            The desired direction (as defined in <code>Tank</code>) of
	 *            the <code>Tank</code> being created. If
	 *            <code>direction < 0</code> will be random in [0, 3]
	 * @return The <code>Tank</code> that has been created, if creation was
	 *         successful, <code>null</code> otherwise.
	 */
	public Tank createTank(String colorName, Location loc, int direction) {
		direction = (direction < 0 ? (int) (getNextRandomDouble() * 4)
				: direction % 4);
		boolean empty = (loc != null ? emptyLocation(loc.getX(), loc.getY())
				: false);
		if (myEcs.colorAvailable(colorName)) {
			if (empty) {
				return (createTankColorLocationDirection(colorName, loc,
						direction, false));
			}
			return (createTankColor(colorName, direction, false));
		} else if (empty) {
			return (createTankLocation(loc, direction, false));
		}
		return (createTank());
	}

	/**
	 * Creates a <code>Tank</code> with the first available color at a random
	 * location in the simulation.
	 * 
	 * @param human
	 *            If <code>true</code>, will create a
	 *            <code>HumanTankControl</code>, a human-controlled
	 *            <code>Tank</code>.
	 * @return The <code>Tank</code> that has been created, if creation was
	 *         successful, <code>null</code> otherwise.
	 */
	private Tank createTank(boolean human) {
		String[] colors = myEcs.colorsAvailable();
		if (colors.length > 0) {
			if (human) {
				return (createTankColor(colors[0], true));
			}
			return (createTankColor(colors[0], false));
		}
		return null;
	}

	/**
	 * Creates a <code>Tank</code> at the specified <code>Location</code> in
	 * the simulation. Since this is a private method, assumes the
	 * <code>Location</code> is available.
	 * 
	 * @param loc
	 *            The <code>Location</code> at which to create the
	 *            <code>Tank</code>.
	 * @param human
	 *            If <code>true</code>, will create a
	 *            <code>HumanTankControl</code>, a human-controlled
	 *            <code>Tank</code>.
	 * @return The <code>Tank</code> that has been created, if creation was
	 *         successful, <code>null</code> otherwise.
	 */
	private Tank createTankLocation(Location loc, boolean human) {
		return (createTankLocation(loc, (int) (getNextRandomDouble() * 4),
				human));
	}

	/**
	 * Creates a <code>Tank</code> at the specified <code>Location</code> in
	 * the simulation. Since this is a private method, assumes the
	 * <code>Location</code> is available and that the direction specified is
	 * valid (in the range [0, 3]).
	 * 
	 * @param loc
	 *            The <code>Location</code> at which to create the
	 *            <code>Tank</code>.
	 * @param direction
	 *            The direction (as defined in <code>Tank</code>) of the
	 *            <code>Tank</code> being created.
	 * @param human
	 *            If <code>true</code>, will create a
	 *            <code>HumanTankControl</code>, a human-controlled
	 *            <code>Tank</code>.
	 * @return The <code>Tank</code> that has been created, if creation was
	 *         successful, <code>null</code> otherwise.
	 */
	private Tank createTankLocation(Location loc, int direction, boolean human) {
		String[] colors = myEcs.colorsAvailable();
		if (colors.length > 0) {
			return (createTankColorLocationDirection(colors[0], loc, direction,
					human));
		}
		return null;
	}

	/**
	 * Creates a <code>Tank</code> of the specified color in the simulation.
	 * Since this is a private method, assumes the color specified is available.
	 * 
	 * @param colorName
	 *            The name of the color to use as the color of the
	 *            <code>Tank</code> being created.
	 * @param human
	 *            If <code>true</code>, will create a
	 *            <code>HumanTankControl</code>, a human-controlled
	 *            <code>Tank</code>.
	 * @return The <code>Tank</code> that has been created, if creation was
	 *         successful, <code>null</code> otherwise.
	 */
	private Tank createTankColor(String colorName, boolean human) {
		Location loc = findEmptyLocation();
		return (createTankColorLocation(colorName, loc, human));
	}

	/**
	 * Creates a <code>Tank</code> of the specified color in the simulation.
	 * Since this is a private method, assumes the color specified is available
	 * and that the direction is valid (i.e. in the range [0, 3]).
	 * 
	 * @param colorName
	 *            The name of the color to use as the color of the
	 *            <code>Tank</code> being created.
	 * @param direction
	 *            The direction (as defined in <code>Tank</code>) of the
	 *            <code>Tank</code> being created.
	 * @param human
	 *            If <code>true</code>, will create a
	 *            <code>HumanTankControl</code>, a human-controlled
	 *            <code>Tank</code>.
	 * @return The <code>Tank</code> that has been created, if creation was
	 *         successful, <code>null</code> otherwise.
	 */
	private Tank createTankColor(String colorName, int direction, boolean human) {
		Location loc = findEmptyLocation();
		return (createTankColorLocationDirection(colorName, loc, direction,
				human));
	}

	/**
	 * Creates a <code>Tank</code> at the specified <code>Location</code>
	 * and with the specified color in the simulation. Since this is a private
	 * method, assumes the color and <code>Location</code> specified are
	 * available.
	 * 
	 * @param colorName
	 *            The name of the color to use as the color of the
	 *            <code>Tank</code> being created.
	 * @param loc
	 *            The <code>Location</code> at which to create the
	 *            <code>Tank</code>.
	 * @param human
	 *            If <code>true</code>, will create a
	 *            <code>HumanTankControl</code>, a human-controlled
	 *            <code>Tank</code>.
	 * @return The <code>Tank</code> that has been created, if creation was
	 *         successful, <code>null</code> otherwise.
	 */
	private Tank createTankColorLocation(String colorName, Location loc,
			boolean human) {
		return (createTankColorLocationDirection(colorName, loc,
				(int) (getNextRandomDouble() * 4), human));
	}

	/**
	 * Creates a <code>Tank</code> at the specified <code>Location</code>
	 * and with the specified color in the simulation. Since this is a private
	 * method, assumes the color and <code>Location</code> specified are
	 * available.
	 * 
	 * @param colorName
	 *            The name of the color to use as the color of the
	 *            <code>Tank</code> being created.
	 * @param loc
	 *            The <code>Location</code> at which to create the
	 *            <code>Tank</code>.
	 * @param direction
	 *            The <code>int</code> value of the direction to create this
	 *            <code>Tank</code> pointing in, as defined in
	 *            <code>Tank</code>.
	 * @param human
	 *            If <code>true</code>, will create a
	 *            <code>HumanTankControl</code>, a human-controlled
	 *            <code>Tank</code>.
	 * @return The <code>Tank</code> that has been created, if creation was
	 *         successful, <code>null</code> otherwise.
	 */
	private Tank createTankColorLocationDirection(String colorName,
			Location loc, int direction, boolean human) {
		Tank t = null;
		if (human) {
			t = new HumanTankControl(colorName, loc, this);
		} else {
			t = new Tank(colorName, loc, this);
		}
		t.setDirection(direction);
		((EnterableSquare) myMap[loc.getX()][loc.getY()]).addOccupant(t);
		myAgents.add(t);
		setOriginalLocations();
		myEcs.useColor(colorName);
		notifyAllTanksAgentChange();
		fireAgentCreatedNotification(t);
		return (t);
	}

	/**
	 * Notifes all the <code>Tank</code>s in the simulation that another
	 * agent has been created or destroyed so that the <code>Tank</code> can
	 * recalculate its sensors (uses <code>Tank.allTanksFinished()</code>).
	 * 
	 * @see Tank#allTanksFinished()
	 */
	private void notifyAllTanksAgentChange() {
		Iterator iter = myAgents.iterator();
		while (iter.hasNext()) {
			((Tank) iter.next()).allTanksFinished();
		}
	}

	/**
	 * Called every time a <code>Tank</code> is created, clears the collection
	 * of original location triples, then fills it again with the
	 * <code>Tank</code>s and their positions and directions when this is
	 * called.
	 */
	private void setOriginalLocations() {
		originalLocations.clear();
		Iterator agentIterator = myAgents.iterator();
		while (agentIterator.hasNext()) {
			SoarAgent a = (SoarAgent) agentIterator.next();
			originalLocations.add(new TankLocationTriple((Tank) a, a
					.getLocation(), ((Tank) a).getDirection()));
		}
	}

	/**
	 * An <code>ArrayList</code> that contains all the <code>Location</code>s
	 * that have been entered during a step of the simulation by a
	 * <code>Tank</code> when there was already a <code>SoarAgent</code>
	 * occupying the <code>Location</code>.
	 * 
	 * @see TankSoarJControl#addCollisionLocation(Location)
	 */
	private ArrayList collidedEnterableLocations = null;

	/**
	 * An <code>ArrayList</code> that contains all the <code>Location</code>s
	 * that <code>
	 * Tank</code>s have exited during a turn, so that in undoing
	 * moves, these <code>
	 * Location</code>s can also be checked.
	 * 
	 * @see TankSoarJControl#addExitedLocation(Location)
	 */
	private ArrayList exitedEnterableLocations = null;

	/**
	 * Parameter that tells whether collisions are being evaluated so calls to
	 * add a collided location can be ignored.
	 */
	private boolean evaluatingCollisions = false;

	/**
	 * Adds a <code>Location</code> to the collection of <code>Location</code>s
	 * that have been exited by <code>Tank</code>s on a given turn. If this
	 * method is called when the simulation is evaluating <code>Tank</code>
	 * collisions, the method returns because it is simply a <code>Tank</code>
	 * performing an undo, and it is unnecessary to keep track of
	 * <code>Location</code>s exited during the undo.
	 * 
	 * @param loc
	 *            The <code>Location</code> exited by a <code>Tank</code>
	 */
	public void addExitedLocation(Location loc) {
		if (exitedEnterableLocations == null)
			exitedEnterableLocations = new ArrayList();
		if (evaluatingCollisions)
			return; /* We don't care because we're already doing undo */
		if (!exitedEnterableLocations.contains(loc)) {
			exitedEnterableLocations.add(loc);
		}
	}

	/**
	 * Adds a <code>Location</code> to the collection of <code>Location</code>s
	 * that have been entered by a <code>Tank</code> when there was already an
	 * agent at the <code>
	 * Location</code> on a given turn. If, when this
	 * method is called, the simulation is evaluating the collisions, this
	 * method is ignored because all <code>Tank</code>s have finished moving
	 * and it is simply an undo operation that causes the collision.
	 * 
	 * @param loc
	 *            The <code>Location</code> that two or more
	 *            <code>SoarAgent</code>s have occupied for some part of
	 *            evaluating a turn in the simulation.
	 */
	public void addCollisionLocation(Location loc) {
		if (collidedEnterableLocations == null)
			collidedEnterableLocations = new ArrayList();
		if (evaluatingCollisions) {
			/* IGNORED because we just don't care--a Tank is performing an undo */
		} else {
			if (!collidedEnterableLocations.contains(loc)) {
				collidedEnterableLocations.add(loc);
			}
		}
	}

	/**
	 * This method checks all of the squares that have had collisions in them,
	 * calling <code>checkCrossovers(Iterator)</code> and
	 * <code>undoMoves(Iterator, boolean)</code> on all the
	 * <code>Location</code>s that have been entered and exited by
	 * <code>Tank</code>s in a given turn. Also clears the containers of
	 * exited and collided <code>Location</code>s.
	 * 
	 * @see TankSoarJControl#undoMoves(Iterator, boolean)
	 * @see TankSoarJControl#checkCrossovers(Iterator)
	 */
	private void evaluateCollisions() {
		evaluatingCollisions = true;
		if (collidedEnterableLocations == null) {
			evaluatingCollisions = false;
			return;
		}
		Iterator iter = collidedEnterableLocations.iterator();
		checkCrossovers(iter);
		iter = collidedEnterableLocations.iterator();
		undoMoves(iter, false);
		if (exitedEnterableLocations == null) {
			evaluatingCollisions = false;
			return;
		}
		iter = exitedEnterableLocations.iterator();
		undoMoves(iter, true);
		collidedEnterableLocations.clear();
		exitedEnterableLocations.clear();
		evaluatingCollisions = false;
	}

	/**
	 * Iterates over the given <code>Location</code>s to check for
	 * collisions, undoing moves if there are collisions--for example, if more
	 * than one <code>Tank</code> resides at the given <code>Location</code>.
	 * 
	 * @param iter
	 *            An <code>Iterator</code> that contains the
	 *            <code>Location</code>s whose contents will be queried to
	 *            check for multiple agents.
	 * @param checkingExits
	 *            <code>true</code> if the function is checking the exited
	 *            <code>Location</code>s, which causes <code>Tank</code>s
	 *            that have already undone moves not to be crashed into again.
	 */
	private void undoMoves(Iterator iter, boolean checkingExits) {
		while (iter.hasNext()) {
			Location loc = (Location) iter.next();
			EnterableSquare square = (EnterableSquare) getLocationContents(loc);
			int numAgents = square.getNumAgents();
			if (numAgents > 1) {
				Object[] occupants = square.getOccupants();
				for (int i = 0; i < occupants.length; ++i) {
					if (occupants[i] instanceof Tank) {
						if (!checkingExits
								|| !((Tank) occupants[i]).getLocationPrevious()
										.equals(
												((Tank) occupants[i])
														.getLocation())) {
							((Tank) occupants[i]).crashed();
						}
						((Tank) occupants[i]).undoLastMove();
					}
				}
			}
		}
	}

	/**
	 * Iterates over the gives <code>Location</code>s to check for
	 * collisions. Checks the contents of each <code>Location</code> for
	 * <code>Tank</code>s, and, if a <code>Tank</code> exists at the
	 * <code>Location</code>, checks the previous position of the
	 * <code>Tank</code> to see if there is a <code>Tank</code> there. If
	 * there is, checks the <code>Tank</code> in the previous position's
	 * previous <code>Location</code>; if it is the same as the first
	 * <code>Tank</code>'s current <code>Location</code>, undoes both
	 * their moves and tells them that they have crashed.
	 * 
	 * @param collidedEnterIter
	 *            An <code>Iterator</code> that contains the
	 *            <code>Location</code>s whose contents will be queried.
	 */
	private void checkCrossovers(Iterator collidedEnterIter) {
		while (collidedEnterIter.hasNext()) {
			Location loc = (Location) collidedEnterIter.next();
			EnterableSquare square = (EnterableSquare) getLocationContents(loc);
			Object[] occupants = square.getOccupants();
			for (int i = 0; i < occupants.length; ++i) {
				if (occupants[i] instanceof Tank) {
					Location previous = ((Tank) occupants[i])
							.getLocationPrevious();
					EnterableSquare previousSquare = (EnterableSquare) getLocationContents(previous);
					if (previousSquare.getNumAgents() > 0) {
						Object[] prevOccupants = previousSquare.getOccupants();
						for (int j = 0; j < prevOccupants.length; ++j) {
							if (prevOccupants[j] instanceof Tank
									&& !prevOccupants[j].equals(occupants[i])
									&& ((Tank) prevOccupants[j])
											.getLocationPrevious().equals(loc)) {
								((Tank) occupants[i]).undoLastMove();
								((Tank) occupants[i]).crashed();
								((Tank) prevOccupants[j]).undoLastMove();
								((Tank) prevOccupants[i]).crashed();
							}
						}
					}
				}
			}
		}
	}

	/**
	 * An <code>ArrayList</code> of <code>Tank</code>s that have been
	 * killed during a step of the simulation.
	 */
	private ArrayList tanksToResurrect = null;

	public Logger logger;

	public argumentContainer commandLine;

	/**
	 * Resurrects all the <code>Tank</code>s in the simulation that need
	 * resurrecting because they've been killed by finding them empty locations
	 * in the simulation and placing them there via
	 * <code>Tank.resurrect(Location)</code>.
	 */
	private void resurrectTanks() {
		if (tanksToResurrect == null)
			return;
		Iterator iter = tanksToResurrect.iterator();
		while (iter.hasNext()) {
			Location loc = findEmptyLocation();
			Tank killed = (Tank) iter.next();
			Location oldLoc = killed.getLocation();
			killed.resurrect(loc);
			fireLocationChangedNotification(loc);
			fireLocationChangedNotification(oldLoc);
		}
		tanksToResurrect.clear();
	}

	/**
	 * Method invoked when a <code>Tank</code> is killed. Resurrects the
	 * <code>Tank</code> at a random <code>Location</code> in the
	 * simulation.
	 * 
	 * @param killed
	 *            The <code>Tank</code> whose health has gone to zero.
	 */
	public void tankKilled(Tank killed) {
		if (tanksToResurrect == null) {
			tanksToResurrect = new ArrayList();
		}
		tanksToResurrect.add(killed);
	}

	/**
	 * Seeds the random number generator so that deterministically random
	 * generation can be used.
	 */
	public void seedRandomNumber() {
		if (myRandom == null)
			myRandom = new Random(314);
		else
			myRandom.setSeed(314);
	}

	/**
	 * Seeds the random number generator so that there is no deterministically
	 * random generation, using <code>Random.setSeed(long)</code> with
	 * <code>System.currentTimeMillis()</code>.
	 */
	public void unseedRandomNumber() {
		if (myRandom == null) {
			myRandom = new Random();
		} else {
			myRandom.setSeed(System.currentTimeMillis());
		}
	}

	/**
	 * Returns the next pseudorandom, uniformly distributed double from 0.0 to
	 * 1.0 in this <code>TankSoarJControl</code>'s random number generator.
	 * If the random number generated has not been seeded, uses a seed based on
	 * the current time.
	 * 
	 * @return A random double greater than or equal to 0.0 but strictly less
	 *         than 1.0.
	 * @see java.util.Random#nextDouble()
	 */
	public double getNextRandomDouble() {
		if (myRandom == null)
			myRandom = new Random();
		return (myRandom.nextDouble());
	}

	/**
	 * Gives caller a random empty location in the simulation's map, random
	 * defined as in <code>getNextRandomDouble()</code>. Note that if all
	 * locations in the map are full, will freeze simulation.
	 * 
	 * @return An empty Location in the map.
	 */
	public Location findEmptyLocation() {
		int x = (int) (getNextRandomDouble() * (MapWidth - 2)) + 1;
		int y = (int) (getNextRandomDouble() * (MapHeight - 2)) + 1;
		while (!emptyLocation(x, y)) {
			x = (int) (getNextRandomDouble() * MapWidth);
			y = (int) (getNextRandomDouble() * MapHeight);
		}
		return (new Location(x, y));
	}

	/**
	 * Tells caller whether the location at the specified coordinates is an
	 * TSEmpty location.
	 * 
	 * @param x
	 *            The x-coordinate of the location being queried.
	 * @param y
	 *            The y-coordinate of the location being queried.
	 * @return true if the location is an TSEmpty, false otherwise.
	 */
	public boolean emptyLocation(int x, int y) {
		try {
			return (myMap[x][y] instanceof TSEmpty
					&& !((TSEmpty) myMap[x][y]).containsAgent() && !((TSEmpty) myMap[x][y])
					.containsMissiles());
		} catch (IndexOutOfBoundsException oops) {

			return (false);
		}
	}

	public void printAgentStats() {
		Iterator iter = myAgents.iterator();
		while (iter.hasNext()) {
			System.out.println(iter.next());
		}
	}

	/**
	 * Notification sent when the location identified by loc has changed (it may
	 * have lost an occupant, etc.).
	 * 
	 * @param loc
	 *            The location that has changed.
	 */
	public void locationChanged(Location loc) {
	}

	/**
	 * Notification sent a SimulationControlListener when the simulation has
	 * begun.
	 */
	public void simStarted() {
		TankSoarLogger.log("TankSoarJControl::simStarted() called....");
	}

	/**
	 * Notification sent a SimulationControl when the simulation ends.
	 * 
	 * @param message
	 *            The message giving the reason the simulation ended.
	 */
	public void simEnded(String message) {
	}

	/**
	 * Notification sent a SimulationControlListener when a SoarAgent is
	 * created.
	 * 
	 * @param created
	 *            As convenience, the tank created is passed with this method.
	 */
	public void agentCreated(SoarAgent created) {
	}

	/**
	 * Notification sent a SimulationControlListener when a SoarAgent is
	 * destroyed.
	 * 
	 * @param destroyed
	 *            As a convenience, the SoarAgent that has been destroyed is
	 *            passed with this method.
	 */
	public void agentDestroyed(SoarAgent destroyed) {
		decisions.remove(destroyed.getName());
	}

	/**
	 * Notification sent an SimulationControlListener when the world count of
	 * the simulation has changed.
	 * 
	 * @param worldCount
	 *            The new world count
	 */
	public void worldCountChanged(int worldCount) {
	}

	/**
	 * Notification sent an SimulationControlListener when the simulation has
	 * quit.
	 */
	public void simQuit() {
		// delete Soar kernel
		kernel.delete();
	}

	/**
	 * Notification sent when a new map is set to be the internal map for the
	 * Simulation.
	 * 
	 * @param message
	 *            An optional message giving information a user may want about a
	 *            map.
	 */
	public void newMap(String message) {
	}

	/**
	 * Method defined to, if the simulation is not running, start the simulation
	 * (in a continuous loop).
	 */
	public void runSimulation() {
		if (running)
			;
		else if (!commandLine.andyMode) {
			TankSoarLogger.log("TankSoarJControl::runSimulation() called....");
			running = true;
			fireSimStartedNotification();
			while (running) {
				// TankSoarLogger.log("runSimulation is going to call
				// 'runAllTanks' right now");
				runAllTanks();
				// startSimulationEvent("TSJControl runSimulation fxn...");
				// singleStep();
			}
			fireSimEndedNotification();
		} else {
			// Run just like Eaters, one decision per world turn
			new Thread(this).start();
		}
	}

	/* ------------ Listener Stuff ------------ */

	/**
	 * Fires a notification that the simulation has ended to all the
	 * <code>SimulationControlListener</code>s, but with the message that a
	 * <code>Tank</code> has won.
	 * 
	 * @see SimulationControlListener#simEnded(String)
	 */
	private void fireTankWonNotification(Tank winner) {
		Iterator iter = simulationControlListeners.iterator();
		while (iter.hasNext()) {
			((SimulationControlListener) iter.next()).simEnded(winner
					.getColorName()
					+ " tank has won.");
		}
	}

	/**
	 * Clears all the flying missiles in the simulation. {@inheritDoc}
	 */
	protected void fireNewMapNotification(String message) {
		flyingMissiles.clear();
		super.fireNewMapNotification(message);
	}

	public void systemEventHandler(int eventID, Object data, Kernel kernel) {
		if (eventID == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue())
			soarStartEvent(eventID, data, kernel);
		else if (eventID == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue())
			soarStopEvent(eventID, data, kernel);
	}

	public void soarStopEvent(int eventID, Object data, Kernel kernel) {
		if (commandLine.andyMode) {
			if (!running) {
				JOptionPane
						.showMessageDialog(
								null,
								"Fatal Error: Soar told us to stop when we think it isn't running.",
								"Eaters", JOptionPane.ERROR_MESSAGE);
				System.exit(-1);
			} else {
				running = false;
				fireSimEndedNotification();
				if (commandLine.autoRun && commandLine.allWindowsOff)
					quitSimulation();
			}
		} else {
			// TankSoarLogger.log("Soar 'stop' event noticed by environment.");
			soarIsRunning = false;
		}
	}

	int tcount = 0;

	private boolean initialRun = true;

	public boolean mapShown = false;

	public void updateEventHandler(int eventID, Object data, Kernel kernel,
			int runFlags) {

		tcount++;

		if (commandLine.andyMode) {
			singleStep();
			if (soarShouldStop) {
				kernel.StopAllAgents();
				soarShouldStop = false;
			}
		}

		TankSoarLogger.log(":" + tcount);
	}

	public void soarStartEvent(int eventID, Object data, Kernel kernel) {
		if (commandLine.andyMode) {
			if (running) {
				JOptionPane
						.showMessageDialog(
								null,
								"Fatal Error: Soar told us to start when we think it's already running.",
								"Eaters", JOptionPane.ERROR_MESSAGE);
				System.exit(-1);
			} else {
				running = true;
				fireSimStartedNotification();
			}
		} else {
			// TankSoarLogger.log("Soar 'start' event noticed by environment.");
			soarIsRunning = true;
		}
	}

	protected void startSimulationEvent(String caller) {
		TankSoarLogger.log("The call to the start event was instigated by "
				+ caller);
		singleStep();
	}

	public void runAllTanks() {
		// FIXME: Singlestep should be two fxns, one for input one for output
		// with the runall inbetween, to handle weird cases
		updateAllTanks();
		TankSoarLogger.log("RunAllTanks called...");
		kernel.RunAllTilOutput();
		singleStep();
		/*
		 * for(Iterator tankItr = myAgents.listIterator();tankItr.hasNext();) {
		 * ((Tank) tankItr.next()).RunTilOutput(); }
		 */
	}

	public void run() {
		kernel.RunAllAgentsForever();
	}

	public boolean isRunnable() {
		// There must be at least one Soar agent and no human ones for it to be
		// runnable.
		if (running)
			return false;
		boolean soar = false;
		Iterator iter = myAgents.iterator();
		while (iter.hasNext()) {
			Tank curr = (Tank) iter.next();
			if (curr instanceof HumanTankControl)
				return false;
			else
				soar = true;
		}
		return soar;
	}

	public boolean isStoppable() {
		return running;
	}

	public boolean isSteppable() {
		return isRunnable();
	}

	public boolean isQuittable() {
		return !running;
	}
}
