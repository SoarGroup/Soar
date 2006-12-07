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

	protected Display display;
	protected Shell shell;
	Label scoreCount;
	Label foodCount;
	SimulationButtons simButtons;
	MapButtons mapButtons;
	VisualWorld visualWorld;
	AgentDisplay agentDisplay;
	Group worldGroup;

	public static final int kMainMapCellSize = 20;
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
		visualWorld = new VisualWorld(worldGroup, SWT.NONE, kMainMapCellSize);
		updateWorldGroup();
		gd = new GridData();
		gd.widthHint = visualWorld.getWidth();
		gd.heightHint = visualWorld.getHeight();
		gd.verticalSpan = 3;
		worldGroup.setLayoutData(gd);
		visualWorld.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				Player eater = visualWorld.getPlayerAtPixel(e.x, e.y);
				if (eater == null) {
					return;
				}
				agentDisplay.selectPlayer(eater);
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
		
		VisualWorld.remapFoodColors();
		VisualWorld.remapPlayerColors();

		shell.setText("Java Eaters");
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
	
	public void errorMessage(String title, String message) {
		MessageBox mb = new MessageBox(shell, SWT.ICON_ERROR | SWT.WRAP);
		mb.setMessage(message);
		mb.setText(title);
		mb.open();
	}

	void updateWorldGroup() {
		worldGroup.setText(Soar2D.config.map.getName());
		visualWorld.setSize(visualWorld.getWidth(), visualWorld.getHeight());
		GridData gd = new GridData();
		gd.widthHint = visualWorld.getWidth();
		gd.heightHint = visualWorld.getHeight();
		gd.verticalSpan = 3;
		worldGroup.setLayoutData(gd);
		shell.setSize(shell.computeSize(SWT.DEFAULT, SWT.DEFAULT));
	}
	
	void updateFoodAndScoreCount() {
		foodCount.setText(Integer.toString(Soar2D.simulation.world.getFoodCount()));
		scoreCount.setText(Integer.toString(Soar2D.simulation.world.getScoreCount()));
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
	
	public void agentEvent() {
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
					VisualWorld.remapFoodColors();
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
		if (display == null) {
			return;
		}
		display.dispose();
		display = null;
		shell = null;
	}
}
