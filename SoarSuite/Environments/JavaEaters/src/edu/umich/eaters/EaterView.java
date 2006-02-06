/* File: EaterView.java
 * Jul 21, 2004
 */

package edu.umich.eaters;

import edu.umich.JavaBaseEnvironment.SimulationControlListener;
import edu.umich.JavaBaseEnvironment.Location;
import edu.umich.JavaBaseEnvironment.SoarAgent;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.*;
/**
 * @author John Duchi
 */
public class EaterView extends Canvas implements SimulationControlListener, EaterListener,
												PaintListener, DisposeListener{
	
	private EaterControl myEC;
	private Eater myEater;
	public static int SquareSize = 25;
	private Display myDisplay = null;
	
	public EaterView(Composite parent, Eater e, EaterControl ec){
		super(parent, SWT.NONE);
		myDisplay = parent.getDisplay();
		myEater = e;
		myEC = ec;
		myEC.addSimulationControlListener(this);
		myEater.addEaterListener(this);
		addPaintListener(this);
		addDisposeListener(this);
	}
	
	public void paintControl(PaintEvent e){
		GC gc = e.gc;
		gc.setForeground(EaterSWindowManager.black);
		drawView(gc);
		gc.drawRectangle(0, 0, 5*SquareSize, 5*SquareSize);

	}
	
	public void widgetDisposed(DisposeEvent e){
		myEC.removeSimulationControlListener(this);
		myEater.removeEaterListener(this);
	}
	
	private void drawView(GC gc){
		Object[][] sights = myEater.getVisibleSquares();
		for(int x = 0; x < 5; x++){
			for(int y = 0; y < 5; y++){
				EaterDrawer.drawContents(gc, x, y,
						sights[x][y], SquareSize);
			}
		}
	}
	
	public void setSquareSize(int newSize){
		SquareSize = newSize;
	}
	
	public Point computeSize(int wHint, int hHint, boolean changed){
		return (new Point(SquareSize*5 + 1, SquareSize*5 + 1));
	}
		
	public void locationChanged(Location loc){
		if(isDisposed() || getDisplay().isDisposed()) return;
		Location distance = myEater.distanceFrom(loc);
		final int x = distance.getX(), y = distance.getY();
		if(-2 <= x && x <= 2 && -2 <= y && y <= 2){
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					if(isDisposed()) return;
//					if(receivedEaterMoveNotification || EaterView.this.isDisposed()) return;
					redraw((x+2)*SquareSize, (y+2)*SquareSize, SquareSize, SquareSize, true);
				}
			});
		}
	}
	
	public void simStarted(){}

	public void simEnded(String message){}

	public void agentCreated(SoarAgent created){
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if (!isDisposed())
					redraw();
			}
		});
	}
	
	public void agentDestroyed(SoarAgent destroyed){
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if (!isDisposed())
					redraw();
			}
		});
	}
	
	public void worldCountChanged(int worldCount){
		
	}
	
	public void simQuit(){}
	
	public void newMap(String message){
		if(myDisplay.isDisposed()) return;
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(isDisposed()) return;
				redraw();
			}
		});
	}
	
	private boolean receivedEaterMoveNotification = false;
	
	public void eaterLocationChanged(Eater e){
		receivedEaterMoveNotification = true;
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(isDisposed()) return;
				redraw();
			}
		});
	}
	
	public void makingDecision(Eater e, EaterInputInfo sensors){
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(isDisposed()) return;
				redraw();
				if(receivedEaterMoveNotification){
					redraw();
				} else {
					redraw(SquareSize*2+1, SquareSize*2+1, SquareSize, SquareSize, true);
				}
			}
		});
		receivedEaterMoveNotification = false;
	}
	
	public void moveCountChanged(Eater e){
		
	}

	public void scoreChanged(Eater e){
		
	}
	
}
