/* File: DrawEatersMap.java
 * Sep 15, 2004
 */
package edu.umich.eaters;


import edu.umich.JavaBaseEnvironment.SimulationControlListener;
import edu.umich.JavaBaseEnvironment.SoarAgent;
import edu.umich.JavaBaseEnvironment.Location;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.*;

/**
 * @author jduchi
 */
public class DrawEatersMap extends Canvas implements SimulationControlListener, PaintListener{

	private Display myDisplay;
	private EaterControl myEC;
	
	public static int SquareSize = 37;
	
	public DrawEatersMap(Composite parent, EaterControl ec){
		super(parent, SWT.NONE);
		myDisplay = parent.getDisplay();
		myEC = ec;
		myEC.addSimulationControlListener(this);
		addPaintListener(this);
	}
	
	public Point computeSize(int wHint, int hHint, boolean changed){
		return(new Point(SquareSize*EaterControl.MapWidth, SquareSize*EaterControl.MapHeight));
	}
	
	public void paintControl(PaintEvent e){
		GC gc = e.gc;
		for(int x = 0; x < EaterControl.MapWidth; x++){
			for(int y = 0; y < EaterControl.MapHeight; y++){
				EaterDrawer.drawContents(gc, x, y, myEC.getLocationContents(x, y), SquareSize);
			}
		}
	}
	
	private void drawAllEaters(){
		Eater[] eaters = myEC.getAllEaters();
		if(eaters == null) return;
		for(int i = 0; i < eaters.length; i++){
			locationChanged(eaters[i].getLocation());
		}
	}
	
	public void agentDestroyed(SoarAgent destroyed){
		locationChanged(destroyed.getLocation());
	}
	
	public void agentCreated(SoarAgent created){
		locationChanged(created.getLocation());
	}
	
	public void simStarted(){
		
	}
	
	public void simQuit(){
		
	}
	
	public void simEnded(String message){
		
	}
	
	public void worldCountChanged(int newWorldCount){
		drawAllEaters();
	}
	
	public void locationChanged(final Location loc){
		if(myEC.hasQuit()) return;
		myDisplay.syncExec(new Runnable(){
			public void run(){
				if(isDisposed()) return;
				redraw(loc.getX()*SquareSize, loc.getY()*SquareSize, SquareSize, SquareSize, false);
			}
		});
	}
	
	public void newMap(String message){
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				redraw();
			}
		});
	}
	
	
}
