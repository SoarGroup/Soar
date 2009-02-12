package broken.soar2d.players;

import soar2d.Soar2D;

/**
 * @author voigtjr
 *
 */
public class Taxi {	
	private CommandInfo move;

	public Taxi(String playerId) {
		super(playerId) ;
	}
	
	public void update(int [] location) {
		super.update(location);
	}

	public CommandInfo getMove() {
		return move;
	}

	public boolean getHumanMove() {
		move = Soar2D.wm.getHumanCommand(this);

		if (move == null) {
			return false;
		}
		
		return true;
	}
}
