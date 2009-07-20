package broken.soar2d.players;

import soar2d.Soar2D;

/**
 * @author voigtjr
 */
public class Cook {	
	private CommandInfo move;
	
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
	public CommandInfo getMove() {
		return move;
	}

	public boolean getHumanMove() {
		if (Soar2D.config.generalConfig().headless) {
			move = new CommandInfo();
			return true;
		}

		move = Soar2D.wm.getHumanCommand(this);

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
