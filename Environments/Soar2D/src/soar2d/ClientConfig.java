package soar2d;

/**
 * @author voigtjr
 *
 * This is a record of data for a client such as the debugger that is
 * going to connect to the simulation.
 */
public class ClientConfig {
	/**
	 * The name the client will report itself as.
	 */
	public String name = null;
	/**
	 * The command used to start the client. If not present, the client
	 * is started externally.
	 */
	public String command = null;
	/**
	 * Seconds to wait for the client before timing out and giving up.
	 */
	public int timeout = Soar2D.config.kDefaultTimeout;
	/**
	 * If true, the client is expected (or spawned) after agent creation.
	 * If false, the client is expected (or spawned) before agent creation. 
	 */
	public boolean after = false;
}

