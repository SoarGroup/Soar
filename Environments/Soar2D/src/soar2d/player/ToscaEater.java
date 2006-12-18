package soar2d.player;

import java.util.logging.Logger;

import soar2d.Direction;
import soar2d.Soar2D;
import soar2d.World;
import soar2d.tosca2d.*;
import tosca.*;

/**
 * @author doug
 *
 * Represents the communication between an agent and Tosca
 */
public class ToscaEater {
	protected Logger logger = Soar2D.logger;

	protected ToscaInterface m_ToscaInterface ;
	protected Library		 m_Library ;
	protected Eater m_Eater ;
	protected Eater getEater() { return m_Eater ; }
	protected EatersInputStateVariable m_InputVar ;
	protected int	m_EaterNumber ;
	
	/** This boolean switches tosca integration on and off.  When off, no Tosca code should run and we're back to regular Soar Eaters */
	public static final boolean kToscaEnabled = true ;
	
	public ToscaEater( Eater eater ) {
		m_Eater = eater ;
		
		// Establish a link to the C++ library code
		m_ToscaInterface = ToscaInterface.getTosca() ;
		m_Library = m_ToscaInterface.getToscaLibrary() ;
		
		// Create an input state variable.
		// BADBAD: The agent number should really be owned by the eater object, but since it's not an existing
		// Eater's concept for now it's easier to have our interface object track this.
		m_EaterNumber = m_ToscaInterface.generateNewAgentNumber() ;
		
		// Input var is named "agent-1-input" etc.  We use this rather than agent name so we can write generic code
		// on the Tosca side to handle these agents.
		m_InputVar = new EatersInputStateVariable("agent-" + m_EaterNumber + "-input") ;
		m_Library.AddStateVariable(m_InputVar) ;
		
		// Note: Had to hold off on this until the input variable has been created
		// so the other C++ modules can find it and hook up to it successfully.
		// BADBAD: Calling it here won't work if we create multiple eaters.
		// It looks like the initalization logic needs some rethinking
		m_Library.InitializeAll() ;
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
		
		Clock clock = m_Library.GetClock() ;
		int time = clock.GetTime() ;
		logger.info("Calling update on input var at time " + time+1) ;
		
		// Need to set the new value in the future (so choosing time+1)
		m_InputVar.update(time+1, this, world, location) ;
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
		// Switching to have tosca eater auto generate a direction so the eater
		// moves w/o my having to enter something.
		MoveInfo move = new MoveInfo() ;
		java.awt.Point oldLocation = getEater().previousLocation ;

		// Find a direction to move that is open
		for (int dir = 0 ; dir < 4 ; dir++)
		{
			move.move = true;
			
			// Tend to keep moving in the same direction as before
			move.moveDirection = getEater().getFacingInt() + dir ;
			if (move.moveDirection > 4)
				move.moveDirection = 1 ;
			
			// Calculate new location
			java.awt.Point newLocation = new java.awt.Point(oldLocation);
			Direction.translate(newLocation, move.moveDirection);
			// Verify legal move and commit move
			if (Soar2D.simulation.world.isInBounds(newLocation) && Soar2D.simulation.world.map.getCell(newLocation).enterable())
				break ;
		}

		// Just make things move slow enough that I can see what's happening
		try { java.lang.Thread.sleep(100) ; } catch (Exception ex) { }
		
		//MoveInfo move = Soar2D.wm.getHumanMove(getEater().getColor());
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
