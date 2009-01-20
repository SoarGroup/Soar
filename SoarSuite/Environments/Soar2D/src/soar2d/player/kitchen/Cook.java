package soar2d.player.kitchen;

import soar2d.Soar2D;
import soar2d.player.MoveInfo;
import soar2d.player.Player;

/**
 * @author voigtjr
 */
public class Cook extends Player {	
	private MoveInfo move;
	
	public Cook(String playerId) {
		super(playerId) ;
	}
	
	/* (non-Javadoc)
	 * @see soar2d.player.Player#update(soar2d.World, int [])
	 */
	public void update(int [] location) {
		super.update(location);
	}

	/* (non-Javadoc)
	 * @see soar2d.player.Player#getMove()
	 */
	public MoveInfo getMove() {
		return move;
	}

	public boolean getHumanMove() {
		if (Soar2D.config.generalConfig().nogui) {
			move = new MoveInfo();
			return true;
		}

		move = Soar2D.wm.getHumanMove(this);

		if (move == null) {
			return false;
		}
		
		return true;
	}

	public void moveWithObjectFailed() {
		// Ignored on this level, overridden in soarcook
		
	}

	public void moveFailed() {
	}

	public void mixFailed() {
	}

	public void cookFailed() {
	}

	public void eatFailed() {
	}
	
}
