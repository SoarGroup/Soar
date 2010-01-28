package edu.umich.soar.gridmap2d.visuals;

import org.apache.log4j.Logger;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ControlAdapter;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.DisposeEvent;
import org.eclipse.swt.events.DisposeListener;
import org.eclipse.swt.events.KeyAdapter;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.MouseAdapter;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.events.ShellAdapter;
import org.eclipse.swt.events.ShellEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Dialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.MenuItem;
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.swt.widgets.Shell;

import edu.umich.soar.gridmap2d.CognitiveArchitecture;
import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Game;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.map.EatersMap;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.world.World;


public class WindowManager {
	private static Logger logger = Logger.getLogger(WindowManager.class);

	public static Color white = null;
	public static Color widget_background = null;
	public static Color blue = null;
	public static Color red = null;
	public static Color yellow = null;
	public static Color orange = null;
	public static Color black = null;
	public static Color green = null;
	public static Color purple = null;
	public static Color brown = null;
	public static Color lightGray = null;
	public static Color darkGray = null;

	static Display display;
	protected Shell shell;
	Label scoreCount;
	Label foodCount;
	Label worldCount;
	SimulationButtons simButtons;
	MapButtons mapButtons;
	VisualWorld visualWorld;
	AgentDisplay agentDisplay;
	Group worldGroup;
	Label statusLine;
	String popUpTitle;
	String popUpMessage;
	String statusMessage;
	CommandInfo humanMove;
	Player human;
	Composite rhs;
	Composite currentSide;
	Menu menuBar;
	Menu fileMenu;
	Menu mapMenu;
	Menu helpMenu;
	
	MenuItem fileMenuHeader;
	MenuItem mapMenuHeader;
	MenuItem helpMenuHeader;
	
	//MenuItem fileConfigurationItem;
	MenuItem fileExitItem;
	MenuItem mapChangeItem;
	MenuItem helpAboutItem;
	World world;

	private CognitiveArchitecture cogArch;
	

	public static final int kEatersMainMapCellSize = 20;
	public static final int kTaxiMainMapCellSize = 20;
	public static final int kTanksoarMainMapCellSize = 32;
	public static final String kFoodRemaining = "Food remaining: ";
	public static final String kScoreRemaining = "Points remaining: ";
	public static final String kWorldCount = "World count: ";

	private static void initColors(Display d) {
	    white = d.getSystemColor(SWT.COLOR_WHITE);
		widget_background = d.getSystemColor(SWT.COLOR_WIDGET_BACKGROUND);
		blue = d.getSystemColor(SWT.COLOR_BLUE);
		red = d.getSystemColor(SWT.COLOR_RED);
		yellow = d.getSystemColor(SWT.COLOR_YELLOW);
		green = d.getSystemColor(SWT.COLOR_GREEN);
		purple = d.getSystemColor(SWT.COLOR_DARK_MAGENTA);
		orange = new Color(d, 255, 127, 0);
		black = d.getSystemColor(SWT.COLOR_BLACK);
		brown = new Color(d, 128, 64, 0);
		lightGray = new Color(d, 170, 170, 170);
		darkGray = new Color(d, 100, 100, 100);
	}
	
	public static Color getColor(String color) {
		if (color == null) {
			return null;
		}
		if (color.equalsIgnoreCase("white")) {
			return white;
		}
		if (color.equalsIgnoreCase("blue")) {
			return blue;
		}
		if (color.equalsIgnoreCase("red")) {
			return red;
		}
		if (color.equalsIgnoreCase("yellow")) {
			return yellow;
		}
		if (color.equalsIgnoreCase("green")) {
			return green;
		}
		if (color.equalsIgnoreCase("purple")) {
			return purple;
		}
		if (color.equalsIgnoreCase("orange")) {
			return orange;
		}
		if (color.equalsIgnoreCase("black")) {
			return black;
		}
		if (color.equalsIgnoreCase("brown")) {
			return brown;
		}
		if (color.equalsIgnoreCase("lightGray")) {
			return lightGray;
		}
		if (color.equalsIgnoreCase("darkGray")) {
			return darkGray;
		}
		return null;
	}

	public boolean initialize() {
		// can be called multiple times
		if (display != null) {
			return true;
		}

		try {
			display = new Display();
		} catch (java.lang.UnsatisfiedLinkError e) {
			logger.info("Failed to create display (java.lang.UnsatisfiedLinkError), this is normal on systems that do not have a window manager.");
			return false;
		} catch (org.eclipse.swt.SWTError e) {
			logger.info("Failed to create display (org.eclipse.swt.SWTError), this is expected if there is no window manager available.");
			return false;
		}
		assert display != null;

		shell = new Shell(display, SWT.BORDER | SWT.CLOSE | SWT.MIN | SWT.TITLE);
		if (shell == null) {
			display.dispose();
			display = null;
			return false;		}
		initColors(display);
		
		shell.addDisposeListener(new DisposeListener() {
			public void widgetDisposed(DisposeEvent arg0) {
				// save window position
				if (shell != null) {
					int [] xy = new int[2];
					xy[0] = shell.getLocation().x;
					xy[1] = shell.getLocation().y;
					if (Gridmap2D.config != null) {
						Gridmap2D.config.saveWindowPosition(xy);
					}
				}
			}
		});
		return true;
	}
	
	public boolean using() {
		if (Gridmap2D.config == null) {
			return false;
		}
		return !Gridmap2D.config.generalConfig().headless;
	}
	
	public void setupEaters() {
		worldGroup = new Group(shell, SWT.NONE);
		worldGroup.setLayout(new FillLayout());
		visualWorld = new EatersVisualWorld(worldGroup, SWT.NONE, kEatersMainMapCellSize);
		visualWorld.setMap(world.getMap());

		visualWorld.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				Player player = visualWorld.getPlayerAtPixel(new int [] { e.x, e.y });
				if (player == null) {
					return;
				}
				agentDisplay.selectPlayer(player);
			}
		});
		visualWorld.addKeyListener(new KeyAdapter() {
			public void keyPressed(KeyEvent e) {
				if (humanMove == null) {
					return;
				}
				boolean go = false;
				switch (e.keyCode) {
				case SWT.KEYPAD_8:
					humanMove.move = true;
					humanMove.moveDirection = Direction.NORTH;
					go = true;
					break;
				case SWT.KEYPAD_6:
					humanMove.move = true;
					humanMove.moveDirection = Direction.EAST;
					go = true;
					break;
				case SWT.KEYPAD_2:
					humanMove.move = true;
					humanMove.moveDirection = Direction.SOUTH;
					go = true;
					break;
				case SWT.KEYPAD_4:
					humanMove.move = true;
					humanMove.moveDirection = Direction.WEST;
					go = true;
					break;
				case SWT.KEYPAD_5:
					humanMove.jump = false;
					go = true;
					break;
				case SWT.KEYPAD_0:
					humanMove.jump = !humanMove.jump;
					break;
				case SWT.KEYPAD_DECIMAL:
					humanMove.dontEat = !humanMove.dontEat;
					break;
				case SWT.KEYPAD_ADD:
					humanMove.open = !humanMove.open;
					break;
				case SWT.KEYPAD_MULTIPLY:
					humanMove.stopSim = !humanMove.stopSim;
					break;
				default:
					break;
				}
				
				Gridmap2D.wm.setStatus(human.getColor() + ": " + humanMove.toString(), black);
				
				if (go) {
					synchronized(humanMove) {
						humanMove.notify();
					}
				}
			}
		});
		
		createRHS();
		createEatersSide();

		shell.setText("Eaters");
	}
	
	public void setupTaxi() {
		worldGroup = new Group(shell, SWT.NONE);
		worldGroup.setLayout(new FillLayout());
		visualWorld = new TaxiVisualWorld(worldGroup, SWT.NONE, kTaxiMainMapCellSize);
		visualWorld.setMap(world.getMap());

		visualWorld.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				Player player = visualWorld.getPlayerAtPixel(new int [] { e.x, e.y });
				if (player == null) {
					return;
				}
				agentDisplay.selectPlayer(player);
			}
		});
		visualWorld.addKeyListener(new KeyAdapter() {
			public void keyPressed(KeyEvent e) {
				if (humanMove == null) {
					return;
				}
				switch (e.keyCode) {
				case SWT.KEYPAD_8:
					humanMove.move = true;
					humanMove.moveDirection = Direction.NORTH;
					break;
				case SWT.KEYPAD_6:
					humanMove.move = true;
					humanMove.moveDirection = Direction.EAST;
					break;
				case SWT.KEYPAD_2:
					humanMove.move = true;
					humanMove.moveDirection = Direction.SOUTH;
					break;
				case SWT.KEYPAD_4:
					humanMove.move = true;
					humanMove.moveDirection = Direction.WEST;
					break;
				case SWT.KEYPAD_5:
					humanMove.fillup = true;
					break;
				case SWT.KEYPAD_1:
					humanMove.pickup = true;
					break;
				case SWT.KEYPAD_3:
					humanMove.putdown = true;
					break;

				case SWT.KEYPAD_MULTIPLY:
					humanMove.stopSim = !humanMove.stopSim;
					break;
				default:
					break;
				}
				
				Gridmap2D.wm.setStatus(human.getColor() + ": " + humanMove.toString(), black);
				
				synchronized(humanMove) {
					humanMove.notify();
				}
			}
		});
		
		createRHS();
		createTaxiSide();

		shell.setText("Taxi");
	}
	
	private void createRHS() {
		rhs = new Composite(shell, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			rhs.setLayout(gl);

			GridData gd = new GridData();
			rhs.setLayoutData(gd);
		}
	}
	
	private void createEatersSide() {
		
		currentSide = new Composite(rhs, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			currentSide.setLayout(gl);
			
			GridData gd = new GridData();
			currentSide.setLayoutData(gd);
		}
		
		Group group1 = new Group(currentSide, SWT.NONE);
		{
			GridData gd = new GridData();
			group1.setLayoutData(gd);
		}
		group1.setText("Simulation");
		group1.setLayout(new FillLayout());
		simButtons = new SimulationButtons(group1, world.numberOfPlayers());
		
		Group group2 = new Group(currentSide, SWT.NONE);
		{
			GridData gd = new GridData();
			group2.setLayoutData(gd);
		}
		group2.setText("Map");
		{
			GridLayout gl = new GridLayout();
			gl.numColumns = 2;
			group2.setLayout(gl);
		}
		
		Label foodLabel = new Label(group2, SWT.NONE);
		{
			GridData gd = new GridData();
			foodLabel.setLayoutData(gd);
		}
		foodLabel.setText(kFoodRemaining);
		
		foodCount = new Label(group2, SWT.NONE);
		{ 
			GridData gd = new GridData();
			gd.widthHint = 50;
			foodCount.setLayoutData(gd);
		}
		
		Label scoreLabel = new Label(group2, SWT.NONE);
		{
			GridData gd = new GridData();
			scoreLabel.setLayoutData(gd);
		}
		scoreLabel.setText(kScoreRemaining);
		
		scoreCount = new Label(group2, SWT.NONE);
		{ 
			GridData gd = new GridData();
			gd.widthHint = 50;
			scoreCount.setLayoutData(gd);
		}
		
		Label worldCountLabel = new Label(group2, SWT.NONE);
		worldCountLabel.setText(kWorldCount);
		{
			GridData gd = new GridData();
			worldCountLabel.setLayoutData(gd);
		}
		
		worldCount = new Label(group2, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.widthHint = 50;
			worldCount.setLayoutData(gd);
		}
		
		updateCounts();
		
		mapButtons = new MapButtons(group2);
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			mapButtons.setLayoutData(gd);
		}

		agentDisplay = new EatersAgentDisplay(currentSide, world, cogArch);
		{
			GridData gd = new GridData();
			agentDisplay.setLayoutData(gd);
		}
	}
	
	private void createTaxiSide() {
		
		currentSide = new Composite(rhs, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			currentSide.setLayout(gl);
			
			GridData gd = new GridData();
			currentSide.setLayoutData(gd);
		}
		
		Group group1 = new Group(currentSide, SWT.NONE);
		{
			GridData gd = new GridData();
			group1.setLayoutData(gd);
		}
		group1.setText("Simulation");
		group1.setLayout(new FillLayout());
		simButtons = new SimulationButtons(group1, world.numberOfPlayers());
		
		Group group2 = new Group(currentSide, SWT.NONE);
		{
			GridData gd = new GridData();
			group2.setLayoutData(gd);
		}
		group2.setText("Map");
		{
			GridLayout gl = new GridLayout();
			gl.numColumns = 2;
			group2.setLayout(gl);
		}
		
		Label worldCountLabel = new Label(group2, SWT.NONE);
		worldCountLabel.setText(kWorldCount);
		{
			GridData gd = new GridData();
			worldCountLabel.setLayoutData(gd);
		}
		
		worldCount = new Label(group2, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.widthHint = 50;
			worldCount.setLayoutData(gd);
		}
		
		updateCounts();
		
		mapButtons = new MapButtons(group2);
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			mapButtons.setLayoutData(gd);
		}

		agentDisplay = new TaxiAgentDisplay(currentSide, world, cogArch);
		{
			GridData gd = new GridData();
			agentDisplay.setLayoutData(gd);
		}
	}
		
	public class GetIdDialog extends Dialog {
		public GetIdDialog(Shell parent) {
			super(parent);
		}
		
		public void open() {
			Shell parent = getParent();
			final Shell dialog = new Shell(parent, SWT.DIALOG_TRIM | SWT.APPLICATION_MODAL);
			dialog.setText("Get ID");
			
			GridLayout gl = new GridLayout();
			gl.numColumns = 2;
			dialog.setLayout(gl);

			final Label label2 = new Label(dialog, SWT.NONE);
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.BEGINNING;
				label2.setLayoutData(gd);
			}
			label2.setText("Id:");
			
			final Button ok = new Button(dialog, SWT.PUSH);
			{
				GridData gd = new GridData();
				gd.horizontalAlignment = GridData.BEGINNING;
				ok.setLayoutData(gd);
			}
			ok.setText("Ok");
			ok.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					dialog.dispose();
				}
			});

			dialog.setSize(dialog.computeSize(SWT.DEFAULT, SWT.DEFAULT, false));
			
			dialog.open();
			Display display = parent.getDisplay();
			while (!dialog.isDisposed()) {
				if (!display.readAndDispatch()) {
					display.sleep();
				}
			}
		}
	}
		
	public void setupTankSoar() {
		worldGroup = new Group(shell, SWT.NONE);
		worldGroup.setLayout(new FillLayout());
		visualWorld = new TankSoarVisualWorld(worldGroup, SWT.NONE, kTanksoarMainMapCellSize);
		visualWorld.setMap(world.getMap());

		visualWorld.addKeyListener(new KeyAdapter() {
			public void keyPressed(KeyEvent e) {
				if (humanMove == null) {
					return;
				}
				boolean go = false;
				Direction facing = human.getFacing();
				switch (e.keyCode) {
				case SWT.KEYPAD_8:
					humanMove.move = true;
					humanMove.moveDirection = facing;
					break;
				case SWT.KEYPAD_6:
					humanMove.move = true;
					humanMove.moveDirection = facing.right();
					break;
				case SWT.KEYPAD_2:
					humanMove.move = true;
					humanMove.moveDirection = facing.backward();
					break;
				case SWT.KEYPAD_4:
					humanMove.move = true;
					humanMove.moveDirection = facing.left();
					break;
				case SWT.KEYPAD_1:
					humanMove.rotate = !humanMove.rotate;
					humanMove.rotateDirection = Names.kRotateLeft;
					break;
				case SWT.KEYPAD_3:
					humanMove.rotate = !humanMove.rotate;
					humanMove.rotateDirection = Names.kRotateRight;
					break;
				case SWT.KEYPAD_5:
					humanMove.move = false;
					break;
				case SWT.KEYPAD_0:
					humanMove.fire = !humanMove.fire;
					break;
				case SWT.KEYPAD_7:
					humanMove.shields = !humanMove.shields;
					humanMove.shieldsSetting = true;
					break;
				case SWT.KEYPAD_9:
					humanMove.shields = !humanMove.shields;
					humanMove.shieldsSetting = false;
					break;
				case SWT.KEYPAD_MULTIPLY:
					humanMove.stopSim = !humanMove.stopSim;
					break;
				case SWT.KEYPAD_ADD:
					humanMove.radar = true;
					humanMove.radarSwitch = true;
					humanMove.radarPower = true;
					if (humanMove.radarPowerSetting <= 0) {
						humanMove.radarPowerSetting = 1;
					} else {
						humanMove.radarPowerSetting += 1;
					}
					break;
				case SWT.KEYPAD_SUBTRACT:
					humanMove.radar = true;
					humanMove.radarPower = true;
					humanMove.radarPowerSetting -= 1;
					if (humanMove.radarPowerSetting <= 0) {
						humanMove.radarPowerSetting = 0;
						humanMove.radarSwitch = false;
					} else {
						humanMove.radarSwitch = true;
					}
					break;
				case SWT.KEYPAD_CR:
					go = true;
					break;
				default:
					break;
				}
				
				Gridmap2D.wm.setStatus(human.getColor() + ": " + humanMove.toString(), black);
				
				if (go) {
					synchronized(humanMove) {
						humanMove.notify();
					}
				}
			}
		});
		
		createRHS();
		createTankSoarSide();

		shell.setText("TankSoar");
	}
	
	private void createTankSoarSide() {
		currentSide = new Composite(rhs, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			gl.numColumns = 2;
			currentSide.setLayout(gl);
			
			GridData gd = new GridData();
			currentSide.setLayoutData(gd);
		}
		
		Group group1 = new Group(currentSide, SWT.NONE);
		{
			GridData gd = new GridData();
			group1.setLayoutData(gd);
		}
		group1.setText("Simulation");
		group1.setLayout(new FillLayout());
		simButtons = new SimulationButtons(group1, world.numberOfPlayers());
		
		Group group2 = new Group(currentSide, SWT.NONE);
		{
			GridData gd = new GridData();
			group2.setLayoutData(gd);
		}
		group2.setText("Map");
		group2.setLayout(new FillLayout());
		mapButtons = new MapButtons(group2);

		Composite comp1 = new Composite(currentSide, SWT.NONE);
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			comp1.setLayoutData(gd);
		}
		{
			GridLayout gl = new GridLayout();
			gl.numColumns = 2;
			comp1.setLayout(gl);
		}

		Label worldCountLabel = new Label(comp1, SWT.NONE);
		worldCountLabel.setText(kWorldCount);
		{
			GridData gd = new GridData();
			worldCountLabel.setLayoutData(gd);
		}
		
		worldCount = new Label(comp1, SWT.NONE);
		{ 
			GridData gd = new GridData();
			gd.widthHint = 50;
			worldCount.setLayoutData(gd);
		}
		
		updateCounts();
		
		agentDisplay = new TankSoarAgentDisplay(currentSide, world, cogArch);
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			agentDisplay.setLayoutData(gd);
		}
	}
	
	public void run(World world) {
		this.world = world;
		
		menuBar = new Menu(shell, SWT.BAR);
		fileMenuHeader = new MenuItem(menuBar, SWT.CASCADE);
		fileMenuHeader.setText("&File");
		
		fileMenu = new Menu(shell, SWT.DROP_DOWN);
		fileMenuHeader.setMenu(fileMenu);
		
		fileExitItem = new MenuItem(fileMenu, SWT.PUSH);
		fileExitItem.setText("&Exit");
		fileExitItem.addSelectionListener(new SelectionListener() {
		    public void widgetSelected(SelectionEvent event) {
		        shell.close();
		        display.dispose();
		    }

		    public void widgetDefaultSelected(SelectionEvent event) {
		        shell.close();
		        display.dispose();
		    }
		});
		
		mapMenuHeader = new MenuItem(menuBar, SWT.CASCADE);
		mapMenuHeader.setText("&Map");
		
		mapMenu = new Menu(shell, SWT.DROP_DOWN);
		mapMenuHeader.setMenu(mapMenu);
		
		mapChangeItem = new MenuItem(mapMenu, SWT.PUSH);
		mapChangeItem.setText("&Change Map");
		mapChangeItem.addSelectionListener(new SelectionListener() {
		    public void widgetSelected(SelectionEvent event) {
		    	mapButtons.changeMap();
		    }

		    public void widgetDefaultSelected(SelectionEvent event) {
		    	mapButtons.changeMap();
		    }
		});

		helpMenuHeader = new MenuItem(menuBar, SWT.CASCADE);
		helpMenuHeader.setText("&Help");

		helpMenu = new Menu(shell, SWT.DROP_DOWN);
		helpMenuHeader.setMenu(helpMenu);
		
		helpAboutItem = new MenuItem(helpMenu, SWT.PUSH);
		helpAboutItem.setText("&About");
		helpAboutItem.addSelectionListener(new SelectionListener() {
		    public void widgetSelected(SelectionEvent event) {
		        infoMessage("About", "Soar2D\nby Jonathan Voigt\nvoigtjr@gmail.com");
			}
			
			public void widgetDefaultSelected(SelectionEvent event) {
				infoMessage("About", "Soar2D\nby Jonathan Voigt\nvoigtjr@gmail.com");
			}
		});
		
		shell.setMenuBar(menuBar);
		
		GridLayout gl = new GridLayout();
		gl.numColumns = 2;
		shell.setLayout(gl);

		switch (Gridmap2D.config.game()) {
		case EATERS:
			setupEaters();
			break;
		case TANKSOAR:
			setupTankSoar();
			break;
			
		case TAXI:
			setupTaxi();
			break;
		}

		statusLine = new Label(shell, SWT.BORDER);
		statusLine.setText("Ready");
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			gd.grabExcessHorizontalSpace = true;
			gd.horizontalAlignment = SWT.FILL;
			gd.heightHint = 16;
			statusLine.setLayoutData(gd);
		}
		
		visualWorld.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				Player player = visualWorld.getPlayerAtPixel(new int [] { e.x, e.y });
				if (player == null) {
					return;
				}
				agentDisplay.selectPlayer(player);
			}
		});
		
		shell.addShellListener(new ShellAdapter() {
			public void shellDeactivated(ShellEvent e) {
				agentDisplay.worldChangeEvent();			
				visualWorld.redraw();			
			}
			public void shellActivated(ShellEvent e) {
				agentDisplay.worldChangeEvent();			
				visualWorld.setRepaint();
				visualWorld.redraw();			
			}
			public void shellDeiconified(ShellEvent e) {
				agentDisplay.worldChangeEvent();			
				visualWorld.setRepaint();
				visualWorld.redraw();			
			}
		});
		
		shell.addControlListener(new ControlAdapter() {
			public void controlMoved(ControlEvent e) {
				agentDisplay.worldChangeEvent();			
				visualWorld.setRepaint();
				visualWorld.redraw();			
			}
			public void controlResized(ControlEvent e) {
				agentDisplay.worldChangeEvent();			
				visualWorld.setRepaint();
				visualWorld.redraw();			
			}
		});

		int [] xy = Gridmap2D.config.getWindowPosition();
		if (xy != null && xy.length == 2)
		{
			// it shouldn't be created off-screen
			if ((xy[0] < display.getClientArea().width - 100) && (xy[1] < display.getClientArea().height - 100)) {
				shell.setLocation(xy[0], xy[1]);
			} else {
				logger.warn("Ignoring old window location, screen is too small.");
			}
		}

		VisualWorld.remapPlayerColors(world.getPlayers());
		reset();

		rhs.layout(true);
		shell.layout(true);
		
		shell.open();

		while (!shell.isDisposed()) {
			if (!display.readAndDispatch()) {
				display.sleep();
			}
		}
	}
	
	public void infoMessage(String title, String message) {
		popUpTitle = title;
		popUpMessage = message;
		if (!isDisposed()) {
			display.syncExec(new Runnable() {
				public void run() {
					MessageBox mb = new MessageBox(shell, SWT.ICON_INFORMATION | SWT.WRAP);
					mb.setMessage(popUpMessage);
					mb.setText(popUpTitle);
					mb.open();
				}
			});
		}
	}

	public void errorMessage(String title, String message) {
		popUpTitle = title;
		popUpMessage = message;
		if (!isDisposed()) {
			display.syncExec(new Runnable() {
				public void run() {
					MessageBox mb = new MessageBox(shell, SWT.ICON_ERROR | SWT.WRAP);
					if (popUpMessage == null) {
						// uh oh
						return;
					}
					mb.setMessage(popUpMessage);
					mb.setText(popUpTitle);
					mb.open();
				}
			});
		}
	}
	
	public void setStatus(String status, final Color color) {
		if (!using()) {
			return;
		}
		statusMessage = status;
		if (!isDisposed()) {
			display.syncExec(new Runnable() {
				public void run() {
					statusLine.setText(statusMessage);
					if (color != null) {
						statusLine.setForeground(color);
					}
				}
			});
		}
	}

	void updateWorldGroup() {
		worldGroup.setText("Map: " + Gridmap2D.simulation.getCurrentMapName());
		visualWorld.setSize(visualWorld.getWidth(), visualWorld.getHeight());
		
		GridData gd = new GridData();
		gd.widthHint = visualWorld.getWidth();
		gd.heightHint = visualWorld.getHeight();
		gd.minimumWidth = visualWorld.getWidth();
		gd.minimumHeight = visualWorld.getHeight();
		gd.verticalSpan = 3;
		worldGroup.setLayoutData(gd);
		worldGroup.layout();

		shell.setSize(shell.computeSize(SWT.DEFAULT, SWT.DEFAULT));
	}
	
	void updateCounts() {
		if (Gridmap2D.config.game() == Game.EATERS) {
			EatersMap eMap = (EatersMap)world.getMap();
			foodCount.setText(Integer.toString(eMap.getFoodCount()));
			scoreCount.setText(Integer.toString(eMap.getScoreCount()));
		}
		worldCount.setText(Integer.toString(Gridmap2D.simulation.getWorldCount()));
	}

	boolean isDisposed() {
		if (display == null) {
			return true;
		}
		if (display.isDisposed() || shell == null) {
			return true;
		}
		return shell.isDisposed();
	}

	public void update() {
		if (!isDisposed()) {
			display.syncExec(new Runnable() {
				public void run() {
					agentDisplay.worldChangeEvent();
					visualWorld.redraw();
					updateCounts();
				}
			});
		}
	}

	public void start() {
		if (!isDisposed()) {
			display.syncExec(new Runnable() {
				public void run() {
					simButtons.updateButtons(world.numberOfPlayers());
					mapButtons.updateButtons();
					agentDisplay.updateButtons();
				}
			});
		}
	}

	public void stop() {
		if (!isDisposed()) {
			display.syncExec(new Runnable() {
				public void run() {
					visualWorld.setRepaint();
					visualWorld.redraw();
					simButtons.updateButtons(world.numberOfPlayers());
					mapButtons.updateButtons();
					agentDisplay.updateButtons();
				}
			});
		}
	}
	
	public void playerEvent() {
		if (visualWorld != null && !visualWorld.isDisposed()) {
			visualWorld.setRepaint();
			visualWorld.redraw();
			VisualWorld.remapPlayerColors(world.getPlayers());
			updateCounts();
			simButtons.updateButtons(world.numberOfPlayers());
			agentDisplay.agentEvent();
		}
	}
	
	public void reset() {
		if (!using() || visualWorld == null) {
			return;
		}
		if (!isDisposed()) {
			display.syncExec(new Runnable() {
				public void run() {
					visualWorld.setMap(world.getMap());
					agentDisplay.setMap(world.getMap());
					updateWorldGroup();
					agentDisplay.worldChangeEvent();
					visualWorld.redraw();
					updateCounts();
					simButtons.updateButtons(world.numberOfPlayers());
				}
			});
		}
	}

	public void shutdown() {

		if (humanMove != null) {
			synchronized(humanMove) {
				humanMove.notifyAll();
			}
		}
		
		if (display == null) {
			shell = null;
			return;
		}
		
		if (!display.isDisposed()) {
			display.dispose();
		}
		
		display = null;
		shell = null;
	}
	
	void setVisualWorldFocus() {
		display.syncExec(new Runnable() {
			public void run() {
				visualWorld.setFocus();
			}
		});
	}

	public CommandInfo getHumanCommand(Player player) {
		humanMove = new CommandInfo();
		if (Gridmap2D.config.generalConfig().headless) {
			return humanMove;
		}
//		if (player.getRadarSwitch()) {
//			humanMove.radar = true;
//			humanMove.radarSwitch = true;
//		}
//		if (player.getRadarPower() > 0) {
//			humanMove.radarPower = true;
//			humanMove.radarPowerSetting = player.getRadarPower();
//		}
		if (!isDisposed()) {
			human = player;
			if (player == null) {
				return null;
			}
			setStatus("Enter move for " + player.getColor(), WindowManager.getColor(player.getColor()));
			setVisualWorldFocus();
			synchronized(humanMove) {
				try {
					humanMove.wait();
				} catch (InterruptedException ignored) {}
			}
		}
		CommandInfo theMove = humanMove;
		humanMove = null;
		return theMove;
	}

	public String promptForConfig() {
		FileDialog fd = new FileDialog(shell, SWT.OPEN);
		fd.setText("Select configuration file");
		fd.setFilterPath("config"); // FIXME: broken on linux with swt.jar version 3.3
		fd.setFileName(Names.configs.tanksoarCnf);
		fd.setFilterExtensions(new String[] {"*.cnf", "*.*"});
		return fd.open();
	}

	public void setCogArch(CognitiveArchitecture cogArch) {
		this.cogArch = cogArch;
	}
}
