package soar2d.visuals;

import java.util.Iterator;
import java.util.logging.*;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.Configuration.SimType;
import soar2d.player.*;
import soar2d.world.CellObject;

public class WindowManager {
	private static Logger logger = Logger.getLogger("soar2d");
	
	public static Color white = null;
	public static Color blue = null;
	public static Color red = null;
	public static Color widget_background = null;
	public static Color yellow = null;
	public static Color orange = null;
	public static Color black = null;
	public static Color green = null;
	public static Color purple = null;

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
	MoveInfo humanMove;
	Player human;
	Composite rhs;
	Composite currentSide;

	public static final int kEatersMainMapCellSize = 20;
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
		return null;
	}

	public boolean initialize() {
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
			return false;
		}
		initColors(display);
		return true;
	}
	
	public boolean using() {
		return shell != null;
	}
	
	public void setupEaters() {
		worldGroup = new Group(shell, SWT.NONE);
		worldGroup.setLayout(new FillLayout());
		visualWorld = new VisualWorld(worldGroup, SWT.NONE, kEatersMainMapCellSize);

		visualWorld.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				Player player = visualWorld.getPlayerAtPixel(e.x, e.y);
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
					humanMove.moveDirection = Direction.kNorthInt;
					go = true;
					break;
				case SWT.KEYPAD_6:
					humanMove.move = true;
					humanMove.moveDirection = Direction.kEastInt;
					go = true;
					break;
				case SWT.KEYPAD_2:
					humanMove.move = true;
					humanMove.moveDirection = Direction.kSouthInt;
					go = true;
					break;
				case SWT.KEYPAD_4:
					humanMove.move = true;
					humanMove.moveDirection = Direction.kWestInt;
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
				
				Soar2D.wm.setStatus(human.getColor() + ": " + humanMove.toString(), black);
				
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
		simButtons = new SimulationButtons(group1);
		
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

		agentDisplay = new EatersAgentDisplay(currentSide);
		{
			GridData gd = new GridData();
			agentDisplay.setLayoutData(gd);
		}
	}
	
	public void setupBook() {
		assert false;
	}
	
	public void setupTankSoar() {
		worldGroup = new Group(shell, SWT.NONE);
		worldGroup.setLayout(new FillLayout());
		visualWorld = new VisualWorld(worldGroup, SWT.NONE, kTanksoarMainMapCellSize);

		visualWorld.addKeyListener(new KeyAdapter() {
			public void keyPressed(KeyEvent e) {
				if (humanMove == null) {
					return;
				}
				boolean go = false;
				int facing = human.getFacingInt();
				switch (e.keyCode) {
				case SWT.KEYPAD_8:
					humanMove.move = true;
					humanMove.moveDirection = facing;
					break;
				case SWT.KEYPAD_6:
					humanMove.move = true;
					humanMove.moveDirection = Direction.rightOf[facing];
					break;
				case SWT.KEYPAD_2:
					humanMove.move = true;
					humanMove.moveDirection = Direction.backwardOf[facing];
					break;
				case SWT.KEYPAD_4:
					humanMove.move = true;
					humanMove.moveDirection = Direction.leftOf[facing];
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
				
				Soar2D.wm.setStatus(human.getColor() + ": " + humanMove.toString(), black);
				
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
		simButtons = new SimulationButtons(group1);
		
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
		
		agentDisplay = new TankSoarAgentDisplay(currentSide);
		{
			GridData gd = new GridData();
			gd.horizontalSpan = 2;
			agentDisplay.setLayoutData(gd);
		}
	}
	
	private void createEditSide() {
		currentSide = new Composite(rhs, SWT.NONE);
		{
			GridLayout gl = new GridLayout();
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			currentSide.setLayout(gl);
			
			GridData gd = new GridData();
			currentSide.setLayoutData(gd);
		}
		
		Label templatesLabel = new Label(currentSide, SWT.NONE);
		templatesLabel.setText("Templates:");
		
		Table templatesTable = new Table(currentSide, SWT.BORDER | SWT.V_SCROLL);
		{
			TableItem item = new TableItem(templatesTable, SWT.NONE);
			item.setText("<empty>");
		}
		Iterator<CellObject> templateIter = Soar2D.simulation.world.map.getObjectManager().getTemplates().iterator();
		while(templateIter.hasNext()) {
			CellObject template = templateIter.next();
			String templateName = template.getName();
			
			if (templateName.equals(Names.kPropertyMissile)
					|| templateName.equals(Names.kExplosion)
					|| templateName.equals(Names.kGround)) {
				continue;
			}
			
			TableItem item = new TableItem(templatesTable, SWT.NONE);
			item.setText(template.getName());
		}
		
		Button editTemplateButton = new Button(currentSide, SWT.PUSH);
		editTemplateButton.setText("Edit Selected Template");

		Button destroyTemplateButton = new Button(currentSide, SWT.PUSH);
		destroyTemplateButton.setText("Destroy Selected Template");

		Button newTemplateButton = new Button(currentSide, SWT.PUSH);
		newTemplateButton.setText("Create New Template");
	}
	
	Menu menuBar;
	Menu fileMenu;
	Menu mapMenu;
	Menu helpMenu;
	
	MenuItem fileMenuHeader;
	MenuItem mapMenuHeader;
	MenuItem helpMenuHeader;
	
	MenuItem fileConfigurationItem;
	MenuItem fileExitItem;
	
	MenuItem mapEditItem;
	
	MenuItem helpAboutItem;

	boolean mapEditMode = false;
	
	private void mapEditModeChange() {
		// if we're going in to edit mode, the agent list must be empty
		if (mapEditMode == false) {
			// we're going in to edit mode
			if (Soar2D.simulation.world.getPlayers().size() > 0) {
				Soar2D.control.infoPopUp("Destroy all agents before editing the map.");
				return;
			}
		}
		
		// flip the bit
    	mapEditMode = !mapEditMode;
    	
    	if (mapEditMode) {
    		// we're going in to edit mode, destroy current side, create edit side
    		// TODO: just testing
    		currentSide.dispose();
   			createEditSide();
    		
    		updateWorldGroup();
    		
    		rhs.layout(true);
    		shell.layout(true);

    	} else {	
    		// we're going in to normal mode, destroy current side, create normal side
    		currentSide.dispose();
    		switch (Soar2D.config.getType()) {
    		case kEaters:
    			createEatersSide();
    			break;
    		case kTankSoar:
        		createTankSoarSide();
        		break;
    		case kBook:
    			assert false;
    		}
    		
    		updateWorldGroup();
    		
    		rhs.layout(true);
    		shell.layout(true);
    	}
	}
	
	public void run() {
		
		menuBar = new Menu(shell, SWT.BAR);
		fileMenuHeader = new MenuItem(menuBar, SWT.CASCADE);
		fileMenuHeader.setText("&File");
		
		fileMenu = new Menu(shell, SWT.DROP_DOWN);
		fileMenuHeader.setMenu(fileMenu);
		
		fileConfigurationItem = new MenuItem(fileMenu, SWT.PUSH);
		fileConfigurationItem.setText("&Configuration");
		fileConfigurationItem.addSelectionListener(new SelectionListener() {
		    public void widgetSelected(SelectionEvent event) {
		    	ConfigurationEditor ce = new ConfigurationEditor(shell);
		    	ce.open();
		    }
			
			public void widgetDefaultSelected(SelectionEvent event) {
		    	ConfigurationEditor ce = new ConfigurationEditor(shell);
		    	ce.open();
			}
		});

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
		
		mapEditItem = new MenuItem(mapMenu, SWT.PUSH);
		mapEditItem.setText("&Edit Map");
		mapEditItem.addSelectionListener(new SelectionListener() {
		    public void widgetSelected(SelectionEvent event) {
		    	mapEditModeChange();
		    }

		    public void widgetDefaultSelected(SelectionEvent event) {
		    	mapEditModeChange();
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
		        infoMessage("About", "Soar2D version x.x\nby Jonathan Voigt\nvoigtjr@gmail.com");
			}
			
			public void widgetDefaultSelected(SelectionEvent event) {
				infoMessage("About", "Soar2D version x.x\nby Jonathan Voigt\nvoigtjr@gmail.com");
			}
		});
		
		shell.setMenuBar(menuBar);
		
		GridLayout gl = new GridLayout();
		gl.numColumns = 2;
		shell.setLayout(gl);

		switch (Soar2D.config.getType()) {
		case kEaters:
			setupEaters();
			break;
		case kTankSoar:
			setupTankSoar();
			break;
			
		case kBook:
			setupBook();
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
				Player player = visualWorld.getPlayerAtPixel(e.x, e.y);
				if (player == null) {
					return;
				}
				agentDisplay.selectPlayer(player);
			}
		});
		
		updateWorldGroup();

		VisualWorld.remapPlayerColors();

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
		worldGroup.setText("Map: " + Soar2D.config.map.getName());
		visualWorld.setSize(visualWorld.getWidth(), visualWorld.getHeight());
		GridData gd = new GridData();
		gd.widthHint = visualWorld.getWidth();
		gd.heightHint = visualWorld.getHeight();
		gd.verticalSpan = 3;
		worldGroup.setLayoutData(gd);

		shell.setSize(shell.computeSize(SWT.DEFAULT, SWT.DEFAULT));
	}
	
	void updateCounts() {
		if (Soar2D.config.getType() == SimType.kEaters) {
			foodCount.setText(Integer.toString(Soar2D.simulation.world.map.getFoodCount()));
			scoreCount.setText(Integer.toString(Soar2D.simulation.world.map.getScoreCount()));
		}
		worldCount.setText(Integer.toString(Soar2D.simulation.world.getWorldCount()));
	}

	boolean isDisposed() {
		return (display.isDisposed() || shell.isDisposed());
	}

	boolean agentDisplayUpdated;
	boolean worldDisplayUpdated;
	public void update() {
		if (!isDisposed()) {
			synchronized(this) {
				agentDisplayUpdated = false;
				worldDisplayUpdated = false;
			}
			display.syncExec(new Runnable() {
				public void run() {
					agentDisplay.worldChangeEvent();
					visualWorld.redraw();
					updateCounts();
				}
			});
			synchronized(this) {
				while (!agentDisplayUpdated || !worldDisplayUpdated) {
					try {
						this.wait();
					} catch (InterruptedException ignored) {}
				}
			}
		}
	}

	public void start() {
		if (!isDisposed()) {
			display.syncExec(new Runnable() {
				public void run() {
					simButtons.updateButtons();
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
					simButtons.updateButtons();
					mapButtons.updateButtons();
					agentDisplay.updateButtons();
				}
			});
		}
	}
	
	public void playerEvent() {
		if (visualWorld != null) {
			visualWorld.setRepaint();
			visualWorld.redraw();
			VisualWorld.remapPlayerColors();
			updateCounts();
			simButtons.updateButtons();
			agentDisplay.agentEvent();
		}
	}
	
	public void reset() {
		if (!isDisposed()) {
			display.syncExec(new Runnable() {
				public void run() {
					updateWorldGroup();
					visualWorld.resetBackground();
					visualWorld.setRepaint();
					agentDisplay.worldChangeEvent();
					visualWorld.redraw();
					updateCounts();
					simButtons.updateButtons();
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

	public MoveInfo getHumanMove(Player player) {
		humanMove = new MoveInfo();
		if (player.getRadarSwitch()) {
			humanMove.radar = true;
			humanMove.radarSwitch = true;
		}
		if (player.getRadarPower() > 0) {
			humanMove.radarPower = true;
			humanMove.radarPowerSetting = player.getRadarPower();
		}
		if (!isDisposed()) {
			human = player;
			setStatus("Enter move for " + player.getColor(), red);
			setVisualWorldFocus();
			synchronized(humanMove) {
				try {
					humanMove.wait();
				} catch (InterruptedException e) {}
			}
		}
		MoveInfo theMove = humanMove;
		humanMove = null;
		return theMove;
	}

	public String promptForConfig() {
		FileDialog fd = new FileDialog(shell, SWT.OPEN);
		fd.setText("Select configuration file");
		fd.setFilterPath(System.getProperty("user.dir") + System.getProperty("file.separator"));
		fd.setFileName(Soar2D.config.kDefaultXMLEatersSettingsFile);
		fd.setFilterExtensions(new String[] {"*.xml", "*.*"});
		return fd.open();
	}
}
