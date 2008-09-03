package soar2d.player.taxi;

import soar2d.Soar2D;
import soar2d.player.MoveInfo;
import soar2d.player.Player;
import soar2d.player.PlayerConfig;

/**
 * @author voigtjr
 *
 */
public class Taxi extends Player {	
	private MoveInfo move;

	public Taxi( PlayerConfig playerConfig ) {
		super(playerConfig) ;
	}
	
	public void update(java.awt.Point location) {
		super.update(location);
	}

	public MoveInfo getMove() {
		return move;
	}

	public boolean getHumanMove() {
		if (Soar2D.config.getNoGUI()) {
			move = new MoveInfo();
			return true;
		}

		move = Soar2D.wm.getHumanMove(this);

		if (move == null) {
			return false;
		}
		
		return true;
	}
}
