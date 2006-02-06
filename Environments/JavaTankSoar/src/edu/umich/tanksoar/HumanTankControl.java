/* File: HumanTankControl.java
 * Aug 24, 2004
 */
package edu.umich.tanksoar;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.*;
import org.eclipse.swt.graphics.*;

import edu.umich.JavaBaseEnvironment.SimulationControlListener;
import edu.umich.JavaBaseEnvironment.Location;
import edu.umich.JavaBaseEnvironment.SoarAgent;

import org.eclipse.swt.layout.*;
import org.eclipse.swt.events.*;
/**
 * This class represents a Human-controlled <code>Tank</code>. Clients can use this class to
 * create a <code>Tank</code> which can be controlled by a human user. This provides
 * an interface through which different movements, missiles, radar, and shields
 * can be controlled via button clicks. It does not control the view of the
 * <code>Tank</code>.
 * @author jduchi
 */
public class HumanTankControl extends Tank implements SimulationControlListener{

	/** Constant to indicate that no decision has been made by the user. */
	public static final int NODECISION = -1;
	/** Constant to indicate that a decision to move forward has been made by the user. */
	public static final int MOVEFORWARD = 0;
	/** Constant to indicate that a decision to move backward has been made by the user. */
	public static final int MOVEBACKWARD = 1;
	/** Constant to indicate that a decision to move left has been made by the user. */
	public static final int STEPLEFT = 2;
	/** Constant to indicate that a decision to move right has been made by the user. */
	public static final int STEPRIGHT = 3;
	/** Constant to indicate that a decision to fire the missile has been made by the user. */
	public static final int FIREMISSILE = 6;
	
	/** <code>Display</code> managing the shells visible to the user. Managed elsewhere. */
	private Display myDisplay;
	/** The <code>Shell</code> containing the manual controls for this <code>Tank</code>. */
	private Shell myShell;
	
	/** <code>Button</code> for moving the <code>Tank</code> forward. */
	private Button forward;
	/** <code>Button</code> for moving the <code>Tank</code> backward. */
	private Button backward;
	/** <code>Button</code> for moving the <code>Tank</code> right. */
	private Button right;
	/** <code>Button</code> for moving the <code>Tank</code> left. */
	private Button left;
	/** <code>Button</code> for turning the <code>Tank</code> clockwise (right). */
	private Button clockwise;
	/** <code>Button</code> for turning the <code>Tank</code> counterclockwise (left). */
	private Button counterclockwise;
	/** <code>Button</code> for firing a missile from the <code>Tank</code>. */
	private Button fire;
	/** <code>Button</code> for turning the <code>Tank</code>'s shields on. */
	private Button shields;
	/** <code>Button</code> for finishing the turn by clicking this check mark. */
	private Button endTurn;
	/** <code>Slider</code> for setting the <code>Tank</code>'s radar setting. */
	private Slider radarSettingSlider;
	/** <code>Button</code> to check whether or not the <code>Tank</code>'s radar ought to be on. */
	private Button radarCheck;
	
	
	private boolean decisionFinished = false;
	private int movementDecision = NODECISION;
	private boolean enableShields = false;
	private boolean fireMissile = false;
	private boolean radarChecked = true;
	private int radarSliderSetting = 0;
	
	static private Image forwardGif;
	static private Image backwardGif;
	static private Image rightGif;
	static private Image leftGif;
	static private Image clockwiseGif;
	static private Image counterclockwiseGif;
	static private Image fireGif;
	static private Image shieldsGif;
	static private Image doneGif;
	
	/**
	 * Constructs a new instance of <code>HumanTankControl</code>. This instance will not have a 
	 * <code>Display</code> associated with it until <code>initControls(Display d, String imageDirectory)</code>
	 * is invoked. This constructor simply invokes the super constructor for <code>Tank</code>.
	 * @see edu.umich.tanksoar#Tank
	 * @param colorName The <code>String</code> name of the color to be used for this <code>Tank</code>.
	 * @param loc The <code>Location</code> at which this <code>Tank</code> is being created.
	 * @param tc The <code>TankSoarJControl</code> running the simulation.
	 */
	public HumanTankControl(String colorName, Location loc, TankSoarJControl tc){
		super(colorName, loc, tc);
		myTC.addSimulationControlListener(this);
	}
	
	/**
	 * Constructs a new instance of <code>HumanTankControl</code>. This instance will have a 
	 * <code>Display</code> associated with it. This constructor invokes the super constructor
	 * for <code>Tank</code>, then initializes the <code>Display</code> and controls.
	 * @see edu.umich.tanksoar#Tank
	 * @param colorName The <code>String</code> name of the color to be used for this <code>Tank</code>.
	 * @param loc The <code>Location</code> at which this <code>Tank</code> is being created.
	 * @param tc The <code>TankSoarJControl</code> running the simulation.
	 * @param d The <code>Display</code> that is managing all the <code>Shell</code>s visible to the user.
	 * @param imageDirectory The <code>String</code> name if the directory in which the .gif files are stored
	 * for the visual representation of the simulation.
	 */
	public HumanTankControl(String colorName, Location loc, TankSoarJControl tc, Display d){
		super(colorName, loc, tc);
		initControls(d);
	}
	
	/**
	 * Initializes all the manual controls for a human-controlled <code>Tank</code>.
	 * @param d The <code>Display</code> managing the windows of the simulation.
	 * @param imageDirectory The file system directory that stores the *.gif files
	 * associated with the images, to which "buttons" +
	 * <code>System.getProperty("file.separator")</code> will be concatenated.
	 */
	public void initControls(Display d){
		myDisplay = d;
		loadImages();
		myShell = new Shell(d);
		myShell.addShellListener(new HumanTankShellAdapter());
		myShell.setLayout(new RowLayout(SWT.VERTICAL));
		loadImages();
		setButtons();
		myShell.pack();
		myShell.setText(myColorName + " Manual Controls");
		myShell.open();
		if(!myTC.isRunning()){
			enableAll(false);
		}
	}
	
	/**
	 * An inner class to open message boxes when a user closes the control window
	 * for a <code>HumanTankControl</code>.
	 */
	private class HumanTankShellAdapter extends ShellAdapter{
		public void shellClosed(ShellEvent e){
			if(!(e.doit = commandFromAbove)){
				MessageBox mb = new MessageBox(myShell, SWT.OK | SWT.CANCEL | SWT.ICON_WARNING);
				mb.setText("Destroy Human-Controlled Tank?");
				mb.setMessage("Are you sure you want to destroy the human-controlled tank?");
				e.doit = (mb.open() == SWT.OK);
				if(!myTC.canDestroyAgent()){
					e.doit = false;
					MessageBox mb2 = new MessageBox(myShell, SWT.OK);
					mb2.setText("C'est la vie");
					mb2.setMessage("You can't destroy a Tank while the simulation is running.");
					mb2.open();
				}
				if(e.doit){
					myTC.destroyAgent(HumanTankControl.this);
				}
			}
		}
	}
	
	/**
	 * Loads the images associated with the buttons for a manually-controlled <code>
	 * Tank</code>, which include up, down, right, left, etc.
	 */
	private void loadImages(){
		if(forwardGif != null) return;
		forwardGif = new Image(myDisplay,
				TanksoarJ.class.getResourceAsStream("/images/buttons/up.gif"));
		backwardGif = new Image(myDisplay,
				TanksoarJ.class.getResourceAsStream("/images/buttons/down.gif"));
		rightGif = new Image(myDisplay,
				TanksoarJ.class.getResourceAsStream("/images/buttons/right.gif"));
		leftGif = new Image(myDisplay,
				TanksoarJ.class.getResourceAsStream("/images/buttons/left.gif"));
		clockwiseGif = new Image(myDisplay,
				TanksoarJ.class.getResourceAsStream("/images/buttons/clockwise.gif"));
		counterclockwiseGif = new Image(myDisplay,
				TanksoarJ.class.getResourceAsStream("/images/buttons/counterclockwise.gif"));
		fireGif = new Image(myDisplay,
				TanksoarJ.class.getResourceAsStream("/images/buttons/fire.gif"));
		shieldsGif = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/buttons/shields.gif"));
		doneGif = new Image(myDisplay,
				TanksoarJ.class.getResourceAsStream("/images/buttons/done.gif"));
	}
	
	/**
	 * Initializes all the buttons to be used for the manual controls of the human-controlled
	 * <code>Tank</code>. These buttons are for movement, firing, radar, and shields.
	 */
	public void setButtons(){
		Composite top = new Composite(myShell, SWT.NONE);
		top.setLayout(new GridLayout(3, true));
		counterclockwise = new Button(top, SWT.PUSH);
		counterclockwise.setImage(counterclockwiseGif);
		counterclockwise.setToolTipText("Turn tank counterclockwise");
		counterclockwise.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				movementDecision = TURNLEFT;
				enableMovement(false);
			}
		});
		forward = new Button(top, SWT.PUSH);
		forward.setImage(forwardGif);
		forward.setToolTipText("Move tank forward");
		forward.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				movementDecision = MOVEFORWARD;
				enableMovement(false);
			}
		});
		clockwise = new Button(top, SWT.PUSH);
		clockwise.setImage(clockwiseGif);
		clockwise.setToolTipText("Turn tank clockwise");
		clockwise.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				movementDecision = TURNRIGHT;
				enableMovement(false);
			}
		});
		left = new Button(top, SWT.PUSH);
		left.setImage(leftGif);
		left.setToolTipText("Step tank left");
		left.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				movementDecision = STEPLEFT;
				enableMovement(false);
			}
		});
		shields = new Button(top, SWT.TOGGLE);
		shields.setImage(shieldsGif);
		shields.setToolTipText("Turn tank's shields on");
		shields.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				enableShields = shields.getSelection();
				if(!enableShields){
					shields.setToolTipText("Turn tank's shields on");
				} else {
					shields.setToolTipText("Turn tank's shields off");
				}
			}
		});
		right = new Button(top, SWT.PUSH);
		right.setImage(rightGif);
		right.setToolTipText("Step tank right");
		right.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				movementDecision = STEPRIGHT;
				enableMovement(false);
			}
		});
		fire = new Button(top, SWT.PUSH);
		fire.setImage(fireGif);
		fire.setToolTipText("Fire missile");
		fire.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				fireMissile = true;
				fire.setEnabled(false);
			}
		});
		backward = new Button(top, SWT.PUSH);
		backward.setImage(backwardGif);
		backward.setToolTipText("Move tank backward");
		backward.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				movementDecision = MOVEBACKWARD;
				enableMovement(false);
			}
		});
		endTurn = new Button(top, SWT.PUSH);
		endTurn.setImage(doneGif);
		endTurn.setToolTipText("Finish your turn");
		endTurn.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				decisionFinished = true;
			}
		});
		
		/* RADAR SETTINGS */
		
		Composite bottom = new Composite(myShell, SWT.NONE);
		Composite topOfBottom = new Composite(bottom, SWT.NONE);
		topOfBottom.setLayout(new RowLayout(SWT.HORIZONTAL));
		final Text radarText = new Text(topOfBottom, SWT.NONE);
		radarText.setEditable(false);
		radarText.setText(String.valueOf(radarSetting));
		radarText.setLayoutData(new RowData(25, 25));
		Label radarLabel = new Label(topOfBottom, SWT.NONE);
		radarLabel.setText("Radar");
		radarCheck = new Button(topOfBottom, SWT.CHECK);
		radarCheck.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				radarChecked = radarCheck.getSelection();
			}
		});
		radarSettingSlider = new Slider(bottom, SWT.HORIZONTAL);
		bottom.setLayout(new RowLayout(SWT.VERTICAL));
		radarSettingSlider.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				radarText.setText(String.valueOf(radarSettingSlider.getSelection()));
				radarSliderSetting = radarSettingSlider.getSelection();
			}
		});
		radarSettingSlider.setLayoutData(new RowData(140, 25));
		radarSettingSlider.setMaximum(Tank.MaxRadar + 1);
		radarSettingSlider.setMinimum(1);
		radarSettingSlider.setThumb(1);
		radarSettingSlider.setSelection(radarSetting);
		radarSettingSlider.setIncrement(1);
		
	}
	
	/**
	 * Sets all the buttons to be enabled or not, depending on the parameter. Will thus
	 * allow the user to press them or force them not to be pressed.
	 * @param enabled <code>true</code> to enable the buttons, <code>false</code> to
	 * disable them all.
	 */
	private void enableAll(final boolean enabled){
		if(myDisplay.isDisposed() || myTC.hasQuit()) return;	
		myDisplay.asyncExec(new Runnable(){
				public void run(){
					if(myTC.hasQuit()) return;
					forward.setEnabled(enabled);
					backward.setEnabled(enabled);
					right.setEnabled(enabled);
					left.setEnabled(enabled);
					fire.setEnabled(enabled);
					counterclockwise.setEnabled(enabled);
					clockwise.setEnabled(enabled);
					shields.setEnabled(enabled);
					endTurn.setEnabled(enabled);
					radarSettingSlider.setEnabled(enabled);
					radarSettingSlider.setSelection(radarSetting);
					radarSliderSetting = radarSetting;
					radarCheck.setEnabled(enabled);
					radarCheck.setSelection(radarOn);
				}
			});
	}
	
	/**
	 * Sets the buttons relating to movement to be enabled--forward, backward, left, right,
	 * and the turning buttons.
	 * @param enabled <code>true</code> to enable the buttons, <code>false</code> to
	 * disable them all.
	 */
	private void enableMovement(final boolean enabled){
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				forward.setEnabled(enabled);
				backward.setEnabled(enabled);
				right.setEnabled(enabled);
				left.setEnabled(enabled);
				counterclockwise.setEnabled(enabled);
				clockwise.setEnabled(enabled);
			}
		});
	}
	
	protected TankOutputInfo getDecision(TankInputInfo sensors){
		TankOutputInfo result = new TankOutputInfo();
		enableAll(true);
		decisionFinished = false;
		movementDecision = NODECISION;
		fireMissile = false;
		while(!decisionFinished){
			if(!myTC.isStepping() && !myTC.isRunning()){
				enableAll(false);
				return (result);
			}
		}
		if(fireMissile){
			result.fire = new TankOutputInfo.FireOutput("missile");
		}
		if(enableShields && !shieldsOn){
			result.shields = new TankOutputInfo.ShieldsOutput(TankOutputInfo.on);
		} else if(!enableShields && shieldsOn){
			result.shields = new TankOutputInfo.ShieldsOutput(TankOutputInfo.off);
		}
		if(radarChecked && !radarOn){
			result.radar = new TankOutputInfo.RadarOutput(TankOutputInfo.on);
		} else if(!radarChecked && radarOn){
			result.radar = new TankOutputInfo.RadarOutput(TankOutputInfo.off);
		}
		if(radarSetting != radarSliderSetting){
			result.radar_power = new TankOutputInfo.Radar_PowerOutput(radarSliderSetting);
			radarSetting = radarSliderSetting;
			fireRadarChangedNotification();
		}
		switch(movementDecision){
		case(TURNLEFT):{ result.rotate = new TankOutputInfo.RotateOutput("left"); break; }
		case(TURNRIGHT):{ result.rotate = new TankOutputInfo.RotateOutput("right"); break; }
		case(MOVEFORWARD):{ result.move = new TankOutputInfo.MoveOutput("forward"); break; }
		case(MOVEBACKWARD):{ result.move = new TankOutputInfo.MoveOutput("backward"); break; }
		case(STEPLEFT):{ result.move = new TankOutputInfo.MoveOutput("left"); break; }
		case(STEPRIGHT):{ result.move = new TankOutputInfo.MoveOutput("right"); break; }
		}
		enableAll(false);
		return(result);
	}
	
	public void agentCreated(SoarAgent created){ /* IGNORED */ }
	
	/** Indicates whether the model controlling the simulation called for the <code>Tank</code>
	 * to be destroyed, in which case the user will not be questioned about destroying the
	 * <code>Tank</code>. */
	private boolean commandFromAbove = false;
	
	public void agentDestroyed(SoarAgent destroyed){
		if(equals(destroyed)){
			commandFromAbove = true;
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					if(!myShell.isDisposed()) myShell.close();
				}
			});
		}
		myTC.removeSimulationControlListener(this);
	}
	
	public void locationChanged(Location changed){ /* IGNORED */ }
	
	public void newMap(String message){ /* IGNORED */ }
	
	public void simEnded(String message){ /* IGNORED */ }
	
	public void simQuit(){ /* IGNORED--The disposed myDisplay will close me */ }
	
	public void simStarted(){ /* IGNORED */ }
	
	public void worldCountChanged(int worldCount){ /* IGNORED */ }
}
