/* File: HealthShieldsDisplay.java
 * Aug 30, 2004
 */
package edu.umich.tanksoar;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.events.*;
import org.eclipse.swt.*;

import edu.umich.JavaBaseEnvironment.AgentColors;
import edu.umich.JavaBaseEnvironment.Location;

/**
 * Implements a <code>Widget</code> to display a <code>Tank</code>'s health, energy,
 * missiles, and score as a set of bars and visible numbers. Can be placed
 * in another <code>Composite</code>. Don't actually know why it's called a
 * 'Shields' display, but energies of a <code>Tank</code> do change if the <code>Tank</code>'s
 * shields are on.
 * @author John Duchi
 */
public class HealthShieldsDisplay extends Canvas implements TankListener, PaintListener,
													DisposeListener{

	public static final int DesiredWidth = 160;
	
	private static final int WidthHeightRatio = 8;
	private Display myDisplay = null;
	private Tank myTank;
	private Color myColor = null;
	private int xSize = 0, ySize = 0;
	
	/**
	 * Creates a new HealthShields display, whose optimum size is 160 pixels wide and 60 pixels tall.
	 * @param t The tank to which this HealthShields display will be listening.
	 * @param parent The parent Composite of this Canvas.
	 */
	public HealthShieldsDisplay(Tank t, Composite parent){
		super(parent, SWT.NONE);
		myDisplay = parent.getDisplay();
		myTank = t;
		t.addTankListener(this);
		try{
			myColor = TankSWindowManager.getNamedColor(myTank.getColorName());
		} catch(SWTException ignored){}
		if(myColor == null){
			TankSWindowManager.initColors(myDisplay);
			myColor = TankSWindowManager.getNamedColor(myTank.getColorName());
		}
		addPaintListener(this);
		setBackground(myDisplay.getSystemColor(SWT.COLOR_WHITE));
	}
	
	/**
	 * Notification sent when this <code>Widget</code> is disposed.
	 * Removes this <code>HealthShieldsDisplay</code> from its <code>Tank</code>'s listeners.
	 * @param e An event describing the disposal.
	 */
	public void widgetDisposed(DisposeEvent e){
		myTank.removeTankListener(this);
	}
	
	/**
	 * Sent when a paint event occurs for the <code>Control</code>.
	 * @param e An event describing the paint, including the graphics context in which to draw.
	 */
	public void paintControl(PaintEvent e){
		GC gc = e.gc;
		Point p = getSize();
		xSize = p.x;
		ySize = p.y;
		int yThird = ySize/3;
		gc.setForeground(myColor);
		gc.setLineWidth(3);
		gc.drawPolygon(new int[]{1, 1, xSize-1, 1, xSize-1, 2*yThird-1, 1, 2*yThird-1});
		gc.setBackground(myDisplay.getSystemColor(SWT.COLOR_RED));
		gc.fillRectangle(4, 4, (int)(((double)myTank.getHealth()/Tank.MaxHealth)*(xSize-8)), yThird-4);
		gc.setBackground(myDisplay.getSystemColor(SWT.COLOR_DARK_BLUE));
		gc.fillRectangle(4, yThird, (int)(((double)myTank.getEnergy()/Tank.MaxHealth)*(xSize-8)), yThird-4);
		gc.setForeground(myDisplay.getSystemColor(SWT.COLOR_BLACK));
		gc.setBackground(myDisplay.getSystemColor(SWT.COLOR_WHITE));
		gc.drawText("Points " + myTank.getScore() + "|Missiles " + myTank.getMissileCount(), 4, 2*yThird+1);
	}
	
	/**
	 * Returns the preferred size of the receiver. The <i>preferred</i> size is
	 * the size at which this <code>Control</code> would best display at. The hint
	 * arguments allow a caller to ask, "Given a particular width, what is the
	 * size at which to best display this?" If <code>wHint < 80</code>, returns
	 * as if <code>wHint</code> were equal to <code>DesiredWidth</code>.
	 * @return A <code>Point</code> whose x is the preferred width, y the preferred
	 * height. 
	 */
	public Point computeSize(int wHint, int hHint, boolean changed){
		if(wHint == SWT.DEFAULT || wHint < 80) wHint = DesiredWidth;
		hHint = (3*wHint)/WidthHeightRatio;
		return new Point(wHint, hHint);
	}
	
	/**
	 * Redraws the energy bar.
	 * {@inheritDoc}
	 */
	public void energyChanged(Tank t){
		if(myDisplay.isDisposed()) return;
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(isDisposed()) return;
				if(xSize != 0){
					redraw(4, ySize/3, xSize-8, ySize/3 -4, false);
				} else {
					redraw();
				}
			}
		});
	}
	
	/**
	 * Redraws the health bar.
	 * {@inheritDoc}
	 */
	public void healthChanged(final Tank t){
		if(myDisplay.isDisposed()) return;
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(isDisposed()) return;
				if(xSize != 0){
					redraw(4, 4, xSize-8, ySize/3-4, false);
				} else {
					redraw();
				}
			}
		});
	}
	
	/**
	 * Gives the caller the <code>Tank</code> being listened to.
	 * @return The <code>Tank</code> to which this <code>HealthShieldsDisplay</code> is listening.
	 */
	public Tank getTank(){
		return(myTank);
	}
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void locationChanged(Tank t) { /* IGNORED */ }
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void missileCountChanged(Tank t) {
		scoreChanged(t);
	}
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void missileHit(Tank t) { /* IGNORED */ }
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void radarSettingChanged(Tank t) { /* IGNORED */ }
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void radarSwitch(Tank t) { /* IGNORED */ }
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void rotationChanged(Tank t) { /* IGNORED */ }
	
	/**
	 * Redraws the score numbers.
	 * {@inheritDoc}
	 */
	public void scoreChanged(Tank t) { 
		if(myDisplay.isDisposed()) return;
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(isDisposed()) return;
				if(xSize != 0){
					redraw(4, 2*ySize/3, xSize-8, ySize/3-4, false);
				} else {
					redraw();
				}
			}
		});
	}
	
	/**
	 * Redraws the entire <code>HealthShieldsDisplay</code>.
	 * {@inheritDoc}
	 */
	public void resurrected(Tank t){
		if(myDisplay.isDisposed()) return;
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(isDisposed()) return;
				redraw();
			}
		});
	}
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void decisionMade(Tank t, TankOutputInfo decision) { /* IGNORED */ }
	
	/**
	 * Ignored.
	 * {@inheritDoc}
	 */
	public void allDecisionsMade(Tank t, TankInputInfo sensors) { /* IGNORED */ }
	
	/*
	 * Just a simple test main.
	 */
	public static void main(String[] args){
		Display d = new Display();
		Shell s = new Shell(d);
		TankSoarJControl tc = new TankSoarJControl();

		Tank t = new Tank(AgentColors.yellow, new Location(0, 0), tc);
		HealthShieldsDisplay hsd = new HealthShieldsDisplay(t, s);
		s.setLayout(new FillLayout());
		s.pack();
		s.open();
		while(!s.isDisposed()){
			if(!d.readAndDispatch()){
				d.sleep();
			}
		}
		d.dispose();
	}
	
}
