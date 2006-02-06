/* File: TankOutputInfo.java
 * Aug 31, 2004
 */
package edu.umich.tanksoar;

/**
 * A simple class to contain the information that a <code>Tank</code> can pass back to the simulation from
 * Soar, structured as much as possible as the information returning from Soar.
 * Default initializations for all sensors are null.
 * @author John Duchi
 */
public class TankOutputInfo {

	/** <code>boolean</code> to indicate switching an output on. Equal to <code>true</code>. */
	public static boolean on = true;
	/** <code>boolean</code> to indicate switching an output off. Equal to <code>false</code>. */
	public static boolean off = false;

  public final static String DIRECTION     = "direction";
  public final static String FIRE          = "fire";
  public final static String MOVE          = "move";
  public final static String OFF           = "off";
  public final static String ON            = "on";
  public final static String RADAR         = "radar";
  public final static String RADAR_POWER   = "radar-power";
  public final static String ROTATE        = "rotate";
  public final static String SETTING       = "setting";
  public final static String SHIELDS       = "shields";
  public final static String SWITCH        = "switch";
  public final static String WEAPON        = "weapon";

	/**
	 * Static nested class maintaining structure of movement output-link.
	 */
	public static class MoveOutput{
		
		/**
		 * Constructs a <code>MoveOutput</code> whose <code>direction</code> parameter is initialized
		 * to the empty string.
		 */
		public MoveOutput(){}
		
		/**
		 * Initializes the move output parameter to be in the direction specified.
		 * @param direction The String name of the direction the <code>Tank</code> wishes to move.
		 */
		public MoveOutput(String direction){
			this.direction = direction;
		}
		
		/**
		 * The direction this <code>Tank</code> wishes to move, if it is moving.
		 * <p><b>IO Usage:</b> ^move.direction forward/backward/left/right
		 */
		public String direction = "";
		
	}
	/**
	 * The <code>Tank</code>'s movement output link. If it is non-<code>null</code>,
	 * the <code>Tank</code> is expected to be moving.
	 */
	public MoveOutput move = null;
	
	/**
	 * Static nested class maintaining structure of rotate output-link.
	 */
	public static class RotateOutput{
		
		/**
		 * Constructs a <code>RotateOutput</code> whose direction is initialized to
		 * the empty string.
		 */
		public RotateOutput(){}
		
		/**
		 * Initializes the rotate output parameter to be in the direction specified.
		 * @param direction The String name of the direction the <code>Tank</code> wishes to rotate.
		 */
		public RotateOutput(String direction){
			this.direction = direction;
		}
		
		/**
		 * The direction this <code>Tank</code> wishes to rotate, if it will rotate.
		 * <p><b>IO Usage:</b> ^rotate.direction left/right
		 */
		public String direction = "";
		
	}
	
	/**
	 * The <code>Tank</code>'s rotate output link. If it is non-<code>null</code>,
	 * the <code>Tank</code> will be rotated.
	 */
	public RotateOutput rotate = null;
	
	/**
	 * Static nested class maintaining structure of missile firing output-link.
	 */
	public static class FireOutput{
		
		/**
		 * Constructs a <code>FireOutput</code> whose weapon is initialized to
		 * <code>null</code>.
		 */
		public FireOutput(){}
		
		/**
		 * Initializes the fire data object to be with the weapon specified.
		 * @param weapon The String name of the weapon (missile is the only one used).
		 */
		public FireOutput(String weapon){
			this.weapon = weapon;
		}
		
		/**
		 * The weapon the <code>Tank</code> will be firing, if it is firing a weapon.
		 * <p><b>IO Usage:</b> ^fire.weapon missile
		 */
		public String weapon = null;
		
	}
	
	/**
	 * This <code>Tank</code>'s firing output link. If it is non-<code>null</code>,
	 * the <code>Tank</code> will shoot a missile.
	 */
	public FireOutput fire = null;
	
	/**
	 * Static nested class maintaining structure of radar switch output-link.
	 */
	public static class RadarOutput{
		
		/**
		 * Constructs a <code>RadarOutput</code> whose switch parameter is initialized
		 * to switch radar off.
		 */
		public RadarOutput(){}
		
		/**
		 * Initializes the radar output object to be on or off as specified.
		 * @param onOff <code>TankOutputInfo.on</code> (or <code>true</code>) to switch the radar on,
		 * <code>TankOutputInfo.off</code> (or <code>false</code>) to switch the radar off.
		 */
		public RadarOutput(boolean onOff){
			Switch = onOff;
		}
		
		/**
		 * Whether the <code>Tank</code> wishes to switch its radar on or off. If set to
		 * true (or TankOutputInfo.on), will switch the radar on. Note that
		 * on Java side, we capitalize Switch.
		 * <p><b>IO Usage:</b> ^radar.switch on/off
		 */
		public boolean Switch = off;
		
	}
	
	/**
	 * The <code>Tank</code>'s radar output link. Used to switch the radar on and off. If it is non-<code>null</code>,
	 * the radar will be switched.
	 */
	public RadarOutput radar = null;
	
	/**
	 * Static nested class maintaining structure of radar-power output-link.
	 */
	public static class Radar_PowerOutput{
		
		/**
		 * Constructs a <code>Radar_PowerOutput</code> whose setting for the radar is
		 * initialized to 1.
		 */
		public Radar_PowerOutput(){}
		
		/**
		 * Initializes the radar-power setting object to the setting specified.
		 * @param set The desired setting for the radar-power.
		 */
		public Radar_PowerOutput(int set){
			setting = set;
		}
		
		/**
		 * Sets the power of the radar of this <code>Tank</code>. Ignored if the <code>Tank</code>'s radar is off.
		 * <p><b>IO Usage:</b> ^radar-power.setting 1-14
		 */
		public int setting = 1;
		
	}
	
	/**
	 * The <code>Tank</code>'s radar-power output link. Used to set the radar power. If it is non-<code>null</code>,
	 * the radar-power will be switched. Note that on the Java side, we use radar_power
	 * rather than radar-power.
	 */
	public Radar_PowerOutput radar_power = null;
	
	/**
	 * Static nested class maintaining structure of shields output-link.
	 */
	public static class ShieldsOutput{
		
		/**
		 * Constructs a <code>ShieldsOutput</code> whose switch parameter is initialized
		 * to switch the shields off.
		 */
		public ShieldsOutput(){}
		
		/**
		 * Initializes the <code>shields.switch</code> parameter of the <code>TankOutputInfo
		 * </code> to be specified by <code>onOff</code>.
		 * @param onOff <code>TankOutputInfo.on</code> (or <code>true</code>) to switch the shields on,
		 * <code>TankOutputInfo.off</code> (or <code>false</code>) to switch the shields off.
		 */
		public ShieldsOutput(boolean onOff){
			Switch = onOff;
		}
		
		/**
		 * Switches the shields of the <code>Tank</code> on or off. If set to <code>true</code>
		 * (or <code>TankOutputInfo.on</code>) will switch shields on. Note that on the Java
		 * side we capitalize Switch.
		 * <p><b>IO Usage:</b> ^shields.switch on/off
		 */
		public boolean Switch = off;

	}
	
	/**
	 * The <code>Tank</code>'s shields output-link. Used to turn the shields on or off. If
	 * non-<code>null</code>, the <code>Tank</code> has requested that the shields 
	 * be switched to whatever <code>shields.Switch</code> is set to. Note that just
	 * because <code>shields.switch</code> is <code>true</code> does not mean that<code>
	 * Tank<code>'s shields are on.
	 */
	public ShieldsOutput shields = null;
	
}
