package edu.umich.eaters;
/* File: EaterListener.java
 * Jul 12, 2004
 */

/**
 * Interface that listeners to an Eater must implement to be able to keep track
 * of the Eater. Allows updating on any changes the Eater makes.
 * @author jduchi
 */
public interface EaterListener {

	/**
	 * Notification fired when the score of an Eater has changed.
	 * @param e As a convenience, the Eater whose score has changed is passed with this
	 * notification.
	 */
	public void scoreChanged(Eater e);
	
	/**
	 * Notification fired by an Eater when the location of an Eater has changed.
	 * @param e The Eater whose location has changed.
	 */
	public void eaterLocationChanged(Eater e);
	
	/**
	 * Notification fired when the move/decision count of the Eater has changed.
	 * @param e The Eater whose move or decision count changed.
	 */
	public void moveCountChanged(Eater e);

	/**
	 * Notification fired when the <code>Eater</code> is supposed to make a decision.
	 * @param e The <code>Eater</code> that will be making the decision.
	 * @param sensors The <code>EaterOutputInfo</code> representing the input-link
	 * sensors available to the <code>Eater</code>.
	 */
	public void makingDecision(Eater e, EaterInputInfo sensors);
	
}
