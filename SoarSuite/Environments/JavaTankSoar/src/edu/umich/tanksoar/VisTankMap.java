/* File: VisTankMap.java
 * Aug 25, 2004
 */
package edu.umich.tanksoar;

import edu.umich.JavaBaseEnvironment.AgentWindow;
import edu.umich.JavaBaseEnvironment.SimulationControlListener;
import edu.umich.JavaBaseEnvironment.SimulationControl;
import edu.umich.JavaBaseEnvironment.SoarAgent;
import edu.umich.JavaBaseEnvironment.Location;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.SWT;
/**
 * Implements a shell that holds the map of the TankSoar simulation, as well as
 * information about the <code>Tanks</code>' <code>HealthShieldsDisplay</code>s.
 * <p>Unless otherwise specified, provides empty implementations of listening methods.</p>
 * @author John Duchi
 */
public class VisTankMap extends AgentWindow implements SimulationControlListener,DisposeListener{

	/** <code>Button</code> to make the simulation take one step (by calling <code>SimulationControl.singleStep()</code>. */
	private Button stepButton;
	/** <code>Button</code> to make the simulation run (by calling <code>SimulationControl.run()</code>. */
	private Button runButton;
	/** <code>Button</code> to make the simulation stop (by calling <code>SimulationControl.stop()</code>. */
	private Button stopButton;
	/** <code>boolean</code> to tell if the <code>Shell</code> is open. */
    private boolean amOpen = false;
    /** The <code>Display</code> managing all the visible <code>Shell</code>s. */
	private Display myDisplay;
	/** The <code>Shell</code> containing the <code>DrawTankMap</code> and the <code>Tank</code>'s
	 * <code>HealthShieldsDisplay</code>s. */
	private Shell myShell;
	/** The <code>TankSoarJControl</code> running the simulation. */
	private TankSoarJControl myTC;
	/** The <code>Composite</code> making up the part inside <code>myShell</code> that contains the
	 * <code>HealthShieldsDisplay</code>s. */
	private Composite leftHalf;
	
	/**
	 * Constructs a new <code>VisTankMap</code> with the specified <code>Display</code> and
	 * control.
	 * @param d The <code>Display</code> managing the GUI parts of the simulation.
	 * @param tc The <code>TankSoarJControl</code> that is managing the simulation. If
	 * tc is not a <code>TankSoarJControl</code>, will throw a <code>ClassCastException</code>.
	 * @param imageDirectory The <code>String</code> path to the directory where the images
	 * for TankSoar are stored.
	 */
	public VisTankMap(Display d, SimulationControl tc){
		myDisplay = d;
		myTC = (TankSoarJControl)tc;
		myTC.addSimulationControlListener(this);
	}
	
	public boolean isMapOpen() { return amOpen; }
	
	protected void initShell(){
		final VisTankMap tmp = this;
		if(myDisplay == null || myDisplay.isDisposed()) return;
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(myShell == null || myShell.isDisposed()) myShell = new Shell(myDisplay);
				myShell.addDisposeListener(tmp);
				myShell.setBackground(myDisplay.getSystemColor(SWT.COLOR_WHITE));
				myShell.setLayout(new RowLayout(SWT.HORIZONTAL));
				leftHalf = new Composite(myShell, SWT.NONE);
				leftHalf.setLayout(new RowLayout(SWT.VERTICAL));
				leftHalf.setLayoutData(new RowData(HealthShieldsDisplay.DesiredWidth + 10, 100));
				((RowLayout)leftHalf.getLayout()).wrap = false;
				((RowLayout)leftHalf.getLayout()).justify = true;
				leftHalf.addPaintListener(new PaintListener(){
				    public void paintControl(PaintEvent e){
				        leftHalf.pack();
				    }
				});
				leftHalf.setBackground(myDisplay.getSystemColor(SWT.COLOR_WHITE));
				DrawTankMap dtm = new DrawTankMap(myTC, myShell);
				myShell.setText("TankSoar Map");
				amOpen = true;
				myShell.addDisposeListener(new DisposeListener(){
					public void widgetDisposed(DisposeEvent e){
						amOpen = false;
						myShell = null;
					}
				});
				initButtons();
				SoarAgent[] tanks = myTC.getAllAgents();
				for(int i = 0; i < tanks.length; ++i){
					HealthShieldsDisplay hsd = new HealthShieldsDisplay((Tank)tanks[i], leftHalf);
				}
				myShell.pack();
				myShell.open();
			}
		});
	}
    
	/**
	 * Initializes all the buttons on the left side of the shell and their tool tip texts for
	 * this <code>VisTankMap</code>.
	 */
	private void initButtons(){
		stepButton = new Button(leftHalf, SWT.PUSH);
		stepButton.setLayoutData(new RowData(160, 25));
		stepButton.setText("Step");
		stepButton.setToolTipText("Go one step in the simulation.");

		stepButton.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				//TODO: step code
			}
		});
		runButton = new Button(leftHalf, SWT.PUSH);
		runButton.setLayoutData(new RowData(160, 25));
		runButton.setText("Run");
		runButton.setToolTipText("Causes the simulation to run.");
		runButton.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				if(!myTC.isRunning()){
					new Thread(){
						public void run(){
							myTC.runSimulation();
						}
					}.start();
				}
			}
		});
		stopButton = new Button(leftHalf, SWT.PUSH);
		stopButton.setLayoutData(new RowData(160, 25));
		stopButton.setText("Stop");
		stopButton.setToolTipText("Stops the simulation");
		stopButton.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				if(myTC.isRunning()){
					myTC.stopSimulation();
				}
			}
		});
	}
	
	/**
	 * In our case, returns <code>null</code>.
	 * {@inheritDoc}
	 */
	public SoarAgent getAgent(){
		return (null);
	}
	
	/**
	 * In this case, will create a new <code>HealthShieldsDisplay</code> and display it on
	 * the left side of the visible shell.
	 * {@inheritDoc}
	 */
	public void agentCreated(SoarAgent created){
		HealthShieldsDisplay hsd = new HealthShieldsDisplay((Tank)created, leftHalf);
		leftHalf.setBackground(myDisplay.getSystemColor(SWT.COLOR_WHITE));
		leftHalf.pack();
	}
	
	/**
	 * In our case, will remove the <code>HealthShieldsDisplay</code> listening to
	 * the destroyed <code>SoarAgent</code>, if it exists.
	 * {@inheritDoc}
	 */
	public void agentDestroyed(SoarAgent destroyed){
		Control[] kids = leftHalf.getChildren();
		for(int i = 0; i < kids.length; ++i){
			if(kids[i] instanceof HealthShieldsDisplay){
				if(((HealthShieldsDisplay)kids[i]).getTank().equals(destroyed)){
					kids[i].dispose();
					break;
				}
			}
		}
	}
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void locationChanged(Location loc) { /* IGNORED */ }
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void newMap(String message) { /* IGNORED */ }
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void simEnded(String message) { /* IGNORED for now*/ }
	
	/**
	 * Closes the <code>Shell</code> of this <code>VisTankMap</code>. {@inheritDoc}
	 */
	public void simQuit() {
	    close();
	}
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void simStarted() { /* IGNORED */ }
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void worldCountChanged(int worldCount) { /* IGNORED */ }

	protected void onClose() {
		myTC.mapShown = false;		
	}

	public void widgetDisposed(DisposeEvent arg0) {
		amOpen = false;		
	}
}
