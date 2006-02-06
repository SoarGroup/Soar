package edu.umich.visualsoar.util;
import javax.swing.Action;
import javax.swing.AbstractButton;

/**
 * This class is used to associate a button with an action so that
 * the the action is enabled, the button is enabled
 * if the action is disabled, then so is the button
 * @author Brad Jones
 * @version 0.5a 5 Aug 1999
 */
public class ActionButtonAssociation implements java.beans.PropertyChangeListener {
///////////////////////////////////////////////////////////////////
// Data Members
///////////////////////////////////////////////////////////////////
	private Action a;
	private AbstractButton b;
	
///////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////
	// deny default construction
	private ActionButtonAssociation() {}
	
	/**
	 * This associates the button with the give action
	 * @param _a the action for which you want the button associated
	 * @param _b the button for which you want the action associated
	 */
	public ActionButtonAssociation(Action _a, AbstractButton _b) {
		a = _a;
		b = _b;
		b.setEnabled(a.isEnabled());
	}

///////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////
	/**
	 * This listener method changes the buttons enabled-ness
	 * when the action's enabled-ness changes
	 * @param evt the change event, not used 
	 */
	public void propertyChange(java.beans.PropertyChangeEvent evt) {
		b.setEnabled(a.isEnabled());
	}
}
