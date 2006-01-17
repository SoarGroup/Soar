/**
 *
 * @file ProductionWrapper.java
 * @date Jun 10, 2004
 */
package edu.rosehulman.soar.editor.outline;

/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class ProductionWrapper implements Comparable {
	private String _name;
	private int _line;
	
	public ProductionWrapper(String name, int line) {
		_name = name;
		_line = line;
	}
	
	public String getName() {
		return _name;
	}
	
	public int getLine() {
		return _line;
	}
	
	public int compareTo(Object o) {
		return _name.toLowerCase().compareTo( 
			((ProductionWrapper) o).getName().toLowerCase() );
	}
}
