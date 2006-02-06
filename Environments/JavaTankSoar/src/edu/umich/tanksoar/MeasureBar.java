/* File: MeasureBar.java
 * Sep 7, 2004
 */
package edu.umich.tanksoar;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
/**
 * Implements a GUI bar to help show sensor settings. Depending on the sensor it is specified
 * to be listening to a <code>Tank</code> for, will listen to radar, health, energy, or
 * smell. Displays the value of the specified sensor every time it is updated by the
 * <code>Tank</code>, and also shows a visual bar that extends to a proportion of the 
 * maximum value for that sensor.
 * @author John Duchi
 */
public class MeasureBar extends Canvas implements TankListener, PaintListener{

	/** Constant to show that this listens to radar setting of a <code>Tank</code>. */
	public static final int ListenRadar = 0;
	/** Constant to show that this listens to health of a <code>Tank</code>. */
	public static final int ListenHealth = 1;
	/** Constant to show that this listens to energy of a <code>Tank</code>. */
	public static final int ListenEnergy = 2;
	/** Constant to show that this listens to smell sensor of a <code>Tank</code>. */
	public static final int ListenSmell = 3;
	
	/** The desired width (in pixels) of the changing bar part of the <code>MeasureBar</code>. */
	public static final int BarDesiredWidth = 100;
	/** The total desired width (in pixels) of the <code>MeasureBar</code>. */
	public static final int TotalDesiredWidth = 120;
	/** The number of pixels offset from the dynamic bar for text. */
	public static final int TopTextOffset = 20;
	
	/** The <code>Tank</code> to which this <code>MeasureBar</code> is listening. */
	private Tank myTank;
	/** The sensor to which this <code>MeasureBar</code> listens. */
	private int mySensor;
	/** The <code>org.eclipse.swt.widgets.Display</code> that manages this <code>MeasureBar</code>. */
	private Display myDisplay = null;
	
	private int myMax, myMin, myValue = 0;
	/** The <code>String.valueOf(int)</code> value of <code>myValue</code>. @see String#valueOf(int)*/
	private String myValueName = null;
	/** The name of the sensor to which this is listening. */
	private String myName = null;
	
	/**
	 * Creates a new <code>MeasureBar</code> with the specified parent, <code>Tank</code>,
	 * and a sensor to which it will listen. Possible values for the sensor are
	 * <code>MeasureBar.ListenRadar</code>, <code>MeasureBar.ListenHealth</code>,
	 * <code>MeasureBar.LsitenEnergy</code>, and <code>MeasureBar.ListenSmell</code>.
	 * @param parent The <code>Composite</code> that is to be the parent of this instance.
	 * @param t The <code>Tank</code> to which this will be listening.
	 * @param sensorToHear The integer value of the sensor to which this will be listening.
	 */
	public MeasureBar(Composite parent, Tank t, int sensorToHear){
		super(parent, SWT.NONE);
		mySensor = sensorToHear;
		myTank = t;
		myTank.addTankListener(this);
		myDisplay = parent.getDisplay();
		switch(sensorToHear){
		case(ListenRadar):
			myMin = 0;
			myMax = Tank.MaxRadar;
			myValue = myTank.getRadarSetting();
			myName = "Radar Setting";
			break;
		case(ListenHealth):
		    myMin = 0;
			myMax = Tank.MaxHealth;
			myValue = myTank.getHealth();
			myName = "Health";
			break;
		case(ListenEnergy):
		    myMin = 0;
			myMax = Tank.MaxEnergy;
			myValue = myTank.getEnergy();
			myName = "Energy";
			break;
		case(ListenSmell):
		    myMin = 0;
			myMax = Tank.MaxSmell;
			myName = "Smell";
			break;
		default:
		    myMin = 0;
			myMax = 100;
			myName = "";
			break;
		}
		addPaintListener(this);
	}
	
	/**
	 * Notification sent to paint this <code>MeasureBar</code>.
	 * @param e An event containing information about this paint call.
	 */
	public void paintControl(PaintEvent e){
		GC gc = e.gc;
		Color lightBlue = myDisplay.getSystemColor(SWT.COLOR_CYAN);
		gc.setBackground(myDisplay.getSystemColor(SWT.COLOR_WHITE));
		gc.fillRectangle(0, 0, TotalDesiredWidth, TopTextOffset + TotalDesiredWidth/5);
		gc.setForeground(myDisplay.getSystemColor(SWT.COLOR_BLACK));
		Point p = gc.stringExtent(myName);
		int x = TotalDesiredWidth/2 - p.x/2;
		gc.drawString(myName, x, 1);
		gc.setForeground(myDisplay.getSystemColor(SWT.COLOR_BLUE));
		gc.setLineWidth(3);
		gc.drawRectangle(1, TopTextOffset + 1, TotalDesiredWidth - 2, TotalDesiredWidth/5 - 2);
		gc.setBackground(lightBlue);
		gc.fillRectangle(4, 4 + TopTextOffset, (int)(((double)myValue/myMax)*(TotalDesiredWidth - 7)), BarDesiredWidth/6);
		gc.setForeground(myDisplay.getSystemColor(SWT.COLOR_BLACK));
		if(myValueName == null){
			myValueName = String.valueOf(myValue);
		}
		gc.drawText(myValueName, TotalDesiredWidth/2 - gc.stringExtent(myValueName).x/2, TopTextOffset + 3, true);
	}
	
	/**
	 * Returns the desired size of this <code>MeasureBar</code> as a <code>Point</code> whose
	 * x coordinate is the desired width, y coordinate the desired height. These values are
	 * <code>TotalDesiredWidth</code> and <code>TopTextOffset + TotalDesiredWidth/5</code>, respectively.
	 * @return The desired size of this <code>MeasureBar</code>.
	 */
	public Point computeSize(int wHint, int hHint, boolean changed){
	    return(new Point(TotalDesiredWidth, TopTextOffset + TotalDesiredWidth/5));
	}
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void decisionMade(Tank t, TankOutputInfo decision){}
	
	/**
	 * This updates the smell sensor for <code>MeasureBar</code>.
	 * {@inheritDoc}
	 */
	public void allDecisionsMade(Tank t, TankInputInfo sensors){
		if(mySensor == ListenSmell){
			TankInputInfo.SmellSensor smells = sensors.smell;
			myValueName = smells.color;
			myValue = (sensors.smell.distance == -1 ? 0:sensors.smell.distance);
			myValueName = (myValue == 0 ? myValueName:myValueName.concat(String.valueOf(myValue)));
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					if(!MeasureBar.this.isDisposed()){
						redraw(0, TopTextOffset, TotalDesiredWidth, TotalDesiredWidth/5, false);
					}
				}
			});
		}
	}
	
	/**
	 * If listening to energy, redraws the dynamic bar portion of this <code>MeasureBar</code>.
	 * {@inheritDoc}
	 */
	public void energyChanged(Tank t){
		if(mySensor == ListenEnergy){
		    myValue = t.getEnergy();
		    myValueName = String.valueOf(myValue);
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					if(!MeasureBar.this.isDisposed()){
						redraw(0, TopTextOffset, TotalDesiredWidth, TotalDesiredWidth/5, false);
					}
				}
			});
		}
	}
	
	/**
	 * If listening to health, redraws the dynamic bar portion of this <code>MeasureBar</code>.
	 * {@inheritDoc}
	 */
	public void healthChanged(Tank t){
		if(mySensor == ListenHealth){
			myValue = t.getHealth();
			myValueName = String.valueOf(myValue);
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					if(!MeasureBar.this.isDisposed()){
						redraw(0, TopTextOffset, TotalDesiredWidth, TotalDesiredWidth/5, false);
					}
				}
			});
		}
	}
	
	/**
	 * Redraws the energy, health, or radar sensor, if those are being listened to.
	 * {@inheritDoc}
	 */
	public void resurrected(Tank t){
		energyChanged(t);
		healthChanged(t);
		radarSettingChanged(t);
	}
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void locationChanged(Tank t){}
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void missileCountChanged(Tank t){}
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void missileHit(Tank t){}
	
	/**
	 * If listening to radar-setting, redraws the dynamic bar portion of this <code>MeasureBar</code>.
	 * {@inheritDoc}
	 */
	public void radarSettingChanged(Tank t){
		if(mySensor == ListenRadar){
		    myValue = t.getRadarSetting();
		    myValueName = String.valueOf(myValue);
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					if(!MeasureBar.this.isDisposed()){
						redraw(0, TopTextOffset, TotalDesiredWidth, TotalDesiredWidth/5, false);
					}
		    	}
			});
		}
	}
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void radarSwitch(Tank t){}
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void rotationChanged(Tank t){}
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void scoreChanged(Tank t){}

}
