package soar2d.player;

/**
 * @author voigtjr
 *
 * output required for interaction with the world
 */
public class MoveInfo {
	/**
	 * true indicates a directional move has been made
	 */
	public boolean move = false;
	/**
	 * must be valid if move is true, indicates direction to move
	 */
	public int moveDirection = -1;
	/**
	 * eaters: indicates if a jump is made, move must be true if this is true,
	 * and by implication, moveDirection must be valid
	 */
	public boolean jump = false;
	/**
	 * eaters: eat the food encountered during this update. does not eat food
	 * on the current cell unless we don't actually move
	 */
	public boolean eat = false;
	/**
	 * stop the simulation by command
	 */
	public boolean stop = false;
	/**
	 * open the box on the current cell
	 */
	public boolean open = false;
	
	public MoveInfo() {
	}
}
