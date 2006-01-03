/* File: TankListener.java
 * Aug 12, 2004
 */
package edu.umich.tanksoar;

/**
 * Interface that a class must implement to receive notifications from a Tank on its changes.
 * @author John Duchi
 */
public interface TankListener {

	/**
	 * Method invoked when a <code>Tank</code> is notified that all <code>Tank</code>s
	 * have finished making decisions and been moved. Lets listeners know that the
	 * <code>Tank</code>, knowing that all the <code>Tank</code>s have moved, has
	 * collected the sensory information about its environment that will enable its
	 * next decision.
	 * @param t As a convenience, the <code>Tank</code> whose sensors are being passed.
	 * @param sensors The input-link information available to the <code>Tank</code> with which it
	 * can make a decision.
	 */
	public void allDecisionsMade(Tank t, TankInputInfo sensors);

	/**
	 * Method invoked when the <code>Tank</code> has finished making a decision. Different
	 * from <code>allDecisionsMade(Tank, TankInputInfo)</code> because it is not guaranteed
	 * that all the other <code>Tank</code>s have finished their decisions.
	 * @param t The <code>Tank</code> that has made a decision.
	 * @param decision The <code>TankOutputInfo</code> that contains the
	 * decision the <code>Tank</code> has made.
	 */
	public void decisionMade(Tank t, TankOutputInfo decision);

	/**
	 * Method invoked when the <code>Tank</code> has been resurrected. This method
	 * is also invoked when a <code>Tank</code> has been reset, because so
	 * many of the <code>Tank</code>'s parameters change.
	 * @param t As a convenience, the <code>Tank</code> which has been resurrected.
	 */
	public void resurrected(Tank t);
	
	/**
	 * Method invoked when the <code>Tank</code> switches its radar on or off.
	 * @param t As a convenience parameter, the <code>Tank</code> whose radar has switched on or off.
	 */
	public void radarSwitch(Tank t);

	/**
	 * Method invoked when the <code>Tank</code>'s radar setting has changed.
	 * @param t As a convenience parameter, the <code>Tank</code> whose radar setting has changed.
	 */
	public void radarSettingChanged(Tank t);
	
	/**
	 * Method invoked when the direction the <code>Tank</code> is facing changes.
	 * @param t As a convenience, the <code>Tank</code> whose direction has changed.
	 */
	public void rotationChanged(Tank t);
	
	/**
	 * Method invoked when the <code>Tank</code>'s score changes.
	 * @param t As a convenience parameter, the <code>Tank</code> whose score has changed.
	 */
	public void scoreChanged(Tank t);
	
	/**
	 * Method invoked when a <code>Tank</code>'s energy changes.
	 * @param t As a convenience parameter, the <code>Tank</code> whose energy has changed.
	 */
	public void energyChanged(Tank t);
	
	/**
	 * Method invoked when a <code>Tank</code>'s health changes.
	 * @param t As a convenience parameter, the <code>Tank</code> whose health has changed.
	 */
	public void healthChanged(Tank t);
	
	/**
	 * Method invoked when the number of missiles a <code>Tank</code> is carrying changes.
	 * @param t The <code>Tank</code> whose missile count has changed.
	 */
	public void missileCountChanged(Tank t);
	
	/**
	 * Method invoked when a missile has hit a Tank.
	 * @param t As a convenience, the <code>Tank</code> that has been hit by the missile.
	 */
	public void missileHit(Tank t);
	
	/**
	 * Method invoked when the location of the <code>Tank</code> changes.
	 * @param t The <code>Tank</code> whose location has changed is passed as a convenience.
	 */
	public void locationChanged(Tank t);
}
