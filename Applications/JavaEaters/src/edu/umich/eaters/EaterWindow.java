/* File: EaterWindow.java
 * Jul 20, 2004
 */

package edu.umich.eaters;

import edu.umich.JavaBaseEnvironment.AgentWindow;
import edu.umich.JavaBaseEnvironment.SimulationControlListener;
import edu.umich.JavaBaseEnvironment.SoarAgent;
import edu.umich.JavaBaseEnvironment.Location;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.*;

/**
 * @author John Duchi
 */
public class EaterWindow extends AgentWindow implements EaterListener, SimulationControlListener{

	private EaterControl myEC;
	private Eater myEater;
	private EaterView myCanvas;
	
	private Label scoreBox;
	private Label worldCountBox;
	private Label eaterMoveBox;
	
	private static int SquarePixels = 25;
	
	public EaterWindow(Display d, EaterControl ec, Eater e){
		myDisplay = d;
		myEC = ec;
		myEC.addSimulationControlListener(this);
		myEater = e;
		myEater.addEaterListener(this);
		initShell();
	}
	
	protected void initShell(){
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(myShell == null || myShell.isDisposed()){
					myShell = new Shell(myDisplay);
				}
				RowLayout rl = new RowLayout(SWT.VERTICAL);
				myShell.setLayout(rl);
				
				Composite top = new Composite(myShell, SWT.NONE);
				top.setLayout(new RowLayout(SWT.VERTICAL));
				myShell.setText(myEater.getColorName() + " eater");
				
				scoreBox = new Label(top, SWT.CENTER);
				scoreBox.setText("Score: " + myEater.getScore());
				
				worldCountBox = new Label(top, SWT.CENTER);
				worldCountBox.setText("World count: " + myEC.getWorldCount());
				
				eaterMoveBox = new Label(top, SWT.CENTER);
				eaterMoveBox.setText("Move count: " + myEater.getNumberMoves());
				top.pack();

				myCanvas = new EaterView(myShell, myEater, myEC);
				
				myShell.pack();
				myShell.open();
			}
		});
	}	
	
	public SoarAgent getAgent(){
		return(myEater);
	}
	
	public void eaterLocationChanged(Eater e){
		//myCanvas.eaterLocationChanged(e);
	}
	
	public void moveCountChanged(Eater e){
		if(e == myEater && !eaterMoveBox.isDisposed()){
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					if(eaterMoveBox.isDisposed()) return;
					eaterMoveBox.setText("Move count: " + myEater.getNumberMoves());
					eaterMoveBox.pack();
					eaterMoveBox.getParent().pack();
				}
			});
		}
	}
	
	public void makingDecision(Eater e, EaterInputInfo sensors){ /* IGNORED */ }
	
	public void scoreChanged(Eater e){
		if(e == myEater && !scoreBox.isDisposed()){
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					if(scoreBox.isDisposed()) return;
					scoreBox.setText("Score: " + myEater.getScore());
					scoreBox.pack();
					scoreBox.getParent().pack();
				}
			});
		}
	}
	
	public void agentCreated(SoarAgent created){
		
	}
	
	public void agentDestroyed(SoarAgent destroyed){
		if(destroyed.equals(myEater)){
			myEC.removeSimulationControlListener(this);
		}
	}
	
	public void locationChanged(Location loc){
		//myCanvas.locationChanged(loc);
	}
	
	public void newMap(String message){
		//myCanvas.newMap(message);
	}
	
	public void simEnded(String message){
		
	}
	
	public void simQuit(){
		//close();
	}
	
	public void simStarted(){
		
	}
	
	public void worldCountChanged(final int newWorldCount){
		if(myDisplay.isDisposed()) return;
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(myShell.isDisposed()) return;
				worldCountBox.setText("World count: " + newWorldCount);
				worldCountBox.pack();
				worldCountBox.getParent().pack();
			}
		});
		//myCanvas.worldCountChanged(newWorldCount);
	}

	protected void onClose() {
		
	}
}