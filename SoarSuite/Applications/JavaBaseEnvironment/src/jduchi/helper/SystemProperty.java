/* File: SystemProperty.java
 * Sep 15, 2004
 */
package jduchi.helper;

/**
 * Provides a simpler static interface through which a client can access system properties.
 * @author John Duchi
 */
public class SystemProperty {

	/**
	 * Returns the file separator character ("/" on UNIX).
	 * @return The file separator character as a <code>String</code>.
	 */
	public static String fileSeparator(){
		return(System.getProperty("file.separator"));
	}
	
	/**
	 * Gives the path separator character (":" on UNIX).
	 * @return The path separator character in a <code>String</code>.
	 */
	public static String pathSeparator(){
		return(System.getProperty("path.separator"));
	}
	
	/**
	 * Returns the user's home directory.
	 * @return The path name of the user's home directory.
	 */
	public static String userHomeDirectory(){
		return(System.getProperty("user.home"));
	}
	
	/**
	 * Gives the current working directory.
	 * @return The path name of the current working directory.
	 */
	public static String workingDirectory(){
		return(System.getProperty("user.dir"));
	}
	
	/**
	 * Gives the current user's name.
	 * @return The current user's name.
	 */
	public static String userName(){
		return(System.getProperty("user.name"));
	}
	
	/**
	 * Gives the character used to indicate the end of a line ("\n" on UNIX).
	 * @return The end-line character as a <code>String</code>.
	 */
	public static String endLine(){
		return(System.getProperty("line.separator"));
	}
	
	/**
	 * Gives the name of the operating system currently in use.
	 * @return The <code>String</code> name of the current OS.
	 */
	public static String osName(){
		return(System.getProperty("os.name"));
	}
	
	/**
	 * Gives the version of the current operating system.
	 * @return The version of the current OS.
	 */
	public static String osVersion(){
		return(System.getProperty("os.version"));
	}
	
}
