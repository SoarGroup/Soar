package edu.umich.visualsoar.misc;
import java.io.*;


/**
 * This class describes the filter of a JFileChooser
 * This currently filters such that only .txt and .soar
 * files show up.
 * @author Brian Harleton
 */
public class TextFileFilter extends javax.swing.filechooser.FileFilter implements FilenameFilter {
	/**
	 * Will only accept files with a vsa extension
	 * @return true if the file ends with vsa or is a folder false otherwise
	 */
	public boolean accept(File f) {
		if (f.isDirectory())
			return true;

		String suffix =  getSuffix(f);
		if (suffix != null) {
			boolean accept = suffix.equals("txt");
      if(!accept)
        return suffix.equals("soar");
			return accept;
		}
		return false;
	}
	
	/**
	 * Same as the other accept method, but included for compatibilty with the FilenameFilter
	 * interface used for native dialogs
	 * @return true if the file ends with vsa or is a folder false otherwise
	 */
	public boolean accept(File dir, String name) {
		if (name.endsWith(".txt") || name.endsWith(".soar")) {
			return true;
		}
		else {
			return false;
		}
	}
	
	/**
	 * @return a string describing the filefilter
	 */
	public String getDescription() {
		return "Visual Soar Text File (*.txt, *.soar)";
	}

	/**
	 * Gets a suffix off a file
	 * @param f the file to examine
	 * @return the suffix of the file 
	 */
	private String getSuffix(File f) {
		String s = f.getPath(), suffix = null;

		int i = s.lastIndexOf('.');
		if (i > 0 && i < s.length() - 1)
			suffix = s.substring(i+1).toLowerCase();

		return suffix;
	}
}