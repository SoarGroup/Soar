package soar2d.player;

public class MoveInfo {
	public boolean move = false;
	public int moveDirection = -1;
	
	public boolean jump = false;
	
	public boolean eat = false;
	
	public MoveInfo() {
		
	}
	
	public MoveInfo(boolean move, int moveDirection, boolean jump, boolean eat) {
		this.move = move;
		this.moveDirection = moveDirection;
		this.jump = jump;
		this.eat = eat;
	}
}
