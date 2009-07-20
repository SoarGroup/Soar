package edu.umich.visualsoar.operatorwindow;

/**
 * This class just defines an exception for our parse error
 * @author Jon Bauman
 * @version 0.5a 6 15 1999
 */


public class IncorrectFormatException extends Exception {
	
	/**
	 * @return a description of the error
	 */
	public String toString() {
		
		return super.toString() + ":\nIncorrect format, file may be corrupt";
	}
}
