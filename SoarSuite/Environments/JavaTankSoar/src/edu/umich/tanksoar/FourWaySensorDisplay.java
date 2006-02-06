/* File: FourWaySensorDisplay.java
 * Sep 3, 2004
 */
package edu.umich.tanksoar;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;

/**
 * A widget designed to be used with TankSoar to give visual representations of the direction
 * from which a Tank's input sensors are receiving information.
 * @author jduchi
 */
public class FourWaySensorDisplay extends Canvas implements PaintListener{

	private Display myDisplay;
	private static Color blueColor;
	private static Color blackColor;
	private static final int myHeight = 50, myWidth = 50;
	private static final int TopOffset = 20;
	private boolean sensorRight = false, sensorLeft = false,
		sensorBackward = false, sensorForward = false;
	private String myName;
	
	public FourWaySensorDisplay(Composite parent, String name){
		super(parent, SWT.NONE);
		myDisplay = parent.getDisplay();
		addPaintListener(this);
		myName = name;
		blueColor = myDisplay.getSystemColor(SWT.COLOR_BLUE);
		blackColor = myDisplay.getSystemColor(SWT.COLOR_BLACK);
	}
	
	public Point computeSize(int wHint, int hHint, boolean changed){
		return(new Point(myWidth, myHeight + TopOffset));
	}

	public void paintControl(PaintEvent e){
		GC gc = e.gc;
		gc.setForeground(blackColor);
		gc.setBackground(blueColor);
		gc.setLineWidth(1);
		gc.fillRectangle(myWidth/3, TopOffset + myHeight/3, myWidth/3, myHeight/3);
		if(sensorRight){
			gc.fillRectangle(2*myWidth/3, TopOffset + myHeight/3, myWidth/3, myHeight/3);
		}
		if(sensorLeft){
			gc.fillRectangle(0, TopOffset + myHeight/3,  myWidth/3, myHeight/3);
		}
		if(sensorForward){
			gc.fillRectangle(myWidth/3, TopOffset, myWidth/3, myHeight/3);
		}
		if(sensorBackward){
			gc.fillRectangle(myWidth/3, TopOffset + 2*myHeight/3, myWidth/3, myHeight/3);
		}
		gc.drawRectangle(myWidth/3, TopOffset, myWidth/3, myHeight-1);
		gc.drawRectangle(0, TopOffset + myHeight/3, myWidth-1, myHeight/3);
		gc.setLineWidth(2);
		gc.drawRectangle(myWidth/3, TopOffset + myHeight/3, myWidth/3, myHeight/3);
		Point p = gc.stringExtent(myName);
		gc.setForeground(myDisplay.getSystemColor(SWT.COLOR_BLACK));
		gc.drawString(myName, myWidth/2 - p.x/2, 2, true);
	}
	
	/**
	 * Sets the left notification box to be either on or off--that is, it turns blue if
	 * on and transparent (gray) if off.
	 * @param enabled <code>true</code> to turn the left on, <code>false</code> to turn
	 * it off.
	 */
	public void setSensorLeft(final boolean enabled){
		if((enabled && !sensorLeft) || (!enabled && sensorLeft)){
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					if(isDisposed()) return;
					sensorLeft = enabled;
					redraw(0, TopOffset + myHeight/3, myWidth/3, myHeight/3, false);
				}
			});
		}
	}
	
	/**
	 * Sets the forward (the top most) notification box to be either on or off--that is,
	 * it turns blue if on and transparent (gray) if off.
	 * @param enabled <code>true</code> to turn the left on, <code>false</code> to turn
	 * it off.
	 */
	public void setSensorForward(final boolean enabled){
		if((enabled && !sensorForward) || (!enabled && sensorForward)){
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					if(isDisposed()) return;
					sensorForward = enabled;
					redraw(myWidth/3, TopOffset, myWidth/3, myHeight/3, false);
				}
			});
		}
	}
	
	/**
	 * Sets the backward (the bottom most one) notification box to be either on or off--that
	 * is, it turns blue if on and transparent (gray) if off.
	 * @param enabled <code>true</code> to turn the backward on, <code>false</code> to turn
	 * it off.
	 */
	public void setSensorBackward(final boolean enabled){
		if((enabled && !sensorBackward) || (!enabled && sensorBackward)){
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					if(isDisposed()) return;
					sensorBackward = enabled;
					redraw(myWidth/3, TopOffset + 2*myHeight/3, myWidth/3, myHeight/3, false);
				}
			});
		}
	}
	
	/**
	 * Sets the right notification box to be either on or off--that is, it turns blue if
	 * on and transparent (gray) if off.
	 * @param enabled <code>true</code> to turn the right on, <code>false</code> to turn
	 * it off.
	 */
	public void setSensorRight(final boolean enabled){
		if((enabled && !sensorRight) || (!enabled && sensorRight)){
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					if(isDisposed()) return;
					sensorRight = enabled;
					redraw(2*myWidth/3, TopOffset + myHeight/3, myWidth/3, myHeight/3, false);
				}
			});
		}
	}
	
	//Simple test main
	public static void main(String[] args){
		Display d = new Display();
		Shell s = new Shell(d);
		final FourWaySensorDisplay fwsd = new FourWaySensorDisplay(s, "Doo!");
		s.setLayout(new FillLayout());
		s.pack();
		s.open();
		new Thread(){
			public void run(){
				for(int i = 10; i < 400; i = (int)(i*1.2)){
				fwsd.setSensorBackward(true);
				try{
					Thread.sleep(i);
				} catch(InterruptedException ignored){}
				fwsd.setSensorBackward(false);
				fwsd.setSensorRight(true);
				try{
					Thread.sleep(i);
				} catch(InterruptedException ignored){}
				fwsd.setSensorRight(false);
				fwsd.setSensorForward(true);
				try{
					Thread.sleep(i);
				} catch(InterruptedException ignored){}
				fwsd.setSensorForward(false);
				fwsd.setSensorLeft(true);
				try{
					Thread.sleep(i);
				} catch(InterruptedException ignored){}
				fwsd.setSensorLeft(false);
				}
			}
		}.start();
		while(!s.isDisposed()){
			if(!d.readAndDispatch()){
				d.sleep();
			}
		}
		d.dispose();
	}
	
}
