/**
 * Port of a configuration file utility used in Soar2D so that it is available
 * for use in C++. Not sure if we're even going to need it. This is essentially
 * an exercise in porting Java straight to C++.
 *
 * Original: SoarSuite/Environments/Soar2D/src/soar2d/config/
 *   Config.java
 *   ConfigFile.java
 *   ConfigSource.java
 *   ConfigUtil.java
 *
 * Until the initial port is done and working:
 *  * Memory issues such as who owns what pointers when is getting pushed aside
 *  * Everything is going to throw std::exception on errors, will sort out later
 */

//package soar2d.config;
//
//import java.io.File;
///** Useful configuration utilities. **/
//public class ConfigUtil {
//	public static Config tryLoadConfig(String path) {
//		if (path == null)
//			return null;
//
//		try {
//			File f = new File(path);
//			if (!f.exists())
//				return null;
//
//			return new ConfigFile(path).getConfig();
//		} catch (IOException ex) {
//			return null;
//		}
//	}
//
//	public static Config getDefaultConfig(String args[]) {
//		Config config = null;
//
//		if (args.length == 1) {
//			config = tryLoadConfig(args[0]);
//		}
//
//		for (int i = 0; i < args.length; i++) {
//			if (args[i].equals("--config") && i + 1 < args.length)
//				config = tryLoadConfig(args[i + 1]);
//			if (args[i].startsWith("--config=")) {
//				config = tryLoadConfig(args[i].substring(9));
//			}
//		}
//
//		String envpath = System.getenv("APRIL_CONFIG");
//		if (config == null) {
//			config = tryLoadConfig(envpath);
//		}
//
//		 last resort
//		if (config == null) {
//			System.out
//					.println("No config file specified, using empty configuration");
//			config = new ConfigFile().getConfig();
//		}
//
//		return config;
//	}
//
//	public static double[] getPosition(Config config, String sensorName) {
//		return config.requireDoubles(sensorName + ".position");
//	}
//	
//	public static void main(String [] args) {
//		Config config = ConfigUtil.tryLoadConfig("Environments/Soar2D/src/soar2d/config/test.cnf");
//		System.out.println("Hello " + config.getString("hello") + ".");
//		
//		System.out.println("block.inside " + config.getString("block.inside"));
//		System.out.println("block.outside " + config.getString("block.outside"));
//		System.out.println("block.notthere " + config.getString("block.notthere"));
//		
//		Config config2 = config.getChild("parent.child1");
//		System.out.println("config2: one (without default) " + config2.getString("one"));
//		System.out.println("config2: one (with default) " + config2.getString("one", null));
//	}
//	
//}
