package soar2d;

public class MoveInfo {
	boolean move = false;
	int moveDirection;
	
	boolean jump = false;
	
	boolean eat = false;
	
	public MoveInfo() {
		
	}
	
	public MoveInfo(boolean move, int moveDirection, boolean jump, boolean eat) {
		this.move = move;
		this.moveDirection = moveDirection;
		this.jump = jump;
		this.eat = eat;
	}
}
