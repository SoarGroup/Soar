package soar2d.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.player.*;

public class WindowManager {
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

	public static final int kEatersMainMapCellSize = 20;
	public static final int kTanksoarMainMapCellSize = 32;
	public static final String kFoodRemaining = "Food remaining: ";
	public static final String kScoreRemaining = "Points remaining: ";

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
		display = new Display();
		if (display == null) {
			return false;
		}

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
	
	public void run() {
		GridLayout gl = new GridLayout();
		gl.numColumns = 2;
		shell.setLayout(gl);
		
		GridData gd;
		
		worldGroup = new Group(shell, SWT.NONE);
		worldGroup.setLayout(new FillLayout());
		if (Soar2D.config.eaters) {
			visualWorld = new VisualWorld(worldGroup, SWT.NONE, kEatersMainMapCellSize);
		} else {
			visualWorld = new VisualWorld(worldGroup, SWT.NONE, kTanksoarMainMapCellSize);
		}

		gd = new GridData();
		gd.widthHint = visualWorld.getWidth();
		gd.heightHint = visualWorld.getHeight();
		gd.verticalSpan = 3;
		worldGroup.setLayoutData(gd);
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
				if (Soar2D.config.eaters) {
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
				} else if (Soar2D.config.tanksoar) {
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
						go = true;
						break;
					case SWT.KEYPAD_0:
						humanMove.fire = !humanMove.fire;
						break;
					case SWT.KEYPAD_MULTIPLY:
						humanMove.stopSim = !humanMove.stopSim;
						break;
					default:
						break;
					}
				} else {
					return;
				}
				if (go) {
					synchronized(humanMove) {
						humanMove.notify();
					}
				}
			}
		});

		Group group1 = new Group(shell, SWT.NONE);
		gd = new GridData();
		group1.setLayoutData(gd);
		group1.setText("Simulation");
		group1.setLayout(new FillLayout());
		simButtons = new SimulationButtons(group1);
		
		Group group2 = new Group(shell, SWT.NONE);
		gd = new GridData();
		group2.setLayoutData(gd);
		group2.setText("Map");
		gl = new GridLayout();
		gl.numColumns = 2;
		group2.setLayout(gl);
		
		Label foodLabel = new Label(group2, SWT.NONE);
		gd = new GridData();
		foodLabel.setLayoutData(gd);
		foodLabel.setText(kFoodRemaining);
		
		foodCount = new Label(group2, SWT.NONE);
		gd = new GridData();
		gd.widthHint = 50;
		foodCount.setLayoutData(gd);
		
		Label scoreLabel = new Label(group2, SWT.NONE);
		gd = new GridData();
		scoreLabel.setLayoutData(gd);
		scoreLabel.setText(kScoreRemaining);
		
		scoreCount = new Label(group2, SWT.NONE);
		gd = new GridData();
		gd.widthHint = 50;
		scoreCount.setLayoutData(gd);
		
		updateFoodAndScoreCount();
		
		mapButtons = new MapButtons(group2);
		gd = new GridData();
		gd.horizontalSpan = 2;
		mapButtons.setLayoutData(gd);

		agentDisplay = new AgentDisplay(shell);
		gd = new GridData();
		agentDisplay.setLayoutData(gd);
		
		statusLine = new Label(shell, SWT.BORDER);
		statusLine.setText("Ready");
		gd = new GridData();
		gd.horizontalSpan = 2;
		gd.widthHint = shell.computeSize(SWT.DEFAULT, SWT.DEFAULT).x - 20;
		gd.heightHint = 16;
		statusLine.setLayoutData(gd);
		
		updateWorldGroup();

		VisualWorld.remapPlayerColors();

		if (Soar2D.config.eaters) {
			shell.setText("Eaters");
		} else if (Soar2D.config.tanksoar) {
			shell.setText("TankSoar");
		} else {
			shell.setText("Soar 2D");
		}
		shell.setSize(shell.computeSize(SWT.DEFAULT, SWT.DEFAULT));
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
	
	public void setStatus(String status) {
		if (!using()) {
			return;
		}
		statusMessage = status;
		if (!isDisposed()) {
			display.syncExec(new Runnable() {
				public void run() {
					statusLine.setText(statusMessage);
				}
			});
		}
	}

	void updateWorldGroup() {
		worldGroup.setText(Soar2D.config.map.getName());
		visualWorld.setSize(visualWorld.getWidth(), visualWorld.getHeight());
		GridData gd = new GridData();
		gd.widthHint = visualWorld.getWidth();
		gd.heightHint = visualWorld.getHeight();
		gd.verticalSpan = 3;
		worldGroup.setLayoutData(gd);

		gd = new GridData();
		gd.horizontalSpan = 2;
		gd.widthHint = shell.computeSize(SWT.DEFAULT, SWT.DEFAULT).x - 20;
		gd.heightHint = 16;
		statusLine.setLayoutData(gd);

		shell.setSize(shell.computeSize(SWT.DEFAULT, SWT.DEFAULT));
	}
	
	void updateFoodAndScoreCount() {
		foodCount.setText(Integer.toString(Soar2D.simulation.world.map.getFoodCount()));
		scoreCount.setText(Integer.toString(Soar2D.simulation.world.map.getScoreCount()));
	}

	boolean isDisposed() {
		return (display.isDisposed() || shell.isDisposed());
	}

	public void update() {
		if (!isDisposed()) {
			display.syncExec(new Runnable() {
				public void run() {
					visualWorld.redraw();
					updateFoodAndScoreCount();
					agentDisplay.worldChangeEvent();
				}
			});
		}
	}

	public void start() {
		if (!isDisposed()) {
			display.syncExec(new Runnable() {
				public void run() {
					visualWorld.setFocus();
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
			updateFoodAndScoreCount();
			simButtons.updateButtons();
			agentDisplay.agentEvent();
		}
	}
	
	public void reset() {
		if (!isDisposed()) {
			display.syncExec(new Runnable() {
				public void run() {
					updateWorldGroup();
					visualWorld.setRepaint();
					visualWorld.redraw();
					updateFoodAndScoreCount();
					simButtons.updateButtons();
					agentDisplay.worldChangeEvent();
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

	public MoveInfo getHumanMove(String color) {
		humanMove = new MoveInfo();
		setStatus("Enter move for " + color);
		try {
			synchronized(humanMove) {
				humanMove.wait();
			}
		} catch (InterruptedException e) {
		}
		return humanMove;
	}
}
