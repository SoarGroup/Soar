/* File: EaterOutputInfo.java
 * Date: Sep 17, 2004
 */
package edu.umich.eaters;

/**
 * Simple struct-like class to simulate the output-link of an <code>Eater</code> in
 * the eaters simulation. Contains information on movement--jumping and moving--and
 * all variables are by default initialized to <code>null</code>. Thus, non-<code>null
 * </code> values are wmes on the output-link.
 * @author John Duchi
 */
public class EaterOutputInfo {
	  
	public int agent;
	/**
	 * Default constructor creates an <code>EaterOutputInfo</code> whose
	 * fields are all <code>null</code>
	 */
	public EaterOutputInfo(){}
	
	/**
	 * Creates an <code>EaterOutputInfo</code> whose fields are initialized
	 * as per <code>decision</code>, where <code>decision</code> is an
	 * <code>int</code> as defined in <code>Eater</code> relating to movement
	 * or jumping. Will initialize either the <code>jump</code> or <code>move
	 * </code> parameter of this object to be in the direction specifed in
	 * <code>decision</code>.
	 * <p>Possible values for <code>decision</code> are <code>Eater.JumpNorth</code>,
	 * <code>Eater.JumpEast</code>, <code>Eater.JumpSouth</code>, <code>Eater.JumpWest</code>,
	 * <code>Eater.MoveEast</code>, <code>Eater.MoveNorth</code>, <code>Eater.MoveWest</code>,
	 * and <code>Eater.MoveSouth</code>.
	 * @param decision The decision as to which this <code>EaterOutputInfo</code> will be
	 * initialized.
	 */
	public EaterOutputInfo(int decision){
		switch(decision){
		case(Eater.JumpEast): jump = new JumpOutput("east"); break;
		case(Eater.JumpWest): jump = new JumpOutput("west"); break;
		case(Eater.JumpNorth): jump = new JumpOutput("north"); break;
		case(Eater.JumpSouth): jump = new JumpOutput("south"); break;
		case(Eater.MoveNorth): move = new MoveOutput("north"); break;
		case(Eater.MoveWest): move = new MoveOutput("west"); break;
		case(Eater.MoveEast): move = new MoveOutput("east"); break;
		case(Eater.MoveSouth): move = new MoveOutput("south"); break;
		default: break;
		}
	}
	
	/** Represents a move. */
	public static class MoveOutput{
		
		/**
		 * Creates a new move output-link with direction attribute value specified.
		 * @param dir The direction of the jump.
		 */
		public MoveOutput(String dir){ direction = dir; }
		
		/**
		 * The direction the <code>Eater</code> wishes to move this turn.
		 * If non-<code>null</code>, will be interpreted as a command to move.
		 * <p><b>IO usage:</b> ^move.direction north/south/east/west
		 */
		public String direction = null;
		
		/**
		 * If this <code>MoveOutput</code>'s <code>direction</code> variable is null,
		 * returns <code>super.toString()</code>, if not, returns a <code>String</code>
		 * indicating the direction that this move output has stored.
		 * {@inheritDoc}
		 */
		public String toString(){
			return((direction == null ? super.toString():"^move.direction " + direction));
		}
	}
	
	/**
	 * An <code>Eater</code>'s move output representation. Default initialization is to
	 * <code>null</code>.
	 */
	public MoveOutput move = null;
	
	/** Represents a jump. */
	public static class JumpOutput{
		
		/**
		 * Creates a new jump output-link with direction attribute value specified.
		 * @param dir The direction of the jump.
		 */
		public JumpOutput(String dir){ direction = dir; }
		
		/**
		 * The direction the <code>Eater</code> wishes to jump this turn.
		 * If non-<code>null</code>, will be interpreted as a command to jump.
		 * <p><b>IO usage:</b> ^jump.direction north/south/east/west
		 */
		public String direction = null;
		
	}
	
	/**
	 * An <code>Eater</code>'s jump output representation. Default initialization is to
	 * <code>null</code>.
	 */
	public JumpOutput jump = null;

}
