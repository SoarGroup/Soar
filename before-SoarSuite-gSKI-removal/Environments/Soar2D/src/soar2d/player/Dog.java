package soar2d.player;

public class Dog extends Player {

	public Dog(PlayerConfig playerConfig) {
		super(playerConfig);
	}

	private MoveInfo move;

	public MoveInfo getMove() {
		// the facing depends on the move
		this.setFacingInt(move.moveDirection);
		return move;
	}

	public boolean getHumanMove() {
		move = new MoveInfo();
		return true;
	}
}
