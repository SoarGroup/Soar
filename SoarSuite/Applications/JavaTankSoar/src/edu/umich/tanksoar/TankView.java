/* File: TankView.java
 * Sep 3, 2004
 */
package edu.umich.tanksoar;

import org.eclipse.swt.widgets.*;

import edu.umich.JavaBaseEnvironment.AgentWindow;
import edu.umich.JavaBaseEnvironment.SoarAgent;


import org.eclipse.swt.layout.*;
import org.eclipse.swt.SWT;
/**
 * This class provides a GUI view of what a <code>Tank</code> can see, and it is updated
 * every time the <code>Tank</code> is supposed to make a decision.
 * @author John Duchi
 */
public class TankView extends AgentWindow implements TankListener{

	/** The <code>FourWaySensorDisplay</code> indicating from which directions this <code>TankView<code>'s
	 * <code>Tank</code> is blocked. */
	private FourWaySensorDisplay blockedSensor;
	/** The <code>FourWaySensorDisplay</code> indicating from which directions this <code>TankView<code>'s
	 * <code>Tank</code> hears another <code>Tank</code> moving. */
	private FourWaySensorDisplay soundSensor;
	/** The <code>FourWaySensorDisplay</code> indicating from which directions this <code>TankView<code>'s
	 * <code>Tank</code> has radar waves hitting it. */
	private FourWaySensorDisplay rwavesSensor;
	/** The <code>FourWaySensorDisplay</code> indicating from which directions this <code>TankView<code>'s
	 * <code>Tank</code> has incoming missiles. */
	private FourWaySensorDisplay incomingSensor;
	private RadarView radarViewSensor;
	private Tank myTank;
	private String myName;
	
	/**
	 * Constructs a new instance of TankView, the shell through which a user can view
	 * the information available to a tank when it makes a decision.
	 * @param d The <code>Display</code> that will hold this <code>TankView</code>.
	 * @param t The <code>Tank</code> to which this <code>TankView</code> is listening.
	 * @param imageDirectory The String path to the directory in which images are stored.
	 */
	public TankView(Display d, Tank t){
		myDisplay = d;
		myTank = t;
		myName = myTank.getName();
		myTank.addTankListener(this);
		initShell();
	}
	
	public void initShell(){
		if(myDisplay == null || myDisplay.isDisposed()) return;
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(myShell == null || myShell.isDisposed()) myShell = new Shell(myDisplay);
				myShell.setLayout(new RowLayout(SWT.HORIZONTAL));
				Composite left = new Composite(myShell, SWT.NONE);
				left.setLayout(new RowLayout(SWT.VERTICAL));
				MeasureBar mb = new MeasureBar(left, myTank, MeasureBar.ListenHealth);
				mb = new MeasureBar(left, myTank, MeasureBar.ListenEnergy);
				mb = new MeasureBar(left, myTank, MeasureBar.ListenRadar);
				Composite c = new Composite(left, SWT.NONE);
				c.setLayout(new RowLayout(SWT.HORIZONTAL));
				blockedSensor = new FourWaySensorDisplay(c, "Blocked");
				soundSensor = new FourWaySensorDisplay(c, "Sound");
				c = new Composite(left, SWT.NONE);
				c.setLayout(new RowLayout(SWT.HORIZONTAL));
				rwavesSensor = new FourWaySensorDisplay(c, "Rwaves");
				incomingSensor = new FourWaySensorDisplay(c, "Incoming");
				Composite right = new Composite(myShell, SWT.NONE);
				mb = new MeasureBar(left, myTank, MeasureBar.ListenSmell);
				right.setLayout(new FillLayout());
				radarViewSensor = new RadarView(right, myTank);
				myShell.setText(myName);
				myShell.pack();
				myShell.open();
				allDecisionsMade(myTank, myTank.fillSensors(null));
			}
		});
	}
	
	public SoarAgent getAgent(){
		return(myTank);
	}
	
	/**
	 * Sets all the <code>FourWaySensorDisplay</code>s to have their directions
	 * highlighted or not depending on the information passed in in <code>sensors</code>.
	 * {@inheritDoc}
	 */
	public void allDecisionsMade(Tank t, TankInputInfo sensors){
		if(sensors == null) return;
		blockedSensor.setSensorBackward(sensors.blocked.backward);
		blockedSensor.setSensorForward(sensors.blocked.forward);
		blockedSensor.setSensorLeft(sensors.blocked.left);
		blockedSensor.setSensorRight(sensors.blocked.right);
		incomingSensor.setSensorBackward(sensors.incoming.backward);
		incomingSensor.setSensorForward(sensors.incoming.forward);
		incomingSensor.setSensorLeft(sensors.incoming.left);
		incomingSensor.setSensorRight(sensors.incoming.right);
		rwavesSensor.setSensorBackward(sensors.rwaves.backward);
		rwavesSensor.setSensorForward(sensors.rwaves.forward);
		rwavesSensor.setSensorLeft(sensors.rwaves.left);
		rwavesSensor.setSensorRight(sensors.rwaves.right);
		boolean sBack = false, sForward = false, sRight = false, sLeft = false;
		switch(sensors.sound.charAt(0)){
		case('b'): sBack = true; break;
		case('f'): sForward = true; break;
		case('r'): sRight = true; break;
		case('l'): sLeft = true; break;
		}
		soundSensor.setSensorBackward(sBack);
		soundSensor.setSensorForward(sForward);
		soundSensor.setSensorLeft(sLeft);
		soundSensor.setSensorRight(sRight);
	}
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void decisionMade(Tank t, TankOutputInfo decision){}
	
	/**
	 * Calls <code>allDecisionsMade(t, t.fillSensors(null))</code> so that the
	 * display can be updated upon resurrection of a <code>Tank</code>.
	 * {@inheritDoc}
	 */
	public void resurrected(final Tank t){
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(myShell == null || myShell.isDisposed()) return;
				allDecisionsMade(t, t.fillSensors(null));
			}
		});
	}
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void energyChanged(Tank t){}
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void healthChanged(Tank t){}
		
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void locationChanged(Tank t){}
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void missileCountChanged(Tank t){}
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void missileHit(Tank t){}
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void radarSettingChanged(Tank t){}
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void radarSwitch(Tank t){}
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void rotationChanged(Tank t){}
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void scoreChanged(Tank t){}

	protected void onClose() {
		
	}
		
}
