/* File: VisMap.java
 * Jul 13, 2004
 */

package edu.umich.eaters;

import edu.umich.JavaBaseEnvironment.AgentWindow;
import edu.umich.JavaBaseEnvironment.SimulationControlListener;
import edu.umich.JavaBaseEnvironment.SimulationControl;
import edu.umich.JavaBaseEnvironment.SoarAgent;
import edu.umich.JavaBaseEnvironment.Location;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;

/**
 * Holds a <code>DrawEatersMap</code> in a shell.
 * @author John Duchi
 */
public class VisMap extends AgentWindow implements SimulationControlListener{

	private EaterControl myEC;
	
	/**
	 * Creates a new instance of VisMap using ec, which has height and width defined
	 * by the number of spaces in the EaterControl's internal map and SquareSize.
	 * @param ec The EaterControl that will serve as the model for this VisMap.
	 */
	public VisMap(SimulationControl ec, Display display){
		myEC = (EaterControl)ec;
		myEC.addSimulationControlListener(this);
		myDisplay = display;
	}
	
	/**
	 * {@inheritDoc}
	 * Overridden to return null.
	 */
	public SoarAgent getAgent(){
	    return(null);
	}
	
	protected void initShell(){
		if(myDisplay == null) return;
		if(myDisplay.isDisposed()){
			return;
		}
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(myShell == null || myShell.isDisposed()){
					myShell = new Shell(myDisplay);
					myShell.setText("Eaters World");
					DrawEatersMap dem = new DrawEatersMap(myShell, myEC);
				}
				myShell.setLayout(new FillLayout());
				myShell.pack();
				myShell.open();
			}
		});				
	}
	
	public void agentCreated(SoarAgent created){ /* IGNORED */ }
	
	public void agentDestroyed(SoarAgent destroyed){ /* IGNORED */ }
	
	public void locationChanged(Location loc){ /* IGNORED */ }
	
	public void newMap(String message){ /* IGNORED */ }
	
	public void simEnded(String message){ /* IGNORED */ }
	
	public void simQuit(){
		close();
	}
	
	public void simStarted(){ /* IGNORED */ }
	
	public void worldCountChanged(int worldCount){ /* IGNORED */ }

	protected void onClose() {
		
	}
}
