package soar2d.player;

import java.util.logging.Logger;

import soar2d.Soar2D;
import soar2d.World;

/**
 * @author doug
 *
 * Represents the communication between an agent and Tosca
 */
public class ToscaEater {
	protected Logger logger = Soar2D.logger;

	protected Eater m_Eater ;
	protected Eater getEater() { return m_Eater ; }
	
	/** This boolean switches tosca integration on and off.  When off, no Tosca code should run and we're back to regular Soar Eaters */
	public static final boolean kToscaEnabled = true ;
	
	public ToscaEater( Eater eater ) {
		m_Eater = eater ;
	}
	
	/* (non-Javadoc)
	 * @see soar2d.player.Player#update(soar2d.World, java.awt.Point)
	 */
	public void update(World world, java.awt.Point location) {
		// check to see if we've moved.
		getEater().moved = (location.x != getEater().previousLocation.x) || (location.y != getEater().previousLocation.y);
		if (getEater().moved) {
			getEater().previousLocation = new java.awt.Point(location);
		}
	}
	
	/* (non-Javadoc)
	 * @see soar2d.player.Player#getMove()
	 */
	public MoveInfo getMove() {
		// if we're not graphical, the human agent can't enter input.
		// maybe we should support this in the future.
		if (Soar2D.config.graphical == false) {
			return new MoveInfo();
		}
		MoveInfo move = Soar2D.wm.getHumanMove(getEater().getColor());
		// the facing depends on the move
		getEater().setFacingInt(move.moveDirection);
		logger.info("Tosca agent move direction " + move.moveDirection);
		return move;
	}
	/* (non-Javadoc)
	 * @see soar2d.player.Player#shutdown()
	 */
	public void shutdown() {
		// nothing to do
	}
}
