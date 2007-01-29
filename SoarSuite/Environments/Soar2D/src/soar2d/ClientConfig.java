package soar2d;

/**
 * @author voigtjr
 *
 * This is a record of data for a client such as the debugger that is
 * going to connect to the simulation.
 */
public class ClientConfig {
	
	public ClientConfig() {}
	
	public ClientConfig(ClientConfig config) {
		this.name = new String(config.name);
		if (config.command != null) {
			this.command = new String(config.command);
		}
		
		this.timeout = config.timeout;
		this.after = config.after;
	}
	/**
	 * The name the client will report itself as.
	 */
	public String name = new String();
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

