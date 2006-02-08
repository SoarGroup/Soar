/* File: EaterControl.java
 * Jul 12, 2004
 */

package edu.umich.eaters;

import edu.umich.JavaBaseEnvironment.*;

import java.io.*;
import java.util.*;

import javax.swing.JOptionPane;

import sml.Agent;
import sml.Kernel;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;

/**
 * This class implements the model underneath the Eaters program.
 * @author jduchi
 * @author Alex Roper
 */
public class EaterControl extends SimulationControl implements
		SimulationControlListener, Runnable, Kernel.SystemEventInterface, Kernel.UpdateEventInterface {

	public static final int Wall = 0;
	public static final int NormalFood = 1;
	public static final int BonusFood = 2;
	public static final int Empty = 3;
	public SoarJavaParser.argumentContainer commandLine = null;
	public static final int EaterMarker = 4;
	public static final int MapWidth = 17;
	public static final int MapHeight = 17;
	public static final int NormalFoodWorth = 5;
	public static final int BonusFoodWorth = 10;
	public static final int JumpCost = 5;

	/* Probabilities for creation of walls in the map. */
	private static final double Probability = .15;
	private static final double BetterProbability = .65;
	private boolean simShouldExit = false;
	private static final int BonusFoodColumns = 3;
	
	private int foodCount = 0;

	private boolean loadedFromFile = false;

	private File loadFile = null;

	private ArrayList collidedEaters = new ArrayList();

	private ArrayList foodCountListeners;

	private Random myRand = new Random();
	
	public Logger logger;

	private Kernel kernel = null;

	boolean soarShouldStop = false;

	private Thread runthread = null;

	/* Constructors */

	/**
	 * Creates a new instance of EaterControl, using the File specified by infile
	 * to create it.
	 * @param infile The file to be used to create a new Eaters map.
	 */
	public EaterControl(File infile) {
		generatePaths();
		initSoar();
		loadMap(infile);
	}

	/**
	 * Creates a new instance of EaterControl, generating a random map to be used.
	 */
	public EaterControl() {
		initSoar();
		generatePaths();
		genMap();
	}
	
	public void generatePaths() {
		// Set up map, agent paths
		String basePath = kernel.GetLibraryLocation() + System.getProperty("file.separator")
		+ ".." + System.getProperty("file.separator") 
		+ "Environments" + System.getProperty("file.separator") 
		+ "JavaEaters" + System.getProperty("file.separator");
		
		agentPath = basePath + "agents";
		mapPath   = basePath + "maps";
	}
	
	public Agent getAgentByColor(String color)
	{
		SoarAgent[] es = getAllAgents();
		
		for (int i = 0;i < es.length;i++)
		{
			if (es[i].getColorName() == color)
			{
				return ((Eater)es[i]).agent;
			}
		}
		
		return null;
	}

	public SoarAgent getSoarAgentByColor(String color)
	{
		SoarAgent[] es = getAllAgents();
		
		for (int i = 0;i < es.length;i++)
		{
			if (es[i].getColorName() == color)
			{
				return es[i];
			}
		}
		
		return null;
	}

	/**
	 * Creates Soar kernel and registers for start, stop, and update events.
	 */
	public void initSoar() {
		kernel = Kernel.CreateKernelInNewThread("SoarKernelSML", 12121);

		// Make sure Soar was created correctly
		if (kernel.HadError()) {
			JOptionPane
					.showMessageDialog(
							null,
							"Fatal error during kernel initialization: kernel.HadError() returned true.",
							"Fatal Error", JOptionPane.ERROR_MESSAGE);
			System.exit(-1);
		}

		int callbackid = 0;
		// Register for Soar start event
		callbackid = kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_START, this, null);
		if (callbackid > 0)
			;//Registration successful
		else {
			JOptionPane
					.showMessageDialog(
							null,
							"Fatal error during kernel initialization: Failed to register for Soar's start event.",
							"Fatal Error", JOptionPane.ERROR_MESSAGE);
			System.exit(-1);
		}

		// Register for Soar stop event
		callbackid = kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_SYSTEM_STOP, this, null);
		if (callbackid > 0)
			;//Registration successful
		else {
			JOptionPane
					.showMessageDialog(
							null,
							"Fatal error during kernel initialization: Failed to register for Soar's stop event.",
							"Fatal Error", JOptionPane.ERROR_MESSAGE);
			System.exit(-1);
		}

		// Register for Soar update event
		callbackid = kernel.RegisterForUpdateEvent(
				smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, null);
		if (callbackid > 0)
			;//Registration successful
		else {
			JOptionPane
					.showMessageDialog(
							null,
							"Fatal error during kernel initialization: Failed to register for Soar's update event",
							"Fatal Error", JOptionPane.ERROR_MESSAGE);
			System.exit(-1);
		}
	}
	
	/**
	 * Method used to find the contents of the specified position in the simulation's internal map.
	 * @param x The x-coordinate of the location being queried.
	 * @param y The y-coordinate of the location being queried.
	 * @return An element of the Simulation. If the x or y
	 * coordinate is outside the indices of the map, returns a <code>Wall</code>.
	 */
	public Object getLocationContents(int x, int y){
		if (0 <= x && x < getMapWidth() && 0 <= y && y < getMapHeight()){
			return(myMap[x][y]);
		}
		return new EatersWall();
	}

	/**
	 * Loads a Eaters map
	 * @param infile file to load
	 */
	public void loadMap(File infile) {
		loadMap(infile, true);
	}

	/**
	 * See EaterControl.loadMap(File infile).
	 * @param infile The file to be loaded.
	 * @param randomPlacement If true, will place eaters randomly, if false, will not.
	 */
	private void loadMap(File infile, boolean randomPlacement) {
		BufferedReader bIn = null;
		try {
			bIn = new BufferedReader(new FileReader(infile));
		} catch (FileNotFoundException f) {
			System.out.println("File not found. Loading random map");
			genMap();
		}
		try {
			EatersSquare empty = new EatersEmpty(0);
			EatersSquare normal = new NormalFood(NormalFoodWorth);
			EatersSquare bonus = new BonusFood(BonusFoodWorth);
			EatersSquare wall = new EatersWall();
			foodCount = 0;
			if (bIn != null) {
				for (int y = 0; y < MapHeight; y++) {
					String s = bIn.readLine();
					if (s.length() != MapWidth) {
						throw new IOException("Map width wrong.");
					} else {
						for (int x = 0; x < s.length(); x++) {
							int type = s.charAt(x) - '0';
							switch (type) {
							case (Empty):
								myMap[x][y] = empty;
								break;
							case (NormalFood):
								myMap[x][y] = normal;
								break;
							case (Wall):
								myMap[x][y] = wall;
								break;
							case (BonusFood):
								myMap[x][y] = bonus;
								break;
							default:
								myMap[x][y] = empty;
								break;
							}
							if (type == NormalFood || type == BonusFood) {
								++foodCount;
							}
						}
					}
				}
			}
		} catch (IOException ioe) {
			System.out.println(ioe.getMessage()
					+ "\nFile improperly formatted. Loading random map");
			genMap();
		} catch (NullPointerException n) {
			System.out.println("File improperly formatted. Loading random map");
			genMap();
		}
		try {
			if (bIn != null) {
				bIn.close();
			}
		} catch (IOException ignored) {
		}
		resetAgents();
		if (randomPlacement) {
			placeAllEaters();
		}
		loadedFromFile = true;
		loadFile = infile;
		fireNewMapNotification(null);
	}

	/**
	 * Generates the map to be used by EaterControl, filling every #BonusFoodColumns
	 * column with bonusfood (or at least the integer), and creating walls randomly
	 * but with probability defined by Probability. No walls are created diagonal to
	 * any others, but walls that will be adjacent to others have probability defined
	 * by BetterProbability of being created.
	 */
	private void genMap() {
		loadedFromFile = false;
		loadFile = null;

		EatersSquare wall = new EatersWall();

		for (int x = 0; x < MapWidth; x++) {
			myMap[x][0] = myMap[x][MapHeight - 1] = wall;
		}
		for (int y = 1; y < MapHeight - 1; y++) {
			myMap[0][y] = myMap[MapWidth - 1][y] = wall;
		}
		
		// Clear old map
		NormalFood normal = new NormalFood(NormalFoodWorth);
		for (int x = 1; x < MapWidth - 1; x++) {
			for (int y = 1; y < MapHeight - 1; y++) {
				myMap[x][y] = normal;
			}
		}
		
		double probability = Probability;
		for (int x = 2; x < MapWidth - 2; x++) {
			for (int y = 2; y < MapHeight - 2; y++) {
				if (!(myMap[x + 1][y + 1] instanceof EatersWall)
						&& !(myMap[x - 1][y - 1] instanceof EatersWall)
						&& !(myMap[x + 1][y - 1] instanceof EatersWall)
						&& !(myMap[x - 1][y + 1] instanceof EatersWall)) {
					if ((myMap[x + 1][y] instanceof EatersWall)
							|| (myMap[x][y + 1] instanceof EatersWall)
							|| (myMap[x - 1][y] instanceof EatersWall)
							|| (myMap[x][y - 1] instanceof EatersWall)) {
						probability = BetterProbability;
					}
					if (myRand.nextDouble() < probability) {
						myMap[x][y] = wall;
					}
					probability = Probability;
				}
			}
		}
		fillFoods();
	}

	private void fillFoods() {
		NormalFood normal = new NormalFood(NormalFoodWorth);
		BonusFood bonus = new BonusFood(BonusFoodWorth);
		foodCount = 0;
		for (int x = 1; x < MapWidth - 1; x++) {
			for (int y = 1; y < MapHeight - 1; y++) {
				if (!(myMap[x][y] instanceof EatersWall)) {
					if (x % BonusFoodColumns == 0) {
						myMap[x][y] = bonus;
					} else {
						myMap[x][y] = normal;
					}
					++foodCount;
				}
			}
		}
	}

	/**
	 * Prints out the map being used internally by EaterControl.
	 * Prints 'W' for wall, 'E' for eater, 'B' for bonusfood, 'N' for normalfood, and
	 * ' ' for empty. '?' serves as an undefined entry in the internal map.
	 * Also prints out existing eaters and their scores.
	 */
	public void printMap() {
		char out;
		for (int y = 0; y < MapHeight; y++) {
			for (int x = 0; x < MapWidth; x++) {
				if (myMap[x][y] instanceof Eater) {
					out = '+';
				} else if (myMap[x][y] instanceof EatersWall) {
					out = 'W';
				} else if (myMap[x][y] instanceof BonusFood) {
					out = 'B';
				} else if (myMap[x][y] instanceof NormalFood) {
					out = 'N';
				} else if (myMap[x][y] instanceof EatersEmpty) {
					out = ' ';
				} else {
					out = '?';
				}
				System.out.print(' ');
				System.out.print(out);
			}
			System.out.print('\n');
		}
		Iterator iter = myAgents.iterator();
		while (iter.hasNext()) {
			Eater curr = (Eater) iter.next();
			System.out.println(curr);
		}
	}

	/**
	 * Returns false
	 * @return false
	 */
	public boolean isQuitting() {
		return false;
	}

	
	public void printAgentStats() {
		Iterator iter = myAgents.iterator();
		while (iter.hasNext()) {
			System.out.println(iter.next());
		}
	}

	/**
	 * This method will try to create the <code>Eater</code> with the given
	 * color. If a file ending with .soar is passed, will place the
	 * <code>Eater</code> randomly, though a .seater file can be passed as well,
	 * configuring the <code>Eater</code>'s starting position, color, and
	 * direction. {@inheritDoc}
	 */
	public SoarAgent loadAgent(File infile, String colorName) {
		try {
			if (infile == null)
				return null;
			String name = infile.getName();
			if (name.toLowerCase().endsWith(".soar")) {
				try {
					Eater t = createEater(colorName);
					if (t != null) {
						t.setKernel(kernel);
						t.attachSoarCode(infile);					
						fireAgentCreatedNotification(t);

					} else {
						JOptionPane.showMessageDialog(null,
								"Fatal Error: Created Eater was null.",
								"Eaters", JOptionPane.ERROR_MESSAGE);
						System.exit(-1);
					}
					spawnDebugger(t.getName());
					return (t);
				}//try
				catch (NullPointerException e) {
					JOptionPane
							.showMessageDialog(
									null,
									"Fatal Error: Caught a null pointer exception while attempting to load agent.",
									"Eaters", JOptionPane.ERROR_MESSAGE);
					System.exit(-1);
					return null;
				}

			} else if (name.toLowerCase().endsWith(".seater")) {

				Eater t = createEater(infile);
				spawnDebugger(t.getName());
				return t;
			}
		} catch (NullPointerException e) {
			JOptionPane
					.showMessageDialog(
							null,
							"Fatal Error: Caught null pointer exception attemtping to load agent.",
							"Eaters", JOptionPane.ERROR_MESSAGE);
			System.exit(-1);
		}
		return (null);
	}

	/* Create only one debugger, the rest will be created by the debugger */
	private static boolean debuggerCreated = false;
	protected void spawnDebugger(String agentName) {
		// Spawn debugger once
		if (debuggerCreated == false) {
			Runtime r = java.lang.Runtime.getRuntime();
			try {
				r.exec("java -jar " + kernel.GetLibraryLocation() 
					+ System.getProperty("file.separator") + "bin"
					+ System.getProperty("file.separator") + "SoarJavaDebugger.jar -remote -agent "
					+ agentName);
			} catch (java.io.IOException e) {
				JOptionPane.showMessageDialog(null, "IOException spawning debugger", "Eaters",
		                JOptionPane.ERROR_MESSAGE);
				System.exit(-1);			
			}
			debuggerCreated = true;
		}		
	}
		
	/**
	 * {@inheritDoc}
	 * Determines what actions this EaterControl needs to take based on a given Eater's decision.
	 * @param decision The integer value corresponding to the decision taken by the Eater, as
	 * defined in class Eater.
	 * @param sa The <code>Eater</code> whose decision we are evaluating.
	 */
	protected void evaluateDecision(Object decision, SoarAgent sa) {
		Eater e = (Eater) sa;
		EaterOutputInfo output = (EaterOutputInfo) decision;
		
		if (commandLine.logVerbosity >= 4 && output.jump != null)
			logger.log(e.getName() + " jumped " + output.jump.direction.charAt(0) + "; location is now "
					+ "(" + e.getLocation().getX() + "," + e.getLocation().getY() + ")\n");
		
		if (commandLine.logVerbosity >= 5 && output.move != null)
			logger.log(e.getName() + " moved " + output.move.direction.charAt(0) + "; location is now "
					+ "(" + e.getLocation().getX() + "," + e.getLocation().getY() + ")\n")
					;
		
		Location newLoc = null;
		if (output.jump != null) {
			switch (output.jump.direction.charAt(0)) {
			case ('w'):
			case ('W'):
				newLoc = sa.getLocation().getAdjacent(Location.West)
						.getAdjacent(Location.West);
				e.myRecentDecision = 3;
				break;
			case ('e'):
			case ('E'):
				newLoc = sa.getLocation().getAdjacent(Location.East)
						.getAdjacent(Location.East);
				e.myRecentDecision = 1;
				break;
			case ('s'):
			case ('S'):
				newLoc = sa.getLocation().getAdjacent(Location.South)
						.getAdjacent(Location.South);
				e.myRecentDecision = 2;
				break;
			case ('n'):
			case ('N'):
				newLoc = sa.getLocation().getAdjacent(Location.North)
						.getAdjacent(Location.North);
				e.myRecentDecision = 0;
				break;
			}
		} else if (output.move != null) {
			switch (output.move.direction.charAt(0)) {
			case ('w'):
			case ('W'):
				newLoc = sa.getLocation().getAdjacent(Location.West);
				e.myRecentDecision = 7;
				break;
			case ('e'):
			case ('E'):
				newLoc = sa.getLocation().getAdjacent(Location.East);
				e.myRecentDecision = 5;
				break;
			case ('s'):
			case ('S'):
				newLoc = sa.getLocation().getAdjacent(Location.South);
				e.myRecentDecision = 6;
				break;
			case ('n'):
			case ('N'):
				newLoc = sa.getLocation().getAdjacent(Location.North);
				e.myRecentDecision = 4;
				break;
			}
		} else {
			newLoc = sa.getLocation();
		}
		Object o = null;
		o = getLocationContents(newLoc.getX(), newLoc.getY());
		if (o instanceof Eater) {
			if (o != sa) {
				/* This avoids the case where the eater makes no decision */
				collidedEaters.add(o);
			}
			setEatersMove(output, (Eater) sa, newLoc, sa.getLocation(),
					(Eater) o);
		} else if (o instanceof EatersSquare && !(o instanceof EatersWall)) {
			setEatersMove(output, (Eater) sa, newLoc, sa.getLocation(),
					(EatersSquare) o);
		}
	}

	/**
	 * Method called when an eater has moved onto a location in the map that does not contain
	 * a wall or eater. Takes care of determining what the eater has landed on and
	 * the actions to take because of that, such as notifying eater to eat NormalFood or
	 * BonusFood.
	 * @param output The <code>EaterOutputInfo</code> that is the decision made by the <code>Eater</code>.
	 * @param eater The eater that is changing.
	 * @param location The location the eater has entered.
	 * @param previousLocation The location the eater is leaving
	 * @param locationPreviousContent The contents of the location the eater is moving to
	 */
	private void setEatersMove(EaterOutputInfo output, Eater eater,
			Location location, Location previousLocation,
			Object locationPreviousContent) {
		if (!collisioning)
			collidedEaters.remove(eater);
		if (output.jump != null)
			eater.jumped();
		if (locationPreviousContent instanceof NormalFood) {
			eater.ateNormalFood();
		} else if (locationPreviousContent instanceof BonusFood) {
			eater.ateBonusFood();
		}
		myMap[location.getX()][location.getY()] = eater;
		fireLocationChangedNotification(location);
		if (previousLocation == null || previousLocation.equals(location)) {
			eater.setLocation(location);
			return;
		}
		if (previousLocation.equals(location))
			return;
		if (getLocationContents(previousLocation) == eater) {
			myMap[previousLocation.getX()][previousLocation.getY()] = new EatersEmpty(
					0);
			/* Otherwise, the previous location had a different eater in it, and we
			 * ignore it. */
		}
		fireLocationChangedNotification(previousLocation);
		eater.setLocation(location);
	}
	
	/**
	 * Removes the specified agent from the simulation permanently, returning its
	 * color to the simulation.
	 * @param destroyed The <code>SoarAgent</code> to permanently remove from the simulation.
	 */
	
	
	protected void removeAgentFromSimulation(SoarAgent destroyed){
		kernel.DestroyAgent(((Eater)destroyed).agent);
		myMap[destroyed.getLocation().getX()][destroyed.getLocation().getY()] = new EatersEmpty();
		fireAgentDestroyedNotification(destroyed);
		myEcs.returnColor(destroyed.getColorName());
		((Eater) destroyed).destroyEater();

		if (myAgents.size() > 0) {
			originalLocs = new OriginalLocation[myAgents.size()];
			for (int i = 0; i < originalLocs.length; i++) {
				originalLocs[i] = new OriginalLocation((Eater) myAgents.get(i));
			}
		} else {
			originalLocs = null;
		}
	}

	private boolean collisioning = false;

	/**
	 * Method invoked if, after all Eaters have finished one step, one or more Eaters share
	 * the same location. Iterates through all the eaters that have collided with one another,
	 * finding the average of their scores and teleporting them to random open locations.
	 */
	private void evaluateCollisions() {
		if (collidedEaters.isEmpty())
			return;
		logger.log("Collisions!\n");
		collisioning = true;
		Iterator collisions = collidedEaters.iterator();
		int count, avg, sum;
		sum = avg = count = 0;
		Eater curr = null;
		while (collisions.hasNext()) {
			curr = (Eater) collisions.next();
			sum += curr.getScore();
			++count;
		}
		Eater collided = (Eater) myMap[curr.getLocation().getX()][curr
				.getLocation().getY()];
		sum += collided.getScore();
		++count;
		avg = sum / count;
		collidedEaters.add(collided);
		collisions = collidedEaters.iterator();
		while (collisions.hasNext()) {
			curr = (Eater) collisions.next();
			curr.setScore(avg);
			Location cLoc = getRandomLocation();
			setEatersMove(new EaterOutputInfo(), curr, cLoc,
					curr.getLocation(), myMap[cLoc.getX()][cLoc.getY()]);
		}
		collisioning = false;
	}

	/**
	 * {@inheritDoc}
	 * In this case, the simulation should stop if there is no food left.
	 */
	public boolean simShouldStop() {
		return (foodCount == 0);
	}

	/**
	 * Tells caller the amount of food that is left in the simulation.
	 * @return Number of pieces of food that have not been eaten.
	 */
	public int foodLeft() {
		return (foodCount);
	}

	/**
	 * Method called (usually by an Eater) to tell EaterControl that a piece of food
	 * has been eaten.
	 * @param type The type of food that was eaten--in case we care later.
	 */
	public void foodEaten(int type) {
		--foodCount;
	}

	/**
	 * Returns to caller the number of steps the simulation has taken.
	 * @return The number of steps the simulation has taken, also known as the
	 * world count.
	 */
	public int getWorldCount() {
		return (myWorldCount);
	}

	public SoarAgent loadAgent(File infile) {
		return (loadAgent(infile, null));
	}

	/**
	 * Creates a <code>Eater</code> using the specified *.seater file, which can deterimine the
	 * <code>eater</code>'s <code>location</code>, color, and .soar file.
	 * <p>A .seater file must have a line specifying the Soar file whose productions will
	 * be used to control the agent. This takes the form <p>file <i>file_name</i>.soar</p>
	 * <p>It may also include lines specifying location and color, as follows:
	 * <ul><li>location <i>x-coordinate y-coordinate</i></li>
	 * <li>color <i>red/green/black...</i></li></ul>
	 * @param toLoad The <code>File</code> to be loaded.
	 * @return The <code>eater</code> that has been created, if creation was successful,
	 * <code>null</code> otherwise.
	 */
	private Eater createEater(File infile) {
		BufferedReader bIn;
		try {
			bIn = new BufferedReader(new FileReader(infile));
		} catch (FileNotFoundException e) {
			return null;
		}
		String line = null, fileToLoad = null, colorName = null;
		int x = -1, y = -1;
		try {
			while ((line = bIn.readLine()) != null) {
				if (line.startsWith("location")) {
					String[] locData = line.split(" ");
					if (locData.length >= 3) {
						try {
							int px = Integer.valueOf(locData[1]).intValue();
							int py = Integer.valueOf(locData[2]).intValue();
							
							if (isOpenLocation(px,py))
							{
								x = px;
								y = py;
							}
						} catch (NumberFormatException e) {
							x = y = -1;
						}
					}
				} else if (line.startsWith("file")) {
					try {
						int spInd = line.indexOf(' ');
						fileToLoad = line.substring(spInd + 1,line.length());
					} catch (ArrayIndexOutOfBoundsException e) {
						return null;
					}
				} else if (line.startsWith("color")) {
					try {
						if (myEcs.colorAvailable(line.split(" ")[1]))
							colorName = line.split(" ")[1];
					} catch (ArrayIndexOutOfBoundsException e) {
						colorName = null;
					}
				}
			}
			bIn.close();
		} catch (IOException e) {
			return null;
		}
				
		Location createLoc = null;
		if (isOpenLocation(x, y)) {
			createLoc = new Location(x, y);
		}
		else {
			String err = "Overrode location in \"" + infile.getAbsolutePath()
					+ "\" with random location.";
			logger.log(err + '\n');
			if (commandLine.seaterErrorLvl > 0) {
				JOptionPane
						.showMessageDialog(
								null,
								err + " and --warn-seater or --error-seater is set.",
								"Seater Error", JOptionPane.ERROR_MESSAGE);
			}
			if (commandLine.seaterErrorLvl == 2)
				System.exit(-1);
		}
		
		if (colorName == null)
		{
			String err = "Overrode color in \"" + infile.getAbsolutePath() + "\" with random color";
			logger.log(err + '\n');
			if (commandLine.seaterErrorLvl > 0)
			{
				JOptionPane.showMessageDialog(null, err
						+ " and --error-seater is set.", "Fatal Error",
						JOptionPane.ERROR_MESSAGE);
			}
			if (commandLine.seaterErrorLvl == 2)
				System.exit(-1);
		}

		Eater created = createEater(colorName, createLoc);
		if (created != null) {
			created.setKernel(kernel);
			created.attachSoarCode(new File(fileToLoad));
		}
		else
		{
			JOptionPane
			.showMessageDialog(
					null,
					"Fatal error during agent creation: createEater failed",
					"Fatal Error", JOptionPane.ERROR_MESSAGE);
			System.exit(-1);
		}
		return (created);
	}

	/**
	 * Creates eater of the color given by colorName and at location specified by
	 * the user. If the location is not open, chooses random location, and if the 
	 * color has been already used, chooses a random color.
	 * @param colorName The name of the color of the new Eater.
	 * @param loc The location desired for the new Eater.
	 * @return The newly created Eater, or null if none created.
	 */
	public Eater createEater(String colorName, Location loc) {
		if (myEcs.colorAvailable(colorName)) {
			if (loc != null && isOpenLocation(loc.getX(), loc.getY())) {
				return createEaterColorLocation(colorName, loc, false);
			} else {
				return createEaterColor(colorName, false);
			}
		} else {
			return createEater();
		}
	}

	/**
	 * Creates an eater of the color given by colorName at a random position on the map.
	 * If the color has already been used, chooses randomly a remaining color.
	 * @param colorName The color to be used for the new eater.
	 * @return The newly created Eater, or null if none created.
	 */
	public Eater createEater(String colorName) {
		if (myEcs.colorAvailable(colorName)) {
			return createEaterColor(colorName, false);
		} else {
			return createEater();
		}
	}

	/**
	 * Method invoked to create an Eater of random color at a random position on the map.
	 * If no more colors are available, no Eater is created.
	 * @return The newly created Eater, or null if none created.
	 */
	public Eater createEater() {
		String color = myEcs.getRandomColorLeft(myRand);
		if (color != null) {
			return createEaterColor(color, false);
		}
		return null;
	}

	public SoarAgent createHumanAgent(String colorName) {
		return (createHumanEater(colorName));
	}

	/**
	 * Method invoked to create a user-controlled Eater of the given color at
	 * a random position on the map. If the color has already been used or does not
	 * exist, chooses a random remaining color for use.
	 * @param colorName The name of the color desired.
	 * @return The newly created HumanEater, or null if none created.
	 */
	public HumanEater createHumanEater(String colorName) {
		if (myEcs.colorAvailable(colorName)) {
			return ((HumanEater) createEaterColor(colorName, true));
		} else {
			return ((HumanEater) createHumanEater());
		}
	}

	/**
	 * Method invoked to create a user-controlled Eater of random color and
	 * at a random location on the map.
	 * @return The newly created HumanEater, or null if none created.
	 */
	public HumanEater createHumanEater() {
		String color = myEcs.getRandomColorLeft(myRand);
		if (color != null) {
			return ((HumanEater) createEaterColor(color, true));
		}
		return (HumanEater) null;
	}

	/**
	 * Gives caller a pseudorandom (as defined in <code>Random</code>) empty
	 * <code>Location</code> in this simulation.
	 * @return A <code>Location</code> that does not contain an <code>Eater</code>
	 * or a <code>Wall</code>.
	 * @see java.util.Random#nextInt(int)
	 */
	private Location getRandomLocation() {
		int x = myRand.nextInt(MapWidth - 2) + 1;
		int y = myRand.nextInt(MapHeight - 2) + 1;
		while (!isOpenLocation(x, y)) {
			x = myRand.nextInt(MapWidth - 2) + 1;
			y = myRand.nextInt(MapHeight - 2) + 1;
		}
		return new Location(x, y);
	}

	/**
	 * Does no checking of colorName. Other than that, same as other creations.
	 * @see EaterControl#createEater()
	 */
	private Eater createEaterColor(String colorName, boolean human) {
		Location loc = getRandomLocation();
		return createEaterColorLocation(colorName, loc, human);
	}

	/**
	 * Assumes <code>loc</code> and <code>colorName</code> are valid and usable.
	 * @see EaterControl#createEater()
	 */
	private Eater createEaterColorLocation(String colorName, Location loc,
			boolean human) {
		Eater created;
		if (human) {
			created = new HumanEater(loc, this, colorName);
		} else {
			created = new Eater(loc, this, colorName);
		}
		Object old = myMap[loc.getX()][loc.getY()];
		myMap[loc.getX()][loc.getY()] = created;
		if (old instanceof BonusFood) {
			created.ateBonusFood();
		} else if (old instanceof NormalFood) {
			created.ateNormalFood();
		}
		myAgents.add(created);
		myEcs.useColor(colorName);

		originalLocs = new OriginalLocation[myAgents.size()];
		for (int i = 0; i < originalLocs.length; i++) {
			originalLocs[i] = new OriginalLocation((Eater) myAgents.get(i));
		}

		fireAgentCreatedNotification(created);
		return (created);
	}

	/**
	 * Returns whether or not an eater can be created at the location given in myMap.
	 * @param x The x-coordinate to be considered
	 * @param y The y-coordinate to be considered
	 * @return <code>true</code> if the position specified does not contain an
	 * <code>Eater</code> or a <code>Wall</code>, <code>false</code> otherwise.
	 */
	private boolean isOpenLocation(int x, int y) {
		Object o = getLocationContents(x, y);
		return (o instanceof EatersEmpty || o instanceof NormalFood || o instanceof BonusFood);
	}

	public void destroyAllAgents() {
		super.destroyAllAgents();
		originalLocs = null;
	}

	/**
	 * The array of original <code>Eater</code> positions in the
	 * simulation.
	 */
	private OriginalLocation[] originalLocs = null;

	/**
	 * Inner class available to keep track of where Eaters are created in the simulation.
	 */
	private class OriginalLocation {
		/** The <code>Eater</code> whose first <code>Location</code> is being stored.*/
		private Eater eater;

		/** The <code>Location</code> of the <code>Eater</code> when created. */
		private Location origin;

		/**
		 * Constructs a new <code>OriginalLocation</code>.
		 * @param e The <code>Eater</code> for which to construct this. Uses this
		 * <code>Eater</code> to find the original <code>Location</code>
		 */
		private OriginalLocation(Eater e) {
			eater = e;
			origin = e.getLocation();
		}

	}

	/**
	 * Resets the map.<p>
	 * Refills the map with whatever food was in it at the outset, and
	 * places any <code>Eaters</code> in the simulation at the same x-y coordinates they
	 * were at when the most recently created <code>Eater</code> was created (or the most
	 * recently destroyed <code>Eater</code> was destroyed). Also resets the world
	 * count of the simulation to 0.
	 */
	public void resetMap() {
		resetAgents();
		boolean randomPlace = false;
		if (originalLocs == null) {
			randomPlace = true;
		}
		if (loadedFromFile && loadFile != null) {
			loadMap(loadFile, randomPlace);
		} else {
			fillFoods();
		}
		for (int i = 0; originalLocs != null && i < originalLocs.length; i++) {
			setEatersMove(new EaterOutputInfo(), originalLocs[i].eater,
					originalLocs[i].origin, null, myMap[originalLocs[i].origin
							.getX()][originalLocs[i].origin.getY()]);
		}
		originalLocs = new OriginalLocation[myAgents.size()];
		for (int i = 0; i < originalLocs.length; i++) {
			originalLocs[i] = new OriginalLocation((Eater) myAgents.get(i));
		}
		myWorldCount = 0;
		fireWorldCountChangedNotification();
		fireNewMapNotification(null);

	}

	/**
	 * Randomly places all the <code>Eater</code>s in the simulation using calls to
	 * <code>getRandomLocation()</code>. Also initializes the original locations
	 * array (<code>originalLocs</code>) to contain the positions and <code>Eater</code>s
	 * being set.
	 */
	private void placeAllEaters() {
		Iterator iter = myAgents.iterator();
		while (iter.hasNext()) {
			Eater e = (Eater) iter.next();
			Location rl = getRandomLocation();
			setEatersMove(new EaterOutputInfo(), e, rl, null,
					myMap[rl.getX()][rl.getY()]);
		}
		originalLocs = new OriginalLocation[myAgents.size()];
		for (int i = 0; i < originalLocs.length; i++) {
			originalLocs[i] = new OriginalLocation((Eater) myAgents.get(i));
		}
	}

	public void newRandomMap() {
		genMap();
		resetAgents();
		placeAllEaters();
		fireNewMapNotification(null);
	}

	/**
	 * Gives the caller an array of the Eaters in the simulation.
	 * Does not directly affect how they are stored, so modification of this
	 * array is allowed, though modification of Eaters should be avoided.
	 * @return An array of all the eaters in the simulation, as defined by
	 * method <code>toArray()<code> in <code>ArrayList<code>.
	 * @see java.util.ArrayList#toArray(java.lang.Object[])
	 */
	public Eater[] getAllEaters() {
		return ((Eater[]) myAgents.toArray(new Eater[0]));
	}

	/**
	 * Notifies listeners that the amount of food that the food for the simulation is empty.
	 * Simply calls <code>fireSimEndedNotification("No more food in simulation")</code>.
	 * @see SimulationControl#fireSimEndedNotification(String)
	 */
	private void fireFoodEmptyNotification() {
		fireSimEndedNotification("No more food in simulation");
	}

	/* Listener stuff */

	/**
	 * Notification sent when the location identified by loc has changed
	 * (it may have lost an occupant, etc.).
	 * @param loc The location that has changed.
	 */
	public void locationChanged(Location loc) {

	}

	/**
	 * Notification sent a SimulationControlListener when the simulation has begun.
	 */
	public void simStarted() {

	}

	/**
	 * Notification sent a SimulationControl when the simulation ends.
	 * @param message The message giving the reason the simulation ended.
	 */
	public void simEnded(String message) {

	}

	/**
	 * Notification sent a SimulationControlListener when a SoarAgent is created.
	 * @param created As convenience, the eater created is passed with this method.
	 */
	public void agentCreated(SoarAgent created) {

	}

	/**
	 * Notification sent a SimulationControlListener when a SoarAgent is destroyed.
	 * @param destroyed As a convenience, the SoarAgent that has been destroyed is passed
	 * with this method.
	 */
	public void agentDestroyed(SoarAgent destroyed) {
		
	}

	/**
	 * Notification sent an SimulationControlListener when the world count of
	 * the simulation has changed.
	 * @param worldCount The new world count
	 */
	public void worldCountChanged(int worldCount) {

	}

	/**
	 * Notification sent an SimulationControlListener when the simulation has quit.
	 */
	public void simQuit() {

	}

	/**
	 * Notification sent when a new map is set to be the internal map for the Simulation.
	 * @param message An optional message giving information a user may want about a map.
	 */
	public void newMap(String message) {

	}

	/**
	 * Method defined to, if the simulation is not running, start the simulation.
	 */
	public void runSimulation() {
		new Thread(this).start();
	}

	/**
	 * Run Soar and simulation for a single step
	 */
	public void singleStep() {
//		singleStep = true;
		kernel.RunAllAgents(1);
//		singleStep = false;
	}

	/**
	 * Run Soar and simulation until told to stop.
	 */
	public void run() {
		kernel.RunAllAgentsForever();
	}

	public void systemEventHandler(int eventID, Object data, Kernel kernel)
	{
		if (eventID == smlSystemEventId.smlEVENT_SYSTEM_START.swigValue())
			soarStartEvent(eventID, data, kernel) ;
		else if (eventID == smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue())
			soarStopEvent(eventID, data, kernel) ;
	}

	public void soarStopEvent(int eventID, Object data, Kernel kernel) {
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
	}

	public void soarStartEvent(int eventID, Object data, Kernel kernel) {
		if (running) {
			JOptionPane
					.showMessageDialog(
							null,
							"Fatal Error: Soar told us to start when we think it's already running.",
							"Eaters", JOptionPane.ERROR_MESSAGE);
			System.exit(-1);
		} else {
			//Ensure input link is up to date in case of first run
			Iterator iter = myAgents.iterator();
			while (iter.hasNext()) {
				Eater curr = ((Eater) iter.next());
				curr.updateSensors();
			}
			
			running = true;
			fireSimStartedNotification();
		}
	}

	public void updateEventHandler(int eventID, Object data, Kernel kernel,
			int runFlags) {
		if (!running) {
			JOptionPane
					.showMessageDialog(
							null,
							"Fatal Error: Soar told us to update world state when we think it isn't running.",
							"Eaters", JOptionPane.ERROR_MESSAGE);
			System.exit(-1);
		} else {
			if (soarShouldStop || simShouldStop()) {
				soarShouldStop = false;
				kernel.StopAllAgents();
				if (simShouldStop())
				{
					this.fireSimEndedNotification("All the food had been successfully eaten!");
					logger.log("\n---Final Scores---\n");
					
					SoarAgent[] es = getAllAgents();
					
					for (int i = 0;i < es.length;i++)
					{
						logger.log(((Eater)es[i]).getName() + ": " + ((Eater)es[i]).getScore() + '\n');
					}
					
					logger.log("\n\n*****SIMULATION FINISHED*****\n\n\n");
					 
					if (commandLine.autoRun)
					{
						simShouldExit = true;
						this.stopSimulation();
					}
				}
			} else {
				Object decisions[] = new Object[myAgents.size()];
				Iterator iter = myAgents.iterator();
				int count = 0;
				while (iter.hasNext()) {
					SoarAgent curr = ((SoarAgent) iter.next());
					decisions[count++] = curr.makeDecision();
				}
				iter = myAgents.iterator();
				count = 0;
				while (iter.hasNext()) {
					evaluateDecision(decisions[count++], (SoarAgent) iter
							.next());
				}

				iter = myAgents.iterator();
				while (iter.hasNext()) {
					((Eater) iter.next()).updateSensors();
				}

				++myWorldCount;
				evaluateCollisions();
				collidedEaters.clear();
				fireWorldCountChangedNotification();
			}
		}
	}

	/**
	 * If the simulation is running, will stop the simulation after all the agents
	 * have finished the current step.
	 * Implementation is simply as follows:<p>
	 * <code>running = false; singleStep = false; fireSimEndedNotification();</code>
	 */
	public void stopSimulation() {
		soarShouldStop = true;
		runthread = null;
	}
	
	/**
	 * @return whether simulation ready to singleStep()
	 */
	public boolean isSteppable()
	{/*
		Iterator iter = myAgents.iterator();
		while (iter.hasNext()) {
			Eater curr = (Eater) iter.next();
			if (!(curr instanceof HumanEater))
				return true;
		}
		return false;*/
		return isRunnable();
	}
	
	/**
	 * @return whether simulation ready to run
	 */
	public boolean isRunnable()
	{
		//There must be at least one Soar agent and no human ones for it to be runnable.
		if (running)
			return false;
		boolean soar = false;
		Iterator iter = myAgents.iterator();
		while (iter.hasNext()) {
			Eater curr = (Eater) iter.next();
			if (curr instanceof HumanEater)
				return false;
			else
				soar = true;
		}
		return soar;
	}
	
	/**
	 * @return whether simulation may stop
	 */
	public boolean isStoppable()
	{
		//Must be running
		return running;
	}
	
	/**
	 * @return whether simulation may quit
	 */
	public boolean isQuittable()
	{
		return !running;
	}
	
	/**
	 * Called by control panel when user had decided to quit for certain.
	 * To us it means destroy Soar kernel and exit 
	 */
	public void quitSimulation()
	{
		if (isQuittable())
		{
			kernel.Shutdown();
		}
		else {
			JOptionPane
					.showMessageDialog(
							null,
							"Fatal Error: Attempted to exit with simulation still running.",
							"Eaters", JOptionPane.ERROR_MESSAGE);
			System.exit(-1);	
		}
		
		super.quitSimulation();
	}
}