package soar2d.player;

import soar2d.*;

public class Cat extends Player {
	private MoveInfo move;

	public Cat(PlayerConfig playerConfig) {
		super(playerConfig);
	}
	
	public MoveInfo getMove() {
		// the facing depends on the move
		this.setFacingInt(move.moveDirection);
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
