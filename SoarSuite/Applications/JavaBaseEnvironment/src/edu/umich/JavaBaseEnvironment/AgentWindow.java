package edu.umich.JavaBaseEnvironment;
/* File: AgentWindow.java
 * Jul 20, 2004
 */

import javax.swing.JOptionPane;

import org.eclipse.swt.widgets.*;

/**
 * Interface to specify the operations to which a window or shell watching a
 * specific SoarAgent must respond.
 * @author jduchi
 */
public abstract class AgentWindow{

	protected Display myDisplay = null;
	protected Shell myShell = null;
	
	/**
	 * Method to tell if the implementing class's main view window is open.
	 * @return True if the shell is open, false otherwise.
	 */
	public boolean isOpen(){
		return(myShell != null && !myShell.isDisposed());
	}

	/**
	 * Method to tell if the implementing class's main view window is open.
	 * @return False if the shell is open, true otherwise.
	 */
	public boolean isClosed(){
		return(myShell != null && myShell.isDisposed());
	}
	
	/**
	 * Method to give the SoarAgent being viewed by the instance of AgentWindow.
	 * @return The SoarAgent to which this AgentWindow is listening.
	 */
	public abstract SoarAgent getAgent();
	
	/**
	 * Opens the agent window (or brings it to the front if already open).
	 */
	public void open(){
		if(myShell != null && !myShell.isDisposed()){
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					myShell.setActive();
				}
			});
		} else {
			initShell();
		}
	}

	/**
	 * Closes the agent window (should have no response if window is already closed).
	 */
	public void close(){
		if(myShell == null) return;
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				myShell.close();
			}
		});
	}
	
	/**
	 * Method that must be implemented by any subclasses of AgentWindow.
	 * Called by open() if the shell is not open, must initialize the shell
	 * and set it visible, etc.
	 * <p>Must set the <code>amOpen</code> ivar of this AgentWindow to be
	 * <code>true</code>.
	 */
	protected abstract void initShell();
}
